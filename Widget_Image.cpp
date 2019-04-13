#include "Widget_Image.h"

#include<QToolTip>
#include<QMouseEvent>
#include<QPaintEvent>
#include<QPainter>
#include<QDebug>

#include"FileStructs/FilePNG.h"

#define HAS_COLOR_TABLE image.colorCount()>0
#define VALID_COLOR_INDEX(index) (index>=0 && index<image.colorCount())

#define FOREACH_PIXEL_IN_RECT(x0,y0,x1,y1,code)\
QColor color;\
for(int y=y0;y<y1;++y){\
	for(int x=x0;x<x1;++x){\
		color=image.pixelColor(x,y);\
		code;\
	}\
}

#define FOREACH_PIXEL(code)\
auto w=image.width(),h=image.height();\
FOREACH_PIXEL_IN_RECT(0,0,w,h,code)\

Palette::Palette():maxGray(0),color(Qt::black),brightness(0xFF){}

Widget_Image::Widget_Image(QWidget *parent):QWidget(parent),
	flashTimerID(0),
	flashStatus(false){
	setMouseTracking(true);
}

void Widget_Image::loadImage(const QString &filename){
	if(image.load(filename)){
		//find the code if exist
		if(HAS_COLOR_TABLE){
			FilePNG filePng;
			filePng.loadFile(filename.toStdString());
			filePng.parseData();
			auto chunk=filePng.findChunk("tEXt");
			if(chunk){
				auto block=chunk->chunkDataBlock();
				block.dataPointer[block.dataLength]='\0';//Hack!
				paletteCode=(char*)block.dataPointer;
			}
			filePng.deleteDataPointer();
		}
		update();
	}
}
void Widget_Image::saveImage(const QString &filename,const QString &code)const{
	if(!image.save(filename))return;

	//fix some thing
	FilePNG filePng;
	filePng.loadFile(filename.toStdString());
	filePng.parseData();

	//remove pHYs chunk
	auto phys=filePng.findChunk("pHYs");
	if(phys)filePng.allChunks.remove(phys);

	//add custom code chunk
	auto len=code.length();
	FilePNG_Chunk chunk;//tEXt
	if(len>0){
		chunk.newDataPointer(12+len);//size(4)+name(4)+data(size)+crc(4)
		chunk.setChunkLength(len);
		chunk.setChunkName("tEXt");
		memcpy(chunk.chunkDataBlock().dataPointer,code.toStdString().data(),len);
		chunk.setChunkCRC(chunk.makeCRC());
		filePng.allChunks.push_back(&chunk);
	}

	//re-save file
	filePng.saveFilePNG(filename.toStdString());
	chunk.deleteDataPointer();
	filePng.deleteDataPointer();
}

void Widget_Image::loadFilePng(const QString &filename){
	//加载文件并分析
	FilePNG filePng;
	filePng.loadFile(filename.toStdString());
	filePng.parseData();
	//进行解码
	Bitmap_32bit bitmap32;
	filePng.decodeTo(bitmap32);
	//生成图像用于显示
	auto w=bitmap32.getWidth(),h=bitmap32.getHeight();
	uint32 color32=0;ColorRGBA rgba;
	image=QImage(w,h,QImage::Format_RGBA8888);
	QColor color;
	for(decltype(h) y=0;y<h;++y){
		for(decltype(w) x=0;x<w;++x){
			bitmap32.getColor(x,y,color32);
			rgba.fromRGBA(color32);//RGBA是OpenGL的格式
			color.setRgba(rgba.toBGRA());
			image.setPixelColor(x,y,color);//BGRA是BMP和QImage的主要格式
		}
	}
	update();
	//生成颜色表
	makeColorsList(filePng);
	//内存回收
	filePng.reset();
	bitmap32.deleteBitmap();
}
void Widget_Image::makeColorsList(const FilePNG &filePng){
	colorsList.clear();
	auto ihdr=filePng.findIHDR();
	auto plte=filePng.findPLTE();
	if(ihdr&&plte){
		bool hasColor=false,hasAlpha=false;//确定色表类型
		if(ihdr->getColorMask_Color(hasColor) && ihdr->getColorMask_Alpha(hasAlpha)){
			qDebug()<<hasColor<<hasAlpha;
			//获取颜色数
			uint amount=hasColor?
				(hasAlpha ? plte->rgbaAmount() : plte->rgbAmount()):
				(hasAlpha ? plte->grayAlphaAmount() : plte->grayAmount());
			auto trns=filePng.findtRNS();
			//获取各个颜色值
			QColor color;ColorRGBA rgba;
			uint16 grayAlpha;
			uint8 u8;
			for(uint i=0;i<amount;++i){
				if(hasColor){
					if(hasAlpha){
						plte->getRGBA(i,rgba);
						color.setRgb(rgba.red,rgba.green,rgba.blue,rgba.alpha);
					}else{//标准色表
						plte->getColor(i,rgba);
						color.setRgb(rgba.red,rgba.green,rgba.blue,rgba.alpha);
					}
				}else{
					if(hasAlpha){
						plte->getGrayAlpha(i,grayAlpha);
						color.setRed(grayAlpha&0xFF);
						color.setGreen(color.red());
						color.setBlue(color.red());
						color.setAlpha(grayAlpha>>8);
					}else{
						plte->getGray(i,u8);
						color.setRgb(u8,u8,u8);
						if(trns && trns->getAlpha(i,u8)){
							color.setAlpha(u8);
						}
					}
				}
				qDebug()<<color;
				colorsList.push_back(color);
			}
		}
	}
}

void Widget_Image::saveFilePng(const QString &filename,uint8 bitDepth,bool hasPalette,bool hasColor,bool hasAlpha)const{
	//转换成Bitmap32格式
	Bitmap_32bit bitmap32;
	auto w=image.width(),h=image.height();
	bitmap32.newBitmap(w,h);
	ColorRGBA rgba;uint32 color32;
	for(decltype(h) y=0;y<h;++y){
		for(decltype(w) x=0;x<w;++x){
			rgba.fromBGRA(image.pixel(x,y));
			color32=rgba.toRGBA();
			bitmap32.setColor(x,y,color32);
		}
	}
	//保存
	FilePNG filePng;
	filePng.encodeFrom(bitmap32,bitDepth,hasPalette,hasColor,hasAlpha);
	filePng.saveFilePNG(filename.toStdString());
	//内存回收
	bitmap32.deleteBitmap();
	filePng.reset();
}

QColor Widget_Image::makeImage(const QImage &image){
	this->image=QImage(image.width(),image.height(),QImage::Format_Indexed8);
	//make color list
	for(int i=0;i<colorsList.size();++i){
		this->image.setColor(i,colorsList[i].rgba());
	}
	//make image
	int idx=0;
	FOREACH_PIXEL(
	idx=colorsList.indexOf(color);
	if(idx>=0)this->image.setPixel(x,y,idx);
	else return color;
	);
	return QColor();
}

void Widget_Image::makeColorsList(const QImage &image){
	colorsList.clear();
	if(HAS_COLOR_TABLE){//use image's color table
		auto colorTable=image.colorTable();
		QColor color;
		for(auto rgb:colorTable){
			color.setRgb(rgb);
			color.setAlpha(qAlpha(rgb));//setRgb will ignore alpha
			colorsList.append(color);
		}
	}else{//statistics all pixels to make color table
		FOREACH_PIXEL(if(!colorsList.contains(color))colorsList.append(color););
	}
}
void Widget_Image::parsePaletteCode(){
	int colorIndex=0;
	paletteList.clear();
	int start=0,end=0,len=paletteCode.length();
	while(start<len){
		//scan name
		while(end<len && !paletteCode[end].isDigit())++end;
		auto name=paletteCode.mid(start,end-start);
		start=end;//next

		if(start>=len)break;

		//scan num
		while(end<len && paletteCode[end].isDigit())++end;//scan num
		auto strNum=paletteCode.mid(start,end-start);
		start=end;
		//got data
		bool ok;
		int num=strNum.toInt(&ok);
		if(ok){
			Palette plte;
			plte.name=name;
			for(int i=0;i<num;++i){
				++colorIndex;
				plte.allIndexes.append(colorIndex);
				//check the gray scale
				if(colorIndex<colorsList.size()){
					auto newGray=qGray(colorsList[colorIndex].rgba());
					if(newGray>=plte.maxGray){
						plte.maxGray=newGray;
						plte.color=colorsList[colorIndex];
					}
				}
			}
			paletteList.push_back(plte);
		}
		qDebug()<<name<<num;
	}
}

static int roundDivide(int a,int b){
	int ret=a/b;
	int remain=a%b;
	if(remain*2>=b)++ret;
	return ret;
}

#define CHANGE_COLOR(name) \
color.set##name(roundDivide(q##name(rgba) * plte.brightness * gray,0xFF * plte.maxGray));\

void Widget_Image::updateByPalette(const Palette &plte){
	//int maxGray=qGray(plte.color.rgba());
	for(auto &index:plte.allIndexes){
		if(plte.maxGray>0){
			QColor color;
			auto rgba=plte.color.rgba();
			int gray=qGray(image.color(index));
			//change color
			CHANGE_COLOR(Red)
			CHANGE_COLOR(Green)
			CHANGE_COLOR(Blue)
			color.setAlpha(qAlpha(rgba));
			image.setColor(index,color.rgba());
		}else{
			image.setColor(index,0x00000000);
		}
	}
}
void Widget_Image::updateByAllPalettes(){
	for(auto &plte:paletteList){
		updateByPalette(plte);
	}
}

bool Widget_Image::imageColor(int index,QColor &color)const{
	if(!VALID_COLOR_INDEX(index))return false;
	auto rgba=image.color(index);
	color.setRgb(rgba);
	color.setAlpha(qAlpha(rgba));
	return true;
}

bool Widget_Image::changeColor(int index,const QColor &destColor){
	//change color
	auto ok=changeColor(index,destColor,image.rect());
	if(ok){
		colorsList[index]=destColor;//save new color
	}
	return ok;
}
bool Widget_Image::changeColor(int index,const QColor &destColor,const QRect &rect){
	if(index>=colorsList.size())return false;
	auto srcColor=colorsList[index];
	FOREACH_PIXEL_IN_RECT(rect.left(),rect.top(),rect.right(),rect.bottom(),
		if(color==srcColor)image.setPixelColor(x,y,destColor);
	);
	return true;
}

void Widget_Image::startFlash(int index){
	if(VALID_COLOR_INDEX(index)){
		QColor color(image.color(index));
		startFlash(color);
	}
}
void Widget_Image::startFlash(const QColor &flashColor){
	penColor=flashColor;
	//create flash image
	QColor flashColorX(flashColor.rgba()^0x00808080);
	flashImage=QImage(image.width(),image.height(),QImage::Format_Mono);
	flashImage.setColor(0,0x00000000);
	flashImage.setColor(1,flashColorX.rgba());
	flashImage.fill(Qt::transparent);
	//make flash image
	FOREACH_PIXEL(if(color==flashColor)flashImage.setPixel(x,y,1););

	//start flash
	stopFlash();
	flashTimerID=startTimer(500);
}
void Widget_Image::stopFlash(){
	killTimer(flashTimerID);
}

void Widget_Image::timerEvent(QTimerEvent *ev){
	if(ev->timerId()==flashTimerID){
		flashStatus=!flashStatus;
		update();
	}
}
void Widget_Image::mousePressEvent(QMouseEvent *ev){
	if(image.isNull())return;
	switch(ev->button()){
		case Qt::LeftButton:
			if(HAS_COLOR_TABLE){//index
				auto table=image.colorTable();
				auto index=table.indexOf(penColor.rgba());
				if(!VALID_COLOR_INDEX(index))return;
				//draw
				image.setPixel(cursorPos,index);
			}else{//non-index
				image.setPixelColor(cursorPos,penColor);
			}
		break;
	}
}
void Widget_Image::mouseMoveEvent(QMouseEvent *ev){
	if(image.isNull())return;
	mousePos=ev->pos();
	QPoint p=mouseToImage(mousePos);
	QColor color=image.pixelColor(p);
	int index=colorsList.indexOf(color);
	//show tip
	QToolTip::showText(ev->globalPos(),
		QString::asprintf("(%d,%d)==(%.2X,%.2X,%.2X,a=%.2X),index==%d",
			p.x(),p.y(),
			color.red(),color.green(),color.blue(),color.alpha(),
			index));
	//update
	if(cursorPos!=p){
		cursorPos=p;
		update();
	}
}
void Widget_Image::mouseReleaseEvent(QMouseEvent *ev){}

void Widget_Image::paintEvent(QPaintEvent *ev){
	QPainter painter(this);
	painter.drawImage(this->rect(),image);
	if(flashStatus){
		painter.drawImage(this->rect(),flashImage);
	}
	//draw cursor rect
	int w=image.width(),h=image.height();
	if(w&&h){
		//caculate pos
		w=this->width()/w;
		h=this->height()/h;
		painter.setBrush(QBrush(penColor));
		//set color
		auto p=imageToMouse(cursorPos);
		painter.drawRect(p.x(),p.y(),w,h);
	}
}

QPoint Widget_Image::mouseToImage(const QPoint &p)const{
	return QPoint(p.x()*image.width()/this->width(),
				  p.y()*image.height()/this->height());
}
QPoint Widget_Image::imageToMouse(const QPoint &p)const{
	return QPoint(p.x()*this->width()/image.width(),
				  p.y()*this->height()/image.height());
}
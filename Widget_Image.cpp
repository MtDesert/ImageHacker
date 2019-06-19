#include "Widget_Image.h"
#include"common.h"

#include<QToolTip>
#include<QMouseEvent>
#include<QPaintEvent>
#include<QPainter>
#include<QDebug>

#include"FileStructs/FilePNG.h"
#include"FileStructs/FileBMP.h"
#include"Number.h"

#define HAS_COLOR_TABLE image.colorCount()>0
#define VALID_COLOR_INDEX(index) (index>=0 && index<colorsList.size())

#define FOREACH_PIXEL_IN_RECT(x0,y0,x1,y1,code)\
for(int y=y0;y<y1;++y){\
	for(int x=x0;x<x1;++x){\
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
	fromBitmap32(bitmap32);
	update();
	//生成颜色表
	makeColorsList(filePng);
	//内存回收
	bitmap32.deleteBitmap();
	filePng.deleteDataPointer();
}
void Widget_Image::makeColorsList(const FilePNG &filePng){
	auto ihdr=filePng.findIHDR();
	auto plte=filePng.findPLTE();
	if(ihdr && plte){
		plte->filePng_tRNS=filePng.findtRNS();
		plte->getColorsList(*ihdr,colorsList);
	}
}
void Widget_Image::makeColorsList(const FileBMP &fileBmp){
	if(fileBmp.bgrasList.dataLength){
		fileBmp.bgrasList.getColorsList(colorsList);
	}
}

void Widget_Image::saveFilePng(const QString &filename,uint8 bitDepth,bool hasPalette,bool hasColor,bool hasAlpha,List<uint32> *colorsList)const{
	//转换成Bitmap32格式
	Bitmap_32bit bitmap32;
	toBitmap32(bitmap32);
	//保存
	FilePNG filePng;
	filePng.encodeFrom(bitmap32,bitDepth,hasPalette,hasColor,hasAlpha,colorsList);
	filePng.saveFilePNG(filename.toStdString());
	//内存回收
	bitmap32.deleteBitmap();
	filePng.deleteDataPointer(true);
}

void Widget_Image::loadFileBmp(const QString &filename){
	//加载文件并分析
	FileBMP fileBmp;
	fileBmp.loadFile(filename.toStdString());
	fileBmp.parseData();
	//进行解码
	Bitmap_32bit bitmap32;
	fileBmp.decodeTo(bitmap32);
	bitmap32.coordinateType=Bitmap_32bit::CoordinateType_Screen;//Qt使用屏幕坐标系
	//生成图像用于显示
	fromBitmap32(bitmap32);
	update();
	//生成色表
	makeColorsList(fileBmp);
	//内存回收
	bitmap32.deleteBitmap();
	fileBmp.deleteDataPointer();
}
void Widget_Image::saveFileBmp(const QString &filename,uint16 bitCount,List<uint32> *colorsList)const{
	//转换成Bitmap32格式
	Bitmap_32bit bitmap32;
	toBitmap32(bitmap32);
	//保存
	FileBMP fileBmp;
	fileBmp.encodeFrom(bitmap32,bitCount,colorsList);
	fileBmp.saveFileBMP(filename.toStdString());
	//内存回收
	bitmap32.deleteBitmap();
	fileBmp.deleteDataPointer(true);
}

void Widget_Image::fromBitmap32(const Bitmap_32bit &bitmap32){
	auto w=bitmap32.getWidth(),h=bitmap32.getHeight();
	uint32 color32=0;
	image=QImage(w,h,QImage::Format_RGBA8888);
	for(decltype(h) y=0;y<h;++y){
		for(decltype(w) x=0;x<w;++x){
			bitmap32.getColor(x,y,color32);
			image.setPixel(x,y,uint2Qrgb(color32));
		}
	}
}
void Widget_Image::toBitmap32(Bitmap_32bit &bitmap32)const{
	auto w=image.width(),h=image.height();
	bitmap32.newBitmap(w,h);
	uint32 color32;
	for(decltype(h) y=0;y<h;++y){
		for(decltype(w) x=0;x<w;++x){
			color32=qRgb2uint32(image.pixel(x,y));
			bitmap32.setColor(x,y,color32);
		}
	}
}

void Widget_Image::makeColorsList(const QImage &image){
	colorsList.clear();
	if(HAS_COLOR_TABLE){//使用QImage的色表图
		auto colorTable=image.colorTable();
		for(auto rgb:colorTable){
			colorsList.push_back(qRgb2uint32(rgb));
		}
	}else{//统计QImage中的颜色数
		uint32 u32;
		FOREACH_PIXEL(
			u32=qRgb2uint32(image.pixel(x,y));
			if(!colorsList.contain(u32))colorsList.push_back(u32);
		);
	}
}
void Widget_Image::parsePaletteCode(){
	/*int colorIndex=0;
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
	}*/
}

#define CHANGE_COLOR(name) \
color.set##name(Number::divideRound(q##name(rgba) * plte.brightness * gray,0xFF * plte.maxGray));\

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
		*colorsList.data(index)=qColor2uint32(destColor);//save new color
	}
	return ok;
}
bool Widget_Image::changeColor(int index,const QColor &destColor,const QRect &rect){
	if(index>=colorsList.size())return false;
	auto srcColor=*colorsList.data(index);
	FOREACH_PIXEL_IN_RECT(rect.left(),rect.top(),rect.right()+1,rect.bottom()+1,
		if(qRgb2uint32(image.pixel(x,y))==srcColor){
			image.setPixel(x,y,destColor.rgba());
		}
	);
	return true;
}

void Widget_Image::startFlash(int index){
	if(VALID_COLOR_INDEX(index)){
		auto clr=colorsList.data(index);
		qDebug()<<hex<<*clr;
		startFlash(uint2QColor(*clr));
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
	FOREACH_PIXEL(if(image.pixelColor(x,y)==flashColor)flashImage.setPixel(x,y,1););

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
				image.setPixel(cursorPos,penColor.rgba());
			}
		break;
	}
}
void Widget_Image::mouseMoveEvent(QMouseEvent *ev){
	if(image.isNull())return;
	mousePos=ev->pos();
	QPoint p=mouseToImage(mousePos);
	QColor color=image.pixelColor(p);
	auto index=colorsList.data(qColor2uint32(color));
	//绘图
	if(cursorPos!=p){
		cursorPos=p;
		if(ev->buttons()==Qt::LeftButton){
			image.setPixel(cursorPos,penColor.rgba());
		}
		update();
	}
	//show tip
	if(index){
		QToolTip::showText(ev->globalPos(),
			QString::asprintf("(%d,%d)==(%.2X,%.2X,%.2X,a=%.2X),index==%d",
				p.x(),p.y(),
				color.red(),color.green(),color.blue(),color.alpha(),
				index));
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
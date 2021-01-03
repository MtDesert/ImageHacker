#include "Widget_Image.h"
#include"TableView_Color.h"
#include"common.h"

#include"FileStructs/FilePNG.h"
#include"FileStructs/FileBMP.h"
#include"Number.h"

#include<QToolTip>
#include<QMouseEvent>
#include<QPaintEvent>
#include<QPainter>
#include<QDebug>

#define HAS_COLOR_TABLE image.colorCount()>0
#define VALID_COLOR_INDEX(index) (index>=0 && index<colorsList.size())

Widget_Image::Widget_Image(QWidget *parent):QWidget(parent),
	flashTimerID(0),flashStatus(false),
	tolerance(0),editMode(Edit_Pen),
	tableViewColor(NULL){
	setMouseTracking(true);
}

void Widget_Image::loadImage(const QString &filename){
	if(image.load(filename)){
		resize(image.size());
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
		chunk.memoryAllocate(12+len);//size(4)+name(4)+data(size)+crc(4)
		chunk.setChunkLength(len);
		chunk.setChunkName("tEXt");
		memcpy(chunk.chunkDataBlock().dataPointer,code.toStdString().data(),len);
		chunk.setChunkCRC(chunk.makeCRC());
		filePng.allChunks.push_back(&chunk);
	}

	//re-save file
	filePng.saveFilePNG(filename.toStdString());
	chunk.memoryFree();
	filePng.memoryFree();
}

void Widget_Image::loadFilePng(const FilePNG &filePng){
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
}

void Widget_Image::forEachPixel(const QRect &rect,function<void(int,int,QColor&)> pixelFunc){
	QColor color;
	for(int x=rect.left();x<=rect.right() && x<image.width();++x){
		for(int y=rect.top();y<=rect.bottom() && y<image.height();++y){
			color = image.pixelColor(x,y);
			pixelFunc(x,y,color);
		}
	}
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
	filePng.memoryFree(true);
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
	fileBmp.memoryFree();
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
	fileBmp.memoryFree(true);
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
		forEachPixel(image.rect(),[&](int x,int y,QColor &color){
			u32=qColor2uint32(color);
			if(!colorsList.contain(u32))colorsList.push_back(u32);
		});
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
	forEachPixel(image.rect(),[&](int x,int y,QColor &color){
		if(qColor2uint32(color)==srcColor){
			image.setPixel(x,y,destColor.rgba());
		}
	});
	return true;
}
bool Widget_Image::changeColor(const QColor &srcColor,const QColor &destColor){
	bool ret=false;
	forEachPixel(image.rect(),[&](int x,int y,QColor &color){
		if(color==srcColor){
			ret=true;
			image.setPixelColor(x,y,destColor);
		}
	});
	return ret;
}
bool Widget_Image::changeColorByColorsList(){
	if(colorsList.size()<=0)return false;
	double *distances=new double[colorsList.size()];
	//准备缓冲
	ColorRGBA rgbaA,rgbaB;
	uint32 u32;
	auto idx=0,minIdx=0;
	double minimum=512;
	forEachPixel(image.rect(),[&](int x,int y,QColor &color){
		rgbaA=qColor2ColorRGBA(color);
		idx=0,minIdx=0;
		minimum=512;
		for(const auto &u32:colorsList){//计算所有特征
			rgbaB.fromRGBA(u32);
			distances[idx]=rgbaA.distance(rgbaB);
			//寻找最小值
			if(distances[idx]<minimum){
				minIdx=idx;
				minimum=distances[idx];
			}
			//下一个
			++idx;
		}
		//改写数据
		u32=*(colorsList.data(minIdx));
		rgbaB.fromRGBA(u32);
		image.setPixelColor(x,y,rgba2QColor(rgbaB));
	});
	//结束
	delete []distances;
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
	forEachPixel(image.rect(),[&](int x,int y,QColor &color){
		if(color==flashColor)flashImage.setPixel(x,y,1);
	});

	//start flash
	stopFlash();
	flashTimerID=startTimer(500);
}
void Widget_Image::stopFlash(){killTimer(flashTimerID);}

//各种画图编辑函数
const QPoint directions[4]={QPoint(-1,0),QPoint(1,0),QPoint(0,-1),QPoint(0,1)};
void Widget_Image::drawPoint(const QPoint &pos){
	if(HAS_COLOR_TABLE){//index
		auto table=image.colorTable();
		auto index=table.indexOf(penColor.rgba());
		if(!VALID_COLOR_INDEX(index))return;
		//draw
		image.setPixel(pos,index);
	}else{//non-index
		image.setPixel(pos,penColor.rgba());
	}
}
void Widget_Image::fillPoints(function<bool(const ColorRGBA &color)> condition){
	QList<QPoint> ptList;
	QPoint currentP,nearP;
	ColorRGBA color;
	ptList.push_back(cursorPos);
	drawPoint(cursorPos);
	for(int i=0;i<ptList.size();++i){
		currentP=ptList[i];
		for(int idx=0;idx<4;++idx){
			//判断点是否在范围内
			nearP = currentP + directions[idx];//得到相邻点
			if(nearP.x()<0||nearP.y()<0)continue;
			if(nearP.x()>=image.width()||nearP.y()>=image.height())continue;
			if(ptList.contains(nearP))continue;
			//判断点是否需要处理
			color=qRgb2ColorRGBA(image.pixel(nearP));
			if(condition(color)){
				drawPoint(nearP);
				ptList.push_back(nearP);
			}
		}
	}
	update();
}
void Widget_Image::fillByBorder(const QPoint &pos){fillByBorder(pos,penColor,borderColor);}
void Widget_Image::fillByBorder(const QPoint &pos,const QColor &fillColor,const QColor &borderColor){
	QList<QPoint> allPoints;
	allPoints.push_back(pos);
	QPoint p;QColor color;
	while(allPoints.size()){
		//处理当前坐标
		p=allPoints.takeFirst();
		color=image.pixelColor(p);
		if(color==fillColor||color==borderColor)continue;//不需要改变,不处理
		image.setPixelColor(p,fillColor);
		//处理周围坐标
		for(int idx=0;idx<4;++idx){
			allPoints.push_back(p + directions[idx]);
		}
	}
	update();
}
void Widget_Image::quantify(int level){
	if(level<0||level>0xFF)return;//验证
	double base=255.0/level;
	//开始量化
	int w=image.width(),h=image.height(),lv=0;
	QColor color;
	for(int x=0;x<w;++x){
		for(int y=0;y<h;++y){
			color=image.pixelColor(x,y);
#define QUANTIFY(name,Name)\
lv=round(color.name()/base);\
color.set##Name(round(base*lv));
			QUANTIFY(red,Red)
			QUANTIFY(green,Green)
			QUANTIFY(blue,Blue)
#undef QUANTIFY
			image.setPixelColor(x,y,color);
		}
	}
	update();
}
void Widget_Image::stroke(const QColor &bgColor, const QColor &strokeColor){
	forEachPixel(image.rect(),[&](int x,int y,QColor &color){
		if(color==bgColor||color==strokeColor)return;//从物体开始计算
		//开始绘制
		const int len=3;
		QColor clr;
		for(int a=x-len;a<=x+len;++a){
			for(int b=y-len;b<=y+len;++b){
				//过滤
				if(a<0||b<0||a>=image.width()||b>=image.height())continue;
				if(abs(a-x)+abs(b-y)>len)continue;
				//开始修改
				clr=image.pixelColor(a,b);
				if(clr==bgColor){
					image.setPixelColor(a,b,strokeColor);
				}
			}
		}
	});
}
//各种事件函数
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
			switch(editMode){
				case Edit_Pen:drawPoint(cursorPos);break;
				case Edit_Fill:{//只把连通的原色改变成目标色
					auto srcColor=qRgb2ColorRGBA(image.pixel(cursorPos));
					fillPoints([=](const ColorRGBA &color){
						return srcColor==color;
					});
				}break;
				case Edit_FillByOrgColor:{
				}break;
				case Edit_FillByBorder:{
					fillByBorder(cursorPos);
				}break;
			}
		break;
		case Qt::RightButton:
			if(!tableViewColor){
				tableViewColor=new TableView_Color();
				tableViewColor->setWindowTitle(windowTitle());
				tableViewColor->setWidgetImage(*this);
			}
			tableViewColor->show();
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
			drawPoint(cursorPos);
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
void Widget_Image::closeEvent(QCloseEvent *ev){
	if(tableViewColor){
		tableViewColor->close();
		tableViewColor->deleteLater();
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
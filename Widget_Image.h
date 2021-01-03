#ifndef WIDGET_IMAGE_H
#define WIDGET_IMAGE_H

#include<QScrollArea>
#include"FileStructs/FilePNG.h"
#include"FileStructs/FileBMP.h"

class TableView_Color;
class Widget_Image:public QWidget{
	Q_OBJECT
public:
	explicit Widget_Image(QWidget *parent=NULL);

	//文件读写
	void loadImage(const QString &filename);
	void saveImage(const QString &filename,const QString &code)const;
	void makeColorsList(const QImage &image);//根据图像image来创建色表,存入colorsList
	//PNG文件读写
	void loadFilePng(const FilePNG &filePng);
	void saveFilePng(const QString &filename,uint8 bitDepth,bool hasPalette,bool hasColor,bool hasAlpha,List<uint32> *colorsList=nullptr)const;
	//BMP文件读写
	void loadFileBmp(const QString &filename);
	void saveFileBmp(const QString &filename,uint16 bitCount,List<uint32> *colorsList=nullptr)const;

	//颜色遍历
	void forEachPixel(const QRect &rect,function<void(int,int,QColor&)> pixelFunc);
	//创建色表
	void makeColorsList(const FilePNG &filePng);
	void makeColorsList(const FileBMP &fileBmp);
	//颜色表读写
	bool imageColor(int index,QColor &color)const;
	bool changeColor(int index,const QColor &destColor);
	bool changeColor(int index,const QColor &destColor,const QRect &rect);
	bool changeColor(const QColor &srcColor,const QColor &destColor);
	bool changeColorByColorsList();

	//转换
	void fromBitmap32(const Bitmap_32bit &bitmap32);
	void toBitmap32(Bitmap_32bit &bitmap32)const;

	//闪烁功能
	void startFlash(int index);
	void startFlash(const QColor &flashColor);
	void stopFlash();

	//数据
	QImage image;//图像
	List<uint32> colorsList;//颜色表(可编辑,用于指定输出格式)
	//编辑
	QColor penColor;//铅笔颜色
	double tolerance;//容差,模糊填充的时候用
	QColor borderColor;//边界颜色,模糊填充时候用
	enum EditMode{
		Edit_Pen,//铅笔做图,描点用
		Edit_Fill,//填充,最基本的同色填充
		Edit_FillByOrgColor,//填充,把本色变暗的部分也进行填充
		Edit_FillByBorder//边界填充
	};
	EditMode editMode;
	void drawPoint(const QPoint &pos);
	void fillPoints(function<bool(const ColorRGBA &)> condition);
	void fillByBorder(const QPoint &pos);
	void fillByBorder(const QPoint &pos,const QColor &fillColor,const QColor &borderColor);
	//调色板
	QString paletteCode;
	//量化
	void quantify(int level);
	//描边
	void stroke(const QColor &bgColor,const QColor &strokeColor);

	//辅助控件
	TableView_Color *tableViewColor;
protected:
	int flashTimerID;
	bool flashStatus;
	QImage flashImage;

	QPoint mousePos;
	QPoint cursorPos;

	void timerEvent(QTimerEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void paintEvent(QPaintEvent*);
	void closeEvent(QCloseEvent*);

	QPoint mouseToImage(const QPoint &p)const;
	QPoint imageToMouse(const QPoint &p)const;
};
#endif // WIDGET_IMAGE_H
#ifndef WIDGET_IMAGE_H
#define WIDGET_IMAGE_H

#include<QWidget>
#include"FileStructs/FilePNG.h"
#include"FileStructs/FileBMP.h"

struct Palette{
	Palette();
	QString name;
	QList<int> allIndexes;
	int maxGray;
	QColor color;
	int brightness;
};

class Widget_Image:public QWidget
{
public:
	Widget_Image(QWidget *parent=0);

	//文件读写
	void loadImage(const QString &filename);//parameter code for output
	void saveImage(const QString &filename,const QString &code)const;
	void makeColorsList(const QImage &image);//根据图像image来创建色表,存入colorsList
	//PNG文件读写
	void loadFilePng(const FilePNG &filePng);
	void saveFilePng(const QString &filename,uint8 bitDepth,bool hasPalette,bool hasColor,bool hasAlpha,List<uint32> *colorsList=nullptr)const;
	//BMP文件读写
	void loadFileBmp(const QString &filename);
	void saveFileBmp(const QString &filename,uint16 bitCount,List<uint32> *colorsList=nullptr)const;
	//创建色表
	void makeColorsList(const FilePNG &filePng);
	void makeColorsList(const FileBMP &fileBmp);

	//转换
	void fromBitmap32(const Bitmap_32bit &bitmap32);
	void toBitmap32(Bitmap_32bit &bitmap32)const;

	bool imageColor(int index,QColor &color)const;

	//颜色变换
	bool changeColor(int index,const QColor &destColor);
	bool changeColor(int index,const QColor &destColor,const QRect &rect);

	//闪烁功能
	void startFlash(int index);
	void startFlash(const QColor &flashColor);
	void stopFlash();

	//调色板代码
	void parsePaletteCode();
	void updateByPalette(const Palette &plte);
	void updateByAllPalettes();

	//数据
	QImage image;//图像
	List<uint32> colorsList;//颜色表(可编辑,用于指定输出格式)
	//编辑
	QColor penColor;
	//调色板
	QString paletteCode;
	QList<Palette> paletteList;
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

	QPoint mouseToImage(const QPoint &p)const;
	QPoint imageToMouse(const QPoint &p)const;
};

#endif // WIDGET_IMAGE_H
#ifndef WIDGET_IMAGE_H
#define WIDGET_IMAGE_H

#include<QWidget>

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

	void loadImage(const QString &filename);//parameter code for output
	void saveImage(const QString &filename,const QString &code)const;

	QColor makeImage(const QImage &fromImage);//if return value is valid, it means error
	void makeColorList(const QImage &image);

	bool imageColor(int index,QColor &color)const;//get color from image.colorTable,return whether success

	bool changeColor(int index,const QColor &destColor);
	bool changeColor(int index,const QColor &destColor,const QRect &rect);

	void startFlash(int index);
	void startFlash(const QColor &flashColor);
	void stopFlash();

	void parsePaletteCode();
	void updateByPalette(const Palette &plte);
	void updateByAllPalettes();

	//data
	QImage image;
	QList<QColor> colorList;

	//edit
	QColor penColor;

	//palette
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
#ifndef DIALOG_SAVEPNG_H
#define DIALOG_SAVEPNG_H

#include "ui_Dialog_SavePNG.h"

class Dialog_SavePNG:public QDialog,public Ui::Dialog_SavePNG{
	Q_OBJECT
public:
	explicit Dialog_SavePNG(QWidget *parent = 0);

	uchar colorType;//颜色类型,并非所有的值都是有效的
	void checkColorType();
public slots:
	void on_pushButton_OutputFilename_clicked();

	void on_checkBox_hasPalette_clicked(bool checked);
	void on_checkBox_hasColor_clicked(bool checked);
	void on_checkBox_hasAlpha_clicked(bool checked);
};
#endif
#ifndef DIALOG_SAVEBMP_H
#define DIALOG_SAVEBMP_H

#include "ui_Dialog_SaveBMP.h"

class Dialog_SaveBMP:public QDialog,public Ui::Dialog_SaveBMP{
	Q_OBJECT
public:
	explicit Dialog_SaveBMP(QWidget *parent=0);
public slots:
	void on_pushButton_OutputFilename_clicked();
	void on_spinBox_bitDepth_valueChanged(int i);
};

#endif // DIALOG_SAVEBMP_H
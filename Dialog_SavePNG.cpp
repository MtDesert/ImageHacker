#include"Dialog_SavePNG.h"
#include"FileStructs/FilePNG.h"

#include<QFileDialog>
#include<QDebug>

Dialog_SavePNG::Dialog_SavePNG(QWidget *parent):QDialog(parent){
	setupUi(this);
	colorType=0;
	checkColorType();
}

void Dialog_SavePNG::checkColorType(){
	checkBox_useCustomColorsList->setEnabled(checkBox_hasPalette->isChecked());
	label_ColorTypeWarning->setVisible(!FilePNG_IHDR::isValid_ColorType(colorType));
}

void Dialog_SavePNG::on_pushButton_OutputFilename_clicked(){
	auto filename=QFileDialog::getSaveFileName(this,tr("保存为PNG格式"));
	if(!filename.isEmpty()){
		lineEdit_OutputFilename->setText(filename);
	}
}
void Dialog_SavePNG::on_checkBox_hasPalette_clicked(bool checked){
	checked ? colorType|=1 : colorType&=0x06;
	checkColorType();
}
void Dialog_SavePNG::on_checkBox_hasColor_clicked(bool checked){
	checked ? colorType|=2 : colorType&=0x05;
	checkColorType();
}
void Dialog_SavePNG::on_checkBox_hasAlpha_clicked(bool checked){
	checked ? colorType|=4 : colorType&=0x03;
	checkColorType();
}
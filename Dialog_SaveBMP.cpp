#include "Dialog_SaveBMP.h"
#include<QFileDialog>
#include"FileStructs/FileBMP.h"

Dialog_SaveBMP::Dialog_SaveBMP(QWidget *parent):QDialog(parent){
	setupUi(this);
}

void Dialog_SaveBMP::on_pushButton_OutputFilename_clicked(){
	auto filename=QFileDialog::getSaveFileName(this,tr("保存为BMP格式"));
	if(!filename.isEmpty()){
		lineEdit_OutputFilename->setText(filename);
	}
}
void Dialog_SaveBMP::on_spinBox_bitDepth_valueChanged(int i){
	label_ColorTypeWarning->setVisible(!FileBMP_InfoHeader::isValid_BitCount(i));
}
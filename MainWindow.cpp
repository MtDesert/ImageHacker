#include"MainWindow.h"
#include"Dialog_SaveBMP.h"
#include"Dialog_SavePNG.h"
#include"TableView_Color.h"

#include"common.h"

#include<QFileDialog>
#include<QColorDialog>
#include<QInputDialog>
#include<QMessageBox>
#include<QPainter>

#include<QKeyEvent>
#include<QDir>
#include<QDebug>

#define SET_MODEL(name) tableView_##name->setModel(&tableModel_##name);

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent){
	setupUi(this);
	widget_SrcImage=new Widget_Image();
	widget_MaskImage=new Widget_Image();
	widget_DestImage=new Widget_Image();
	//table model(png)
	tableModel_PNG_Chunk.filePng=&filePng;
	//table view
	SET_MODEL(PNG_Chunk)
	SET_MODEL(PNG_IHDR)
	SET_MODEL(PNG_PLTE)
	//debug
#define IMG(name)\
	widget_##name##Image=new Widget_Image();\
	scrollArea_##name##Image->setWidget(widget_##name##Image);
	IMG(Src)
	IMG(Mask)
	IMG(Dest)
}

void MainWindow::on_actionImage_load_triggered(){
	QString filename=QFileDialog::getOpenFileName(this,tr("Original Image"));
	if(!filename.isEmpty()){
		//加载图像
		widget_SrcImage->loadImage(filename);
		widget_MaskImage->loadImage(filename);
		widget_DestImage->loadImage(filename);
	}
}

void MainWindow::on_actionImage_loadBMP_triggered(){
	QString filename=QFileDialog::getOpenFileName(this,tr("Original Image"),QString(),QString("*.bmp"));
	if(!filename.isEmpty()){
		//显示原图和色表
		widget_SrcImage->loadFileBmp(filename);
		afterLoadImage();
		//设定目标图
		widget_DestImage->image=widget_SrcImage->image;
		//设置目标色表
		if(widget_SrcImage->colorsList.size()>0){
			widget_DestImage->colorsList=widget_SrcImage->colorsList;
		}else{
			widget_DestImage->makeColorsList(widget_DestImage->image);
		}
	}
}
void MainWindow::on_actionImage_loadPNG_triggered(){
	QString filename=QFileDialog::getOpenFileName(this,tr("Original Image"),"","PNG Image(*.png)");
	if(!filename.isEmpty()){
		//载入PNG并分析
		filePng.memoryFree();
		filePng.loadFile(filename.toStdString());
		filePng.parseData();
		//显示分析结果
		tableModel_PNG_IHDR.ihdr=filePng.findIHDR();
		tableModel_PNG_PLTE.plte=filePng.findPLTE();
		tableModel_PNG_Chunk.reset();
		tableModel_PNG_IHDR.reset();
		tableModel_PNG_PLTE.reset();
		//显示原图和色表
		widget_SrcImage->loadFilePng(filePng);
		afterLoadImage();
	}
}
void MainWindow::on_actionImage_save_triggered(){
	QString filename=QFileDialog::getSaveFileName(this,tr("Save Image"),"","PNG Image(*.png)");
	if(!filename.isEmpty()){
		widget_DestImage->image.save(filename);
	}
}
void MainWindow::on_actionMask_Open_triggered(){
	QString filename=QFileDialog::getOpenFileName(this,tr("Mask Image"));
	if(!filename.isEmpty()){
		widget_MaskImage->loadImage(filename);
		widget_MaskImage->makeColorsList(widget_MaskImage->image);
	}
}
void MainWindow::on_actionImage_save_palette_triggered(){
	QString filename=QFileDialog::getSaveFileName(this,tr("Save Palette Image"),"","PNG Image(*.png)");
	if(!filename.isEmpty()){
		widget_DestImage->saveImage(filename,textEdit_Code->toPlainText());
	}
}
void MainWindow::on_actionImage_saveBMP_triggered(){
	Dialog_SaveBMP dialog;
	int answer=dialog.exec();
	if(answer==QDialog::Accepted){
		widget_SrcImage->saveFileBmp(dialog.lineEdit_OutputFilename->text(),
			dialog.spinBox_bitDepth->value(),
			dialog.checkBox_useCustomColorsList->isChecked()?&widget_DestImage->colorsList:nullptr);
	}
}
void MainWindow::on_actionImage_savePNG_triggered(){
	Dialog_SavePNG dialog;
	int answer=dialog.exec();
	if(answer==QDialog::Accepted){
		widget_DestImage->saveFilePng(dialog.lineEdit_OutputFilename->text(),
			dialog.spinBox_bitDepth->value(),
			dialog.checkBox_hasPalette->isChecked(),
			dialog.checkBox_hasColor->isChecked(),
			dialog.checkBox_hasAlpha->isChecked(),
			dialog.checkBox_useCustomColorsList->isChecked()?&widget_DestImage->colorsList:nullptr);
	}
}
void MainWindow::on_actionExit_triggered(){close();}

void MainWindow::on_actionPaint_Pen_triggered(){
	widget_DestImage->editMode=Widget_Image::Edit_Pen;
	auto color=QColorDialog::getColor(widget_DestImage->penColor,this,tr("设置铅笔颜色"),
		QColorDialog::ShowAlphaChannel|QColorDialog::DontUseNativeDialog);
	if(color.isValid()){
		widget_DestImage->penColor=color;
	}
}
void MainWindow::on_actionPaint_Fill_triggered(){widget_DestImage->editMode=Widget_Image::Edit_Fill;}
void MainWindow::on_actionPaint_FillByOrgColor_triggered(){
	widget_DestImage->editMode=Widget_Image::Edit_FillByOrgColor;
	bool ok;//需要设置容差
	auto tolerance=QInputDialog::getDouble(this,tr("色偏容差"),tr("容差越小,颜色偏差越大的值越不容易连通"),widget_DestImage->tolerance,0,999,1,&ok);
	if(ok){
		widget_SrcImage->tolerance=tolerance;
	}
}
void MainWindow::on_actionPaint_FillByBorder_triggered(){
	auto fillColor=QColorDialog::getColor(widget_DestImage->penColor,this,tr("设置填充颜色"),
		QColorDialog::ShowAlphaChannel|QColorDialog::DontUseNativeDialog);
	if(!fillColor.isValid())return;
	auto borderColor=QColorDialog::getColor(widget_DestImage->borderColor,this,tr("设置边界颜色"),
		QColorDialog::ShowAlphaChannel|QColorDialog::DontUseNativeDialog);
	if(!borderColor.isValid())return;
	//写入
	widget_DestImage->editMode=Widget_Image::Edit_FillByBorder;
	widget_DestImage->penColor=fillColor;
	widget_DestImage->borderColor=borderColor;
}
void MainWindow::on_actionPaint_Quantify_triggered(){
	bool ok;
	auto value=QInputDialog::getInt(this,tr("量化最大值"),tr("将RGB按照[0~量化最大值]进行量化"),1,1,255,1,&ok);
	if(ok){
		widget_DestImage->quantify(value);
	}
}
void MainWindow::afterLoadImage(){
	//设定目标图
	widget_DestImage->image=widget_SrcImage->image;
	//设置目标色表
	if(widget_SrcImage->colorsList.size()>0){
		widget_DestImage->colorsList=widget_SrcImage->colorsList;
	}else{
		widget_DestImage->makeColorsList(widget_DestImage->image);
	}
}
#include"MainWindow.h"
#include"Dialog_SavePNG.h"
#include"FileStructs/FilePNG.h"

#include"common.h"

#include<QFileDialog>
#include<QColorDialog>
#include<QInputDialog>
#include<QMessageBox>

#include<QKeyEvent>
#include<QDebug>

struct TestFile{
	QString filename;
	int bitDepth;
	bool hasPalette,hasColor,hasAlpha;
};
TestFile allFiles[]={
	{"0g01",1,false,false,false},
	{"0g02",2,false,false,false},
	{"0g04",4,false,false,false},
	{"0g08",8,false,false,false},
	{"0g16",16,false,false,false},
	{"2c08",8,false,true,false},
	{"2c16",16,false,true,false},
	{"3p01",1,true,true,false},
	{"3p02",2,true,true,false},
	{"3p04",4,true,true,false},
	{"3p08",8,true,true,false},
	/*{"4a08",8,false,false,true},
	{"4a16",16,false,false,true},
	{"6a08",8,false,true,true},
	{"6a16",16,false,true,true},*/
};

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent){
	setupUi(this);
	//color table
	tableModel_SrcColor.colorList=&widget_SrcImage->colorsList;
	tableModel_DestColor.colorList=&widget_DestImage->colorsList;
	tableModel_Palette.paletteList=&widget_DestImage->paletteList;

	tableView_SrcColor->setModel(&tableModel_SrcColor);
	tableView_DestColor->setModel(&tableModel_DestColor);
	tableView_Palette->setModel(&tableModel_Palette);
	//debug
	/*auto amount=sizeof(allFiles)/sizeof(TestFile);
	for(auto i=0;i<amount;++i){
		auto &testFile=allFiles[i];
		widget_SrcImage->loadFilePng("~/图片/png/basn"+testFile.filename+".png");
		widget_SrcImage->saveFilePng("~/图片/pngOut/"+testFile.filename+".png",testFile.bitDepth,testFile.hasPalette,testFile.hasColor,testFile.hasAlpha);
		//break;
	}*/
}

void MainWindow::on_actionImage_load_triggered(){
	QString filename=QFileDialog::getOpenFileName(this,tr("Original Image"));
	if(!filename.isEmpty()){
		//image
		widget_SrcImage->loadImage(filename);
		widget_DestImage->makeColorsList(widget_SrcImage->image);
		//code
		widget_DestImage->paletteCode=widget_SrcImage->paletteCode;
		widget_DestImage->parsePaletteCode();
		//table model
		tableModel_SrcColor.reset();
		tableModel_DestColor.reset();
		tableModel_Palette.reset();
		textEdit_Code->setPlainText(widget_SrcImage->paletteCode);
	}
}
void MainWindow::on_actionImage_loadPNG_triggered(){
	QString filename=QFileDialog::getOpenFileName(this,tr("Original Image"));
	if(!filename.isEmpty()){
		//显示原图和色表
		widget_SrcImage->loadFilePng(filename);
		tableModel_SrcColor.reset();
		//设定目标图
		widget_DestImage->image=widget_SrcImage->image;
		//设置目标色表
		if(widget_SrcImage->colorsList.size()>0){
			widget_DestImage->colorsList=widget_SrcImage->colorsList;
		}else{
			widget_DestImage->makeColorsList(widget_SrcImage->image);
		}
		//更新TableModel数据
		tableModel_SrcColor.reset();
		tableModel_DestColor.reset();
		tableModel_Palette.reset();
		//textEdit_Code->setPlainText(widget_SrcImage->paletteCode);
	}
}
void MainWindow::on_actionImage_save_triggered(){
	QString filename=QFileDialog::getSaveFileName(this,tr("Save Image"),"","PNG Image(*.png)");
	if(!filename.isEmpty()){
		widget_DestImage->image.save(filename);
	}
}
void MainWindow::on_actionImage_save_palette_triggered(){
	QString filename=QFileDialog::getSaveFileName(this,tr("Save Palette Image"),"","PNG Image(*.png)");
	if(!filename.isEmpty()){
		widget_DestImage->saveImage(filename,textEdit_Code->toPlainText());
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

#define IF_VALID_SRC_INDEX \
auto index=tableView_SrcColor->currentIndex();\
if(!index.isValid())return;\

#define IF_VALID_DEST_INDEX \
auto index=tableView_DestColor->currentIndex();\
if(!index.isValid())return;\

void MainWindow::on_tableView_SrcColor_activated(const QModelIndex &index){
	widget_SrcImage->startFlash(index.row());
}
void MainWindow::on_tableView_SrcColor_pressed(const QModelIndex &index){
	if(QApplication::mouseButtons()!=Qt::RightButton)return;
	QMenu menu(tableView_SrcColor);
	menu.addAction(actionSrcTable_Insert);
	menu.addAction(actionSrcTable_Delete);
	menu.addAction(actionSrcTable_Edit);
	menu.exec(QCursor::pos());
}
void MainWindow::on_actionSrcTable_Insert_triggered(){
	slotSrcTableColorCount(1);
}
void MainWindow::on_actionSrcTable_Delete_triggered(){
	slotSrcTableColorCount(-1);
}
void MainWindow::on_actionSrcTable_Edit_triggered(){
	IF_VALID_SRC_INDEX
	QColor color;
	int row=index.row();
	widget_SrcImage->imageColor(row,color);
	//change color
	color=QColorDialog::getColor(color,this,tr("Color"),
		QColorDialog::ShowAlphaChannel|
		QColorDialog::DontUseNativeDialog);
	//write
	if(color.isValid()){
		widget_SrcImage->image.setColor(row,color.rgba());
	}
}

void MainWindow::on_tableView_DestColor_activated(const QModelIndex &index){
	widget_DestImage->startFlash(index.row());
}
void MainWindow::on_tableView_DestColor_pressed(const QModelIndex &index){
	if(QApplication::mouseButtons()!=Qt::RightButton)return;
	QMenu menu(tableView_SrcColor);
	menu.addAction(actionDestTable_Insert);
	menu.addAction(actionDestTable_Delete);
	menu.addAction(actionDestTable_Edit);
	menu.exec(QCursor::pos());
}
void MainWindow::on_actionDestTable_Insert_triggered(){}
void MainWindow::on_actionDestTable_Delete_triggered(){}
void MainWindow::on_actionDestTable_Edit_triggered(){
	IF_VALID_DEST_INDEX
	int row=index.row();
	QColor color=uint2QColor(*widget_DestImage->colorsList.data(row));
	//change color
	color=QColorDialog::getColor(color,this,tr("Color"),
		QColorDialog::ShowAlphaChannel|
		QColorDialog::DontUseNativeDialog);
	//write
	if(color.isValid()){
		*widget_DestImage->colorsList.data(row)=qColor2uint32(color);
	}
}
void MainWindow::on_actionDestImage_Remake_triggered(){}

void MainWindow::on_tableView_Palette_activated(const QModelIndex &index){
	auto row=index.row();
	auto plte=widget_DestImage->paletteList[row];
	switch(index.column()){
		case 1:{
			auto color=QColorDialog::getColor(plte.color,this,tr("Color"),
				QColorDialog::ShowAlphaChannel|
				QColorDialog::DontUseNativeDialog);
			if(color.isValid()){
				widget_DestImage->paletteList[row].color=color;
			}
		}break;
		case 2:{
			bool ok;
			int value=QInputDialog::getInt(this,tr("Brightness"),tr("0~255"),plte.brightness,0,255,1,&ok);
			if(ok){
				widget_DestImage->paletteList[row].brightness=value;
			}
		}break;
	}
}
void MainWindow::on_actionPalette_MakePalette_triggered(){
	widget_DestImage->paletteCode=textEdit_Code->toPlainText();
	widget_DestImage->parsePaletteCode();
}
void MainWindow::on_actionPalette_MakeDestImage_triggered(){
	widget_DestImage->updateByAllPalettes();
}

void MainWindow::slotSrcTableColorCount(int delta){
	QImage &image(widget_SrcImage->image);
	image.setColorCount(image.colorCount()+delta);
	tableModel_SrcColor.reset();
}
void MainWindow::slotMoveUpDown(int delta){
	IF_VALID_DEST_INDEX
	auto row=index.row();
	auto row0=row+delta;
	if(row0>=0 && row0<widget_DestImage->colorsList.size()){
		//交换数据
		auto p=widget_DestImage->colorsList.data(row);
		auto p0=widget_DestImage->colorsList.data(row0);
		if(p && p0){
			auto tmp=*p;
			*p=*p0;
			*p0=tmp;
		}
		tableModel_DestColor.reset();
		//follow
		index=index.sibling(row0,index.column());
		tableView_DestColor->setCurrentIndex(index);
	}
}

void MainWindow::keyReleaseEvent(QKeyEvent *ev){
	switch(ev->key()){
		case Qt::Key_Minus:
			slotMoveUpDown(-1);
		break;
		case Qt::Key_Plus:
			slotMoveUpDown(1);
		break;
		default:qDebug()<<hex<<ev->key();
	}
	QMainWindow::keyReleaseEvent(ev);
}
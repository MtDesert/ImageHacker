#include"MainWindow.h"
#include"Dialog_SavePNG.h"
#include"FileStructs/FilePNG.h"

#include<QFileDialog>
#include<QColorDialog>
#include<QInputDialog>
#include<QMessageBox>

#include<QKeyEvent>
#include<QDebug>

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent){
	setupUi(this);
	//color table
	tableModel_SrcColor.image=&widget_SrcImage->image;
	tableModel_DestColor.image=&widget_DestImage->image;
	tableModel_DestColor.colorList=&widget_DestImage->colorsList;
	tableModel_Palette.paletteList=&widget_DestImage->paletteList;

	tableView_SrcColor->setModel(&tableModel_SrcColor);
	tableView_DestColor->setModel(&tableModel_DestColor);
	tableView_Palette->setModel(&tableModel_Palette);
}

void MainWindow::on_actionImage_load_triggered(){
	QString filename=QFileDialog::getOpenFileName(this,tr("Original Image"));
	if(!filename.isEmpty()){
		//image
		widget_SrcImage->loadImage(filename);
		widget_DestImage->makeColorsList(widget_SrcImage->image);
		widget_DestImage->makeImage(widget_SrcImage->image);
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
		//图像处理
		widget_SrcImage->loadFilePng(filename);
		/*widget_DestImage->makeColorsList(widget_SrcImage->image);
		widget_DestImage->makeImage(widget_SrcImage->image);
		//code
		widget_DestImage->paletteCode=widget_SrcImage->paletteCode;
		widget_DestImage->parsePaletteCode();
		//table model
		tableModel_SrcColor.reset();
		tableModel_DestColor.reset();
		tableModel_Palette.reset();
		textEdit_Code->setPlainText(widget_SrcImage->paletteCode);*/
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
			dialog.checkBox_hasAlpha->isChecked());
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
	QColor color=widget_DestImage->colorsList[row];
	//change color
	color=QColorDialog::getColor(color,this,tr("Color"),
		QColorDialog::ShowAlphaChannel|
		QColorDialog::DontUseNativeDialog);
	//write
	if(color.isValid()){
		widget_DestImage->colorsList[row]=color;
	}
}
void MainWindow::on_actionDestImage_Remake_triggered(){
	auto color=widget_DestImage->makeImage(widget_SrcImage->image);
	if(!color.isValid()){
		QMessageBox::information(this,tr("Success"),tr("nice!"));
	}else{
		QMessageBox::critical(this,tr("Failed"),QString::asprintf("r=%d,g=%d,b=%d,a=%d not found",
			color.red(),color.green(),color.blue(),color.alpha()));
	}
}

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
	widget_DestImage->makeImage(widget_SrcImage->image);
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
		widget_DestImage->colorsList.swap(row,row0);
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
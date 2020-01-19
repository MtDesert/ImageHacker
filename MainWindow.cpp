#include"MainWindow.h"
#include"Dialog_SaveBMP.h"
#include"Dialog_SavePNG.h"

#include"common.h"

#include<QFileDialog>
#include<QColorDialog>
#include<QInputDialog>
#include<QMessageBox>

#include<QKeyEvent>
#include<QDir>
#include<QDebug>

#define SET_MODEL(name) tableView_##name->setModel(&tableModel_##name);

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent){
	setupUi(this);
	//color table
	tableModel_SrcColor.colorList=&widget_SrcImage->colorsList;
	tableModel_DestColor.colorList=&widget_DestImage->colorsList;
	tableModel_Palette.paletteList=&widget_DestImage->paletteList;
	//table model(png)
	tableModel_PNG_Chunk.filePng=&filePng;
	//table view
	SET_MODEL(SrcColor)
	SET_MODEL(DestColor)
	SET_MODEL(Palette)
	SET_MODEL(PNG_Chunk)
	SET_MODEL(PNG_IHDR)
	SET_MODEL(PNG_PLTE)
}

void MainWindow::on_actionImage_load_triggered(){
	QString filename=QFileDialog::getOpenFileName(this,tr("Original Image"));
	if(!filename.isEmpty()){
		//加载图像
		widget_SrcImage->loadImage(filename);
		widget_DestImage->image=widget_SrcImage->image;
		widget_DestImage->makeColorsList(widget_DestImage->image);
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

void MainWindow::on_actionImage_loadBMP_triggered(){
	QString filename=QFileDialog::getOpenFileName(this,tr("Original Image"),QString(),QString("*.bmp"));
	if(!filename.isEmpty()){
		//显示原图和色表
		widget_SrcImage->loadFileBmp(filename);
		afterLoadImage();
		tableModel_SrcColor.reset();
		//设定目标图
		widget_DestImage->image=widget_SrcImage->image;
		//设置目标色表
		if(widget_SrcImage->colorsList.size()>0){
			widget_DestImage->colorsList=widget_SrcImage->colorsList;
		}else{
			widget_DestImage->makeColorsList(widget_DestImage->image);
		}
		//更新TableModel数据
		tableModel_SrcColor.reset();
		tableModel_DestColor.reset();
		tableModel_Palette.reset();
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
	widget_SrcImage->colorsList.push_back(0);
}
void MainWindow::on_actionSrcTable_Delete_triggered(){
	widget_SrcImage->colorsList.pop_back();
}
void MainWindow::on_actionSrcTable_Edit_triggered(){
	IF_VALID_SRC_INDEX
	int row=index.row();
	auto p=widget_SrcImage->colorsList.data(row);
	if(!p)return;
	//编辑颜色
	QColor color=uint2QColor(*p);
	color=QColorDialog::getColor(color,this,tr("Color"),
		QColorDialog::ShowAlphaChannel|
		QColorDialog::DontUseNativeDialog);
	//write
	if(color.isValid()){
		*widget_SrcImage->colorsList.data(row)=qColor2uint32(color);
	}
}

void MainWindow::on_tableView_DestColor_activated(const QModelIndex &index){
	widget_DestImage->startFlash(index.row());
	tableModel_DestColor.compareColor=uint2QColor(*widget_DestImage->colorsList.data(index.row()));
	tableModel_DestColor.reset();
}
void MainWindow::on_tableView_DestColor_pressed(const QModelIndex &index){
	if(QApplication::mouseButtons()!=Qt::RightButton)return;
	QMenu menu(tableView_SrcColor);
	menu.addAction(actionDestTable_Insert);
	menu.addAction(actionDestTable_Delete);
	menu.addAction(actionDestTable_Edit);
	menu.exec(QCursor::pos());
}
void MainWindow::on_actionDestTable_Insert_triggered(){
	IF_VALID_DEST_INDEX
	widget_DestImage->colorsList.insert(index.row(),0);
	tableModel_DestColor.reset();
}
void MainWindow::on_actionDestTable_Delete_triggered(){
	IF_VALID_DEST_INDEX
	widget_DestImage->colorsList.erase(index.row());
	tableModel_DestColor.reset();
}
void MainWindow::on_actionDestTable_Edit_triggered(){
	IF_VALID_DEST_INDEX
	int row=index.row();
	QColor oldColor=uint2QColor(*widget_DestImage->colorsList.data(row));
	//设置颜色
	QColor newColor=QColorDialog::getColor(oldColor,this,tr("Color"),
		QColorDialog::ShowAlphaChannel|
		QColorDialog::DontUseNativeDialog);
	//写入
	if(newColor.isValid()){
		int answer=QMessageBox::question(this,tr("Change color"),
			QString::asprintf("Change %s to %s?",
				oldColor.name(QColor::HexArgb).toStdString().data(),
				newColor.name(QColor::HexArgb).toStdString().data()));
		//批量变色
		if(answer==QMessageBox::Yes){
			widget_DestImage->changeColor(row,newColor);
		}
		*widget_DestImage->colorsList.data(row)=qColor2uint32(newColor);
	}
}
void MainWindow::on_actionDestTable_MoveUp_triggered(){
	IF_VALID_DEST_INDEX
	tableModel_DestColor.colorList->movePrev(index.row());
	tableModel_DestColor.reset();
}
void MainWindow::on_actionDestTable_MoveDown_triggered(){
	IF_VALID_DEST_INDEX
	tableModel_DestColor.colorList->moveNext(index.row());
	tableModel_DestColor.reset();
}
void MainWindow::on_actionDestTable_DeltaComp_triggered(){
	QColor color=QColorDialog::getColor(tableModel_DestColor.compareColor,this,tr("Compare Color"),
		QColorDialog::ShowAlphaChannel|QColorDialog::DontUseNativeDialog);
	if(color.isValid()){
		tableModel_DestColor.compareColor=color;
		tableModel_DestColor.reset();//计算差异值
	}
}
void MainWindow::on_actionDestTable_SortByDelta_triggered(){
	auto cmpColor=qColor2ColorRGBA(tableModel_DestColor.compareColor);
	ColorRGBA colorA,colorB;
	tableModel_DestColor.colorList->sort([&](const uint32 &a,const uint32 &b){
		colorA.fromRGBA(a);
		colorB.fromRGBA(b);
		return colorA.deltaSum(cmpColor) < colorB.deltaSum(cmpColor);
	});
	tableModel_DestColor.reset();
}
void MainWindow::on_actionDestTable_RemoveDelta_triggered(){
	bool ok;
	int tolerance=QInputDialog::getInt(this,tr("容差"),tr("请输入允许的容差"),0,0,255*3,1,&ok);
	if(ok){//开始进行调色
		ColorRGBA rgba,cmpColor=qColor2ColorRGBA(tableModel_DestColor.compareColor);
		int idx=0;
		for(auto &u32:widget_DestImage->colorsList){
			rgba.fromRGBA(u32);
			if(rgba.deltaSum(cmpColor)<=tolerance){
				widget_DestImage->changeColor(idx,tableModel_DestColor.compareColor);//改变图像
				u32=cmpColor.toRGBA();
			}
			//下一个
			++idx;
		}
		//去掉重复
		widget_DestImage->colorsList.unique();
		tableModel_DestColor.reset();
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

void MainWindow::afterLoadImage(){
	//设定目标图
	widget_DestImage->image=widget_SrcImage->image;
	//设置目标色表
	if(widget_SrcImage->colorsList.size()>0){
		widget_DestImage->colorsList=widget_SrcImage->colorsList;
	}else{
		widget_DestImage->makeColorsList(widget_DestImage->image);
	}
	//更新TableModel数据
	tableModel_SrcColor.reset();
	tableModel_DestColor.reset();
	tableModel_Palette.reset();
}
void MainWindow::keyReleaseEvent(QKeyEvent *ev){
	switch(ev->key()){
		case Qt::Key_Minus:
			slotMoveUpDown(-1);
		break;
		case Qt::Key_Equal:
			slotMoveUpDown(1);
		break;
		default:qDebug()<<hex<<ev->key();
	}
	QMainWindow::keyReleaseEvent(ev);
}
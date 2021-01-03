#include "TableView_Color.h"
#include"Widget_Image.h"
#include"common.h"

#include<QMouseEvent>
#include<QMenu>
#include<QInputDialog>
#include<QColorDialog>
#include<QMessageBox>
#include<QDebug>

#define COLOR_DIALOG_FLAG QColorDialog::ShowAlphaChannel|QColorDialog::DontUseNativeDialog

TableView_Color::TableView_Color(QWidget *parent):QTableView(parent),widgetImage(NULL){
	setModel(&tableModel);
	resize(800,600);
	setSelectionBehavior(QAbstractItemView::SelectItems);
	setSelectionMode(QAbstractItemView::SingleSelection);
	for(int i=2;i<=5;++i){
		setColumnWidth(i,144);
	}
}
void TableView_Color::setWidgetImage(Widget_Image &widget){
	widgetImage=&widget;
	if(widgetImage){
		tableModel.colorsList=&widgetImage->colorsList;
		tableModel.reset();
	}
}
int TableView_Color::currentIndexRow()const{
	auto index=currentIndex();
	return index.isValid() ? index.row() : 0;
}
void TableView_Color::keyReleaseEvent(QKeyEvent *ev){
	QTableView::keyReleaseEvent(ev);
	switch(ev->key()){
		case Qt::Key_Minus:slotMoveUp();break;
		case Qt::Key_Equal:slotMoveDown();break;
		default:;
	}
}
void TableView_Color::mousePressEvent(QMouseEvent *ev){
	QTableView::mousePressEvent(ev);
	qDebug()<<ev;
}
void TableView_Color::mouseReleaseEvent(QMouseEvent *ev){
	QTableView::mouseReleaseEvent(ev);
	if(ev->button()==Qt::RightButton){//弹出右键菜单
		QMenu menu(this);
#define MENU_ITEM(name,slotName) menu.addAction(tr(#name),this,SLOT(slot##slotName()));
		//添加删除
		MENU_ITEM(获取色表,MakeColorsList)
		MENU_ITEM(插入,Insert)
		MENU_ITEM(修改,Modify)
		MENU_ITEM(移除,Remove)
		MENU_ITEM(清空色表,ClearColorsList)
		//移动
		menu.addSeparator();
		MENU_ITEM(上移,MoveUp)
		MENU_ITEM(下移,MoveDown)
		//差异色处理
		menu.addSeparator();
		MENU_ITEM(差异比对,DeltaComp)
		MENU_ITEM(按差异排序,SortByDelta)
		MENU_ITEM(去差异化,RemoveDelta)
		MENU_ITEM(根据颜色表变色,ChangeColorByColorsList)
		menu.addSeparator();
		MENU_ITEM(描边,Stroke)
		menu.exec(QCursor::pos());
	}
}
//槽函数
#define COLORS_LIST tableModel.colorsList
#define REFRESH tableModel.reset();
void TableView_Color::slotMakeColorsList(){
	widgetImage->makeColorsList(widgetImage->image);
	REFRESH
}
void TableView_Color::slotInsert(){
	COLORS_LIST->insert(currentIndexRow(),0xFF000000);
	REFRESH
}
void TableView_Color::slotModify(){
	auto pU32=COLORS_LIST->data(currentIndexRow());
	if(!pU32)return;
	QColor srcColor=uint2QColor(*pU32);
	QColor destColor=QColorDialog::getColor(srcColor,this,tr("颜色修改"),COLOR_DIALOG_FLAG);
	if(destColor.isValid()){
		*pU32=qColor2uint32(destColor);//改变颜色
		//询问是否修改图像
		auto answer=QMessageBox::question(this,tr("是否修改图像颜色?"),tr("确认后,会将所有源颜色变换成目标颜色"));
		if(answer==QMessageBox::Yes){
			widgetImage->changeColor(srcColor,destColor);
			update();
		}
	}
}
void TableView_Color::slotRemove(){
	COLORS_LIST->erase(currentIndexRow());
	REFRESH
}
void TableView_Color::slotClearColorsList(){
	COLORS_LIST->clear();
	REFRESH
}
void TableView_Color::slotMoveUp(){
	auto idx=currentIndex();
	if(COLORS_LIST->movePrev(idx.row())){
		REFRESH
		setCurrentIndex(tableModel.index(idx.row()-1,idx.column()));
	}
}
void TableView_Color::slotMoveDown(){
	auto idx=currentIndex();
	if(COLORS_LIST->moveNext(idx.row())){
		REFRESH
		setCurrentIndex(tableModel.index(idx.row()+1,idx.column()));
	}
}
void TableView_Color::slotDeltaComp(){
	QColor color=QColorDialog::getColor(tableModel.compareColor,this,tr("颜色比对"),
		QColorDialog::ShowAlphaChannel|QColorDialog::DontUseNativeDialog);
	if(color.isValid()){
		tableModel.compareColor=color;
		tableModel.reset();//计算差异值
	}
}
void TableView_Color::slotSortByDelta(){
	auto cmpColor=qColor2ColorRGBA(tableModel.compareColor);
	ColorRGBA colorA,colorB;
	tableModel.colorsList->sort([&](const uint32 &a,const uint32 &b){
		colorA.fromRGBA(a);
		colorB.fromRGBA(b);
		return colorA.deltaSum(cmpColor) < colorB.deltaSum(cmpColor);
	});
	tableModel.reset();
}
void TableView_Color::slotRemoveDelta(){
	bool ok;
	int tolerance=QInputDialog::getInt(this,tr("容差"),tr("请输入允许的容差"),0,0,255*3,1,&ok);
	if(ok){//开始进行调色
		ColorRGBA rgba,cmpColor=qColor2ColorRGBA(tableModel.compareColor);
		int idx=0;
		for(auto &u32:widgetImage->colorsList){
			rgba.fromRGBA(u32);
			if(rgba.deltaSum(cmpColor)<=tolerance){
				widgetImage->changeColor(idx,tableModel.compareColor);//改变图像
				u32=cmpColor.toRGBA();
			}
			//下一个
			++idx;
		}
		//去掉重复
		widgetImage->colorsList.unique();
		tableModel.reset();
	}
}
void TableView_Color::slotChangeColorByColorsList(){
	widgetImage->changeColorByColorsList();
}
void TableView_Color::slotStroke(){
	widgetImage->stroke(QColor(0,255,0),QColor(0,0,0));
}
#ifndef TABLEVIEW_COLOR_H
#define TABLEVIEW_COLOR_H

#include<QTableView>
#include<QAction>

#include"TableModel_Color.h"

//颜色表,提供对图像的颜色表的操作
class Widget_Image;
class TableView_Color:public QTableView{
	Q_OBJECT
private:
	Widget_Image *widgetImage;//图像控件,用于获取数据
	TableModel_Color tableModel;//自带颜色表的Model
	QAction actionInsert,actionDelete;
protected:
	void keyReleaseEvent(QKeyEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
public:
	explicit TableView_Color(QWidget *parent=NULL);
	void setWidgetImage(Widget_Image&);
	int currentIndexRow()const;
public slots:
	//增删颜色
	void slotMakeColorsList();
	void slotInsert();
	void slotModify();
	void slotRemove();
	void slotClearColorsList();
	//移动
	void slotMoveUp();
	void slotMoveDown();
	//差异色处理
	void slotDeltaComp();
	void slotSortByDelta();
	void slotRemoveDelta();
	void slotChangeColorByColorsList();
	//描边
	void slotStroke();
};
#endif // TABLEVIEW_COLOR_H
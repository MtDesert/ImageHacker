#ifndef TABLEMODEL_COLOR_H
#define TABLEMODEL_COLOR_H

#include"List.h"
#include"TableModel.h"
#include<QColor>

TABLEMODEL_CLASS(
	TableModel_Color,
	List<uint32> *colorsList;//指向外部颜色表
	QColor compareColor;//颜色比对专用
)
#endif
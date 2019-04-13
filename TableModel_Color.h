#ifndef TABLEMODEL_COLOR_H
#define TABLEMODEL_COLOR_H

#include"List.h"

#include<QAbstractTableModel>

class TableModel_Color:public QAbstractTableModel{
public:
	TableModel_Color();

	void reset();

	QVariant headerData(int section,Qt::Orientation orientation,int role)const;
	int rowCount(const QModelIndex &parent)const;
	int columnCount(const QModelIndex &parent)const;
	QVariant data(const QModelIndex &index,int role)const;

	List<uint32> *colorList;
};
#endif // TABLEMODEL_COLOR_H
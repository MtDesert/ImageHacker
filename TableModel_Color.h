#ifndef TABLEMODEL_COLOR_H
#define TABLEMODEL_COLOR_H

#include<QAbstractTableModel>
#include<QImage>

class TableModel_Color:public QAbstractTableModel{
public:
	TableModel_Color();

	void reset();

	QVariant headerData(int section,Qt::Orientation orientation,int role)const;
	int rowCount(const QModelIndex &parent)const;
	int columnCount(const QModelIndex &parent)const;
	QVariant data(const QModelIndex &index,int role)const;

	QImage *image;
	QList<QColor> *colorList;
};
#endif // TABLEMODEL_COLOR_H
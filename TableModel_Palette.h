#ifndef TABLEMODEL_PALETTE_H
#define TABLEMODEL_PALETTE_H

#include<QAbstractTableModel>
#include"Widget_Image.h"

class TableModel_Palette:public QAbstractTableModel{
public:
	TableModel_Palette();

	void reset();

	QVariant headerData(int section,Qt::Orientation orientation,int role)const;
	int rowCount(const QModelIndex &parent)const;
	int columnCount(const QModelIndex &parent)const;
	QVariant data(const QModelIndex &index,int role)const;

	QList<Palette> *paletteList;
};

#endif // TABLEMODEL_PALETTE_H
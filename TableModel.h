#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include<QAbstractTableModel>

#define TABLEMODEL_CLASS(className,otherMembers) \
class className:public QAbstractTableModel{\
public:\
	className();\
	void reset(){\
		beginResetModel();\
		endResetModel();\
	}\
	QVariant headerData(int section,Qt::Orientation orientation,int role)const;\
	int rowCount(const QModelIndex &parent)const;\
	int columnCount(const QModelIndex &parent)const;\
	QVariant data(const QModelIndex &index,int role)const;\
	otherMembers\
};

#endif
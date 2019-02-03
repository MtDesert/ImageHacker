#include "TableModel_Palette.h"

TableModel_Palette::TableModel_Palette(){}

void TableModel_Palette::reset(){
	beginResetModel();
	endResetModel();
}

QVariant TableModel_Palette::headerData(int section, Qt::Orientation orientation, int role)const{
	switch(orientation){
		case Qt::Horizontal:
			switch(role){
				case Qt::DisplayRole:
					switch(section){
						case 0:return tr("Name");
						case 1:return tr("Color");
						case 2:return tr("Brightness");
					}
				break;
			}
		break;
		case Qt::Vertical:
			switch(role){
				case Qt::DisplayRole:return section;
			}
		break;
	}
	return QVariant();
}
int TableModel_Palette::rowCount(const QModelIndex &parent)const{return paletteList->size();}
int TableModel_Palette::columnCount(const QModelIndex &parent)const{return 3;}
QVariant TableModel_Palette::data(const QModelIndex &index,int role)const{
	auto data=(*paletteList)[index.row()];
	int col=index.column();
	switch(role){
		case Qt::DisplayRole:{
			switch(col){
				case 0:return data.name;
				case 1:return data.color.name(QColor::HexArgb);
				case 2:return QString::asprintf("%d(%.2Xh)",data.brightness,data.brightness);
			}
		}break;
		case Qt::DecorationRole:
			switch(col){
				case 1:return data.color;
				case 2:{
					auto btns=data.brightness;
					return QColor(btns,btns,btns);
				}
			}
		break;
		case Qt::TextAlignmentRole:
			return Qt::AlignCenter;
		break;
	}
	return QVariant();
}
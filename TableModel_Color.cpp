#include "TableModel_Color.h"
#include<QDebug>

TableModel_Color::TableModel_Color():image(nullptr),colorList(nullptr){}

void TableModel_Color::reset(){
	beginResetModel();
	endResetModel();
}

QVariant TableModel_Color::headerData(int section,Qt::Orientation orientation,int role)const{
	switch(orientation){
		case Qt::Horizontal:
			switch(role){
				case Qt::DisplayRole:
					switch(section){
						case 0:return tr("Color");
						case 1:return tr("Hex(ARGB)");
						case 2:return tr("GrayScale");
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
int TableModel_Color::rowCount(const QModelIndex &parent)const{
	if(colorList){
		return colorList->size();
	}else if(image){
		return image->colorCount();
	}
	return 0;
}
int TableModel_Color::columnCount(const QModelIndex &parent)const{return 3;}
QVariant TableModel_Color::data(const QModelIndex &index,int role)const{
	int col=index.column(),row=index.row();
	//get color data
	switch(role){
		case Qt::DisplayRole:
		case Qt::DecorationRole:{
			QColor color;
			if(colorList){
				color=(*colorList)[row];
			}else if(image){
				color.setRgba(image->color(row));
			}
			if(role==Qt::DisplayRole){
				switch(col){
					case 0:return QString::asprintf("r=%d,g=%d,b=%d,a=%d",color.red(),color.green(),color.blue(),color.alpha());
					case 1:return color.name(QColor::HexArgb);
					case 2:{
						auto gray=qGray(color.rgba());
						return QString::asprintf("%d(%.2Xh)",gray,gray);
					}
				}
			}else{
				return color;
			}
		}break;
		case Qt::TextAlignmentRole:return Qt::AlignCenter;
	}
	return QVariant();
}
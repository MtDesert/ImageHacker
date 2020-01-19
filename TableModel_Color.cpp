#include "TableModel_Color.h"
#include"ColorRGB.h"

#include"common.h"

TableModel_Color::TableModel_Color():colorList(nullptr){}

QVariant TableModel_Color::headerData(int section,Qt::Orientation orientation,int role)const{
	switch(orientation){
		case Qt::Horizontal:
			switch(role){
				case Qt::DisplayRole:
					switch(section){
						case 0:return tr("Color");
						case 1:return tr("Hex(ARGB)");
						case 2:return tr("GrayScale");
						case 3:return tr("DeltaSum");
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
	return colorList?colorList->size():0;
}
int TableModel_Color::columnCount(const QModelIndex &parent)const{return 4;}
QVariant TableModel_Color::data(const QModelIndex &index,int role)const{
	if(!colorList)return QVariant();
	int col=index.column(),row=index.row();
	//get color data
	switch(role){
		case Qt::DisplayRole:
		case Qt::DecorationRole:{
			QColor color=uint2QColor(*(*colorList).data(row));
			if(role==Qt::DisplayRole){
				switch(col){
					case 0:return QString::asprintf("r=%d,g=%d,b=%d,a=%d",color.red(),color.green(),color.blue(),color.alpha());
					case 1:return color.name(QColor::HexArgb);
					case 2:{
						auto gray=qGray(color.rgba());
						return QString::asprintf("%d(%.2Xh)",gray,gray);
					}
					case 3:return qColorDeltaSum(color,compareColor);
				}
			}else{
				switch(col){
					case 2:{
						auto gray=qGray(color.rgba());
						color=QColor::fromRgb(gray,gray,gray);
					}break;
					case 3:{
						color=compareColor;
					}break;
				}
				return color;
			}
		}break;
		case Qt::TextAlignmentRole:return Qt::AlignCenter;
	}
	return QVariant();
}
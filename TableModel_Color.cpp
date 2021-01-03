#include "TableModel_Color.h"
#include"ColorRGB.h"
#include"ColorHS.h"

#include"common.h"
#include"Number.h"

TableModel_Color::TableModel_Color():colorsList(nullptr){}

QVariant TableModel_Color::headerData(int section,Qt::Orientation orientation,int role)const{
	switch(orientation){
		case Qt::Horizontal:
			switch(role){
				case Qt::DisplayRole:
					switch(section){
						case 0:return tr("颜色");
						case 1:return tr("16进制(ARGB)");
						case 2:return tr("HSL");
						case 3:return tr("灰度");
						case 4:return tr("本色偏差和");
						case 5:return tr("本色距离");
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
	return colorsList?colorsList->size():0;
}
int TableModel_Color::columnCount(const QModelIndex &parent)const{return 7;}
QVariant TableModel_Color::data(const QModelIndex &index,int role)const{
	if(!colorsList)return QVariant();
	int col=index.column(),row=index.row();
	//get color data
	switch(role){
		case Qt::DisplayRole:
		case Qt::DecorationRole:{
			auto u32=*(*colorsList).data(row);
			QColor color=uint2QColor(u32);
			ColorRGBA rgba(u32);
			if(role==Qt::DisplayRole){//显示文字信息
				switch(col){
					case 0:return QString::asprintf("r=%d,g=%d,b=%d,a=%d",color.red(),color.green(),color.blue(),color.alpha());
					case 1:return color.name(QColor::HexArgb);
#define FMT "%3.1lf"
					case 2:{
						ColorHSL hsl;
						hsl.fromRGB(ColorRGB(u32));
						return QString::asprintf(FMT "|" FMT "|" FMT,hsl.hue,hsl.saturation,hsl.lightness);
					}
					case 3:return rgba.gray();
					case 4:return qColor2ColorRGBA(compareColor).deltaSum(rgba);
					case 5:return qColor2ColorRGBA(compareColor).distance(rgba);
					default:"????";
				}
			}else{//显示颜色
				switch(col){
					default:;
				}
				return color;
			}
		}break;
		case Qt::TextAlignmentRole:return Qt::AlignCenter;
	}
	return QVariant();
}
#include "TableModel_PNG.h"

static uint8 u8=0;
static uint32 u32=0;

TableModel_PNG_Chunk::TableModel_PNG_Chunk():filePng(nullptr){}
TableModel_PNG_IHDR::TableModel_PNG_IHDR():ihdr(nullptr){}
TableModel_PNG_PLTE::TableModel_PNG_PLTE():plte(nullptr){}

QVariant TableModel_PNG_Chunk::headerData(int section,Qt::Orientation orientation,int role)const{
	switch(orientation){
		case Qt::Horizontal:
			switch(role){
				case Qt::DisplayRole:
					switch(section){
						case 0:return tr("Chunk Length");
						case 1:return tr("Name");
						case 2:return tr("Checksum");
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
int TableModel_PNG_Chunk::rowCount(const QModelIndex &parent)const{return filePng?filePng->allChunks.size():0;}
int TableModel_PNG_Chunk::columnCount(const QModelIndex &parent)const{return 3;}
QVariant TableModel_PNG_Chunk::data(const QModelIndex &index,int role)const{
	switch(role){
		case Qt::DisplayRole:{
			auto chunk=*(filePng->allChunks.data(index.row()));
			if(chunk){
				switch(index.column()){
					case 0:{
						chunk->getChunkLength(u32);
						return u32;
					}
					case 1:return (chunk->chunkName().data());
					case 2:{
						chunk->getChunkCRC(u32);
						return QString().sprintf("%.8X",u32);
					}
				}
			}
		}break;
	}
	return QVariant();
}

//IHDR
QVariant TableModel_PNG_IHDR::headerData(int section,Qt::Orientation orientation,int role)const{
switch(orientation){
		case Qt::Horizontal:
			switch(role){
				case Qt::DisplayRole:
					switch(section){
						case 0:return tr("Value");
						case 1:return tr("Note");
					}
				break;
			}
		break;
		case Qt::Vertical:
			switch(role){
				case Qt::DisplayRole:
					switch(section){
						case 0:return tr("Width");
						case 1:return tr("Height");
						case 2:return tr("BitDepth");
						case 3:return tr("ColorType");
						case 4:return tr("Compression");
						case 5:return tr("Filter");
						case 6:return tr("Interlace");
					}
				break;
			}
		break;
	}
	return QVariant();
}
int TableModel_PNG_IHDR::rowCount(const QModelIndex &parent)const{return 7;}
int TableModel_PNG_IHDR::columnCount(const QModelIndex &parent)const{return 1;}
QVariant TableModel_PNG_IHDR::data(const QModelIndex &index,int role)const{
	if(!ihdr)return QVariant();
	switch(role){
		case Qt::DisplayRole:
			switch(index.row()){
				case 0:return ihdr->getWidth(u32)?u32:-1;
				case 1:return ihdr->getHeight(u32)?u32:-1;
				case 2:return ihdr->getBitDepth(u8)?u8:-1;
				case 3:return ihdr->getColorType(u8)?u8:-1;
				case 4:return ihdr->getCompressionMethod(u8)?u8:-1;
				case 5:return ihdr->getFilterMethod(u8)?u8:-1;
				case 6:return ihdr->getInterlaceMethod(u8)?u8:-1;
			}
		break;
	}
	return QVariant();
}

//PLTE
QVariant TableModel_PNG_PLTE::headerData(int section,Qt::Orientation orientation,int role)const{return QVariant();}
int TableModel_PNG_PLTE::rowCount(const QModelIndex &parent)const{return plte?plte->rgbAmount():0;}
int TableModel_PNG_PLTE::columnCount(const QModelIndex &parent)const{return 0;}
QVariant TableModel_PNG_PLTE::data(const QModelIndex &index,int role)const{return QVariant();}
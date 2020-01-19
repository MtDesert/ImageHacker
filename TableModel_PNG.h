#ifndef TABLEMODEL_PNG_H
#define TABLEMODEL_PNG_H

#include"TableModel.h"
#include"FileStructs/FilePNG.h"

TABLEMODEL_CLASS(
	TableModel_PNG_Chunk,
	FilePNG *filePng;
)

TABLEMODEL_CLASS(
	TableModel_PNG_IHDR,
	FilePNG_IHDR *ihdr;
)

TABLEMODEL_CLASS(
	TableModel_PNG_PLTE,
	FilePNG_PLTE *plte;
)

#endif
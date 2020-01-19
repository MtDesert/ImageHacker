#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_MainWindow.h"
#include"TableModel_Color.h"
#include"TableModel_Palette.h"

#include"TableModel_PNG.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = 0);

	//表格
	TableModel_Color tableModel_SrcColor;
	TableModel_Color tableModel_DestColor;
	TableModel_Palette tableModel_Palette;
	//图片文件结构体
	FilePNG filePng;
	//PNG专用表格
	TableModel_PNG_Chunk tableModel_PNG_Chunk;
	TableModel_PNG_IHDR tableModel_PNG_IHDR;
	TableModel_PNG_PLTE tableModel_PNG_PLTE;
public slots:
	//菜单-加载文件
	void on_actionImage_load_triggered();
	void on_actionImage_loadBMP_triggered();
	void on_actionImage_loadPNG_triggered();

	void on_actionImage_save_triggered();
	void on_actionImage_save_palette_triggered();
	void on_actionImage_saveBMP_triggered();
	void on_actionImage_savePNG_triggered();

	void on_actionExit_triggered();

	//src color table
	void on_tableView_SrcColor_activated(const QModelIndex &index);
	void on_tableView_SrcColor_pressed(const QModelIndex &index);
	void on_actionSrcTable_Insert_triggered();
	void on_actionSrcTable_Delete_triggered();
	void on_actionSrcTable_Edit_triggered();

	//目标颜色表
	void on_tableView_DestColor_activated(const QModelIndex &index);
	void on_tableView_DestColor_pressed(const QModelIndex &index);
	void on_actionDestTable_Insert_triggered();
	void on_actionDestTable_Delete_triggered();
	void on_actionDestTable_Edit_triggered();
	void on_actionDestTable_MoveUp_triggered();
	void on_actionDestTable_MoveDown_triggered();
	//目标颜色表-差异比对
	void on_actionDestTable_DeltaComp_triggered();
	void on_actionDestTable_SortByDelta_triggered();
	void on_actionDestTable_RemoveDelta_triggered();
	void on_actionDestImage_Remake_triggered();

	//palette table
	void on_tableView_Palette_activated(const QModelIndex &index);
	void on_actionPalette_MakePalette_triggered();
	void on_actionPalette_MakeDestImage_triggered();

	void slotMoveUpDown(int delta);
protected:
	void afterLoadImage();
	void keyReleaseEvent(QKeyEvent*);
};

#endif
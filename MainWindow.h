#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_MainWindow.h"
#include"TableModel_Color.h"
#include"TableModel_Palette.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = 0);

	TableModel_Color tableModel_SrcColor;
	TableModel_Color tableModel_DestColor;
	TableModel_Palette tableModel_Palette;
public slots:
	//menu
	void on_actionImage_load_triggered();
	void on_actionImage_loadPNG_triggered();

	void on_actionImage_save_triggered();
	void on_actionImage_save_palette_triggered();
	void on_actionImage_savePNG_triggered();

	void on_actionExit_triggered();

	//src color table
	void on_tableView_SrcColor_activated(const QModelIndex &index);
	void on_tableView_SrcColor_pressed(const QModelIndex &index);
	void on_actionSrcTable_Insert_triggered();
	void on_actionSrcTable_Delete_triggered();
	void on_actionSrcTable_Edit_triggered();

	//dest color table
	void on_tableView_DestColor_activated(const QModelIndex &index);
	void on_tableView_DestColor_pressed(const QModelIndex &index);
	void on_actionDestTable_Insert_triggered();
	void on_actionDestTable_Delete_triggered();
	void on_actionDestTable_Edit_triggered();
	void on_actionDestImage_Remake_triggered();

	//palette table
	void on_tableView_Palette_activated(const QModelIndex &index);
	void on_actionPalette_MakePalette_triggered();
	void on_actionPalette_MakeDestImage_triggered();

	void slotMoveUpDown(int delta);
protected:
	void keyReleaseEvent(QKeyEvent*);
};

#endif
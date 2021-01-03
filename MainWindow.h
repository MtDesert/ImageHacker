#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_MainWindow.h"
#include"TableModel_PNG.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = 0);

	//图片文件结构体
	FilePNG filePng;
	//控件
	Widget_Image *widget_SrcImage,*widget_MaskImage,*widget_DestImage;
	//PNG专用表格
	TableModel_PNG_Chunk tableModel_PNG_Chunk;
	TableModel_PNG_IHDR tableModel_PNG_IHDR;
	TableModel_PNG_PLTE tableModel_PNG_PLTE;
public slots:
	//菜单-加载文件
	void on_actionImage_load_triggered();
	void on_actionImage_loadBMP_triggered();
	void on_actionImage_loadPNG_triggered();
	void on_actionMask_Open_triggered();

	void on_actionImage_save_triggered();
	void on_actionImage_save_palette_triggered();
	void on_actionImage_saveBMP_triggered();
	void on_actionImage_savePNG_triggered();

	void on_actionExit_triggered();

	//绘图菜单
	void on_actionPaint_Pen_triggered();
	void on_actionPaint_Fill_triggered();
	void on_actionPaint_FillByOrgColor_triggered();
	void on_actionPaint_FillByBorder_triggered();
	void on_actionPaint_Quantify_triggered();
protected:
	void afterLoadImage();
};
#endif
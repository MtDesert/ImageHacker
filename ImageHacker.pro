#-------------------------------------------------
#
# Project created by QtCreator 2018-10-02T13:51:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageHacker
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
../../CppProject/libGamesEngines/Cpp11 \
../../CppProject/libGamesEngines/Cpp11/Containers \
../../CppProject/libGamesEngines/Zipper \
../../CppProject/libGamesEngines/Image \
../../CppProject/libGamesEngines/Image/ColorSpaces

LIBS += -L../../CppProject/libs -lGamesEngines

SOURCES += \
		main.cpp \
		MainWindow.cpp \
	TableModel_Color.cpp \
	Widget_Image.cpp \
	TableModel_Palette.cpp \
	Dialog_SavePNG.cpp

HEADERS += \
		MainWindow.h \
	TableModel_Color.h \
	Widget_Image.h \
	TableModel_Palette.h \
	Dialog_SavePNG.h

FORMS += \
		MainWindow.ui \
	Dialog_SavePNG.ui

SUBDIRS += \
	ImageHacker.pro
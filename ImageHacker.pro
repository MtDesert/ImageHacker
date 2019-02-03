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
../../CppProject/libCpp11 ../../CppProject/libCpp11/Containers \
../../CppProject/libZipper \
../../CppProject/libImage ../../CppProject/libImage/ColorSpaces

LIBS += -L../../CppProject/lib-Linux -lCpp11 -lZipper -lImage

SOURCES += \
		main.cpp \
		MainWindow.cpp \
	TableModel_Color.cpp \
	Widget_Image.cpp \
    TableModel_Palette.cpp

HEADERS += \
		MainWindow.h \
	TableModel_Color.h \
	Widget_Image.h \
    TableModel_Palette.h

FORMS += \
		MainWindow.ui

SUBDIRS += \
	ImageHacker.pro
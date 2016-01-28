#-------------------------------------------------
#
# Project created by QtCreator 2016-01-26T09:55:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SimpleImgImporter
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dialog_invalid_path.cpp \
    dialogimportchecker.cpp

HEADERS  += mainwindow.h \
    dialog_invalid_path.h \
    dialogimportchecker.h

FORMS    += mainwindow.ui \
    dialog_invalid_path.ui \
    dialogimportchecker.ui

DISTFILES +=

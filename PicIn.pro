#-------------------------------------------------
#
# Project created by QtCreator 2016-01-26T09:55:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PicIn
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dialog_invalid_path.cpp \
    dialogimportchecker.cpp \
    dialog_import_progress.cpp

HEADERS  += mainwindow.h \
    dialog_invalid_path.h \
    dialogimportchecker.h \
    dialog_import_progress.h

FORMS    += mainwindow.ui \
    dialog_invalid_path.ui \
    dialogimportchecker.ui \
    dialog_import_progress.ui

DISTFILES +=

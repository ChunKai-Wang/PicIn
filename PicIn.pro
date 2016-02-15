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
    dialogimportchecker.cpp \
    dialog_import_progress.cpp \
    picin_core.cpp \
    dialog_msg.cpp

HEADERS  += mainwindow.h \
    dialogimportchecker.h \
    dialog_import_progress.h \
    picin_core.h \
    dialog_msg.h

FORMS    += mainwindow.ui \
    dialogimportchecker.ui \
    dialog_import_progress.ui \
    dialog_msg.ui

DISTFILES +=

#-------------------------------------------------
#
# Project created by QtCreator 2017-03-09T22:28:34
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = editor2
TEMPLATE = app


SOURCES += main.cpp\
        editor.cpp \
    draw.cpp

HEADERS  += editor.h \
    draw.h \
    object.h

FORMS    += editor.ui

#-------------------------------------------------
#
# Project created by QtCreator 2015-07-27T15:09:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = bspwalker
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    openglwidget.cpp \
    bsp.cpp \
    camera.cpp \
    bspshader.cpp \
    q3parser.cpp \
    bspentity.cpp

HEADERS  += mainwindow.h \
    openglwidget.h \
    bspdefs.h \
    bsp.h \
    camera.h \
    bspshader.h \
    q3parser.h \
    bspentity.h

FORMS    += mainwindow.ui

RESOURCES += \
    bspwalker.qrc

DISTFILES +=

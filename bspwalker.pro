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
    camera.cpp

HEADERS  += mainwindow.h \
    openglwidget.h \
    bspdefs.h \
    bsp.h \
    camera.h

FORMS    += mainwindow.ui

RESOURCES += \
    bspwalker.qrc

DISTFILES +=

#-------------------------------------------------
#
# Project created by QtCreator 2017-02-16T12:07:26
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = CameraServer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    httpd.cpp \
    getpicture.cpp \
    camera.cpp

HEADERS += \
    httpd.h \
    getpicture.h \
    camera.h

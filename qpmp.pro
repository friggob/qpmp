#-------------------------------------------------
#
# Project created by QtCreator 2017-02-12T12:00:42
#
#-------------------------------------------------

VERSION = 0.9.0

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qpmp
TEMPLATE = app


SOURCES += main.cpp\
        qpmpwin.cpp

HEADERS  += qpmpwin.h

FORMS    += qpmpwin.ui

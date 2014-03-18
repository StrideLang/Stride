#-------------------------------------------------
#
# Project created by QtCreator 2014-02-21T06:00:39
#
#-------------------------------------------------

QT       += core gui qml quick

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

lessThan(QT_MAJOR_VERSION, 5): message("Qt 5 required!")

TARGET = StreamStacker
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    projectwindow.cpp \
    baseproject.cpp \
    simpleproject.cpp \
    blocks/baseblock.cpp \
    blocks/oscblock.cpp \
    blocks/outputblock.cpp

HEADERS  += mainwindow.h \
    projectwindow.h \
    baseproject.h \
    simpleproject.h \
    blocks/baseblock.h \
    blocks/oscblock.h \
    blocks/outputblock.h

FORMS    += mainwindow.ui \
    projectwindow.ui

OTHER_FILES += \
    qml/Editor.qml \
    templates/simple/Makefile \
    templates/simple/src/main.xc

folder_01.source = qml
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

RESOURCES += \
    qmlfiles.qrc

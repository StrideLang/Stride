#-------------------------------------------------
#
# Project created by QtCreator 2014-11-13T20:28:02
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_parsertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_parsertest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

include("../config.pri")

BISONSOURCES = ../editor/parser/lang_stream.y
FLEXSOURCES = ../editor/parser/lang_stream.l
include("../editor/parser.pri")

win32 {
LIBS += -L${FLEX_LIB_PATH}
}

LIBS += -lfl

folder_01.source = data/
folder_01.target = data/
DEPLOYMENTFOLDERS += folder_01

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

INCLUDEPATH += ../parser

TEMPLATE = app

SOURCES += tst_parsertest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

include("../config.pri")

BISONSOURCES = ../editor/parser/lang_stream.y
FLEXSOURCES = ../editor/parser/lang_stream.l

win32 {
LIBS += -L${FLEX_LIB_PATH}
}

LIBS += -lfl

folder_01.source = data/
folder_01.target = data/
DEPLOYMENTFOLDERS += folder_01

DISTFILES += \
    data/stream.stream \
    data/block.stream \
    data/array.stream \
    data/introFeedback.stream \
    data/list.stream \
    data/block.stream \
    data/introFM.stream \
    data/platform.stream \
    data/introGenerator.stream \
    data/introBlock.stream \
    data/introProcessor.stream \
    data/simple.stream \
    data/introConverter.stream \
    data/introRemote.stream \
    data/test.stream \
    data/noneswitch.stream \
    data/platfromBasic.stream \
    data/platformBasic.stream


# Link to parser library
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../parser/release/ -lStreamParser
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../parser/debug/ -lStreamParser
else:unix: LIBS += -L$$OUT_PWD/../parser/ -lStreamParser

INCLUDEPATH += $$PWD/../parser
DEPENDPATH += $$PWD/../parser

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../parser/release/libStreamParser.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../parser/debug/libStreamParser.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../parser/release/StreamParser.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../parser/debug/StreamParser.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../parser/libStreamParser.a

# Link to codegen library
unix|win32: LIBS += -L$$OUT_PWD/../codegen/ -lcodegen

INCLUDEPATH += $$PWD/../codegen
DEPENDPATH += $$PWD/../codegen

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../codegen/codegen.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../codegen/libcodegen.a

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
    data/introFeedback.stream \
    data/introGenerator.stream \
    data/introBlock.stream \
    data/introProcessor.stream \
    data/introConverter.stream \
    data/introRemote.stream \
    data/platfromBasic.stream \
    data/defaultProperties.stream \
    data/shorthand.stream \
    data/introComplex.stream \
    data/introControlFlow.stream \
    data/introEnvelope.stream \
    data/introFIR.stream \
    data/introGranular.stream \
    data/introRandom.stream \
    data/introSequencer.stream \
    data/introStreamRate.stream \
    data/introVocoder.stream \
    data/introModulation.stream \
    data/01_header.stream \
    data/02_basic_blocks.stream \
    data/03_basic_bundle.stream \
    data/04_basic_stream.stream \
    data/05_basic_functions.stream \
    data/07_bundle_indeces.stream \
    data/06_basic_noneswitch.stream \
    data/P01_platform_objects.stream \
    data/P02_check_duplicates.stream \
    data/P03_bundle_resolution.stream \
    data/E01_constant_res.stream \
    data/E02_stream_expansions.stream \
    data/E03_multichn_streams.stream \
    data/E04_rates.stream \
    data/08_namespace.stream \
    data/09_lists.stream

# Link to codegen library

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../codegen/release/ -lcodegen
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../codegen/debug/ -lcodegen
else:unix: LIBS += -L$$OUT_PWD/../codegen/ -lcodegen

INCLUDEPATH += $$PWD/../codegen
DEPENDPATH += $$PWD/../codegen

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../codegen/release/libcodegen.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../codegen/debug/libcodegen.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../codegen/release/codegen.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../codegen/debug/codegen.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../codegen/libcodegen.a

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


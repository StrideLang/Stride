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
    data/introFeedback.stride \
    data/introGenerator.stride \
    data/introBlock.stride \
    data/introProcessor.stride \
    data/introConverter.stride \
    data/introRemote.stride \
    data/platfromBasic.stride \
    data/defaultProperties.stride \
    data/shorthand.stride \
    data/introComplex.stride \
    data/introControlFlow.stride \
    data/introEnvelope.stride \
    data/introFIR.stride \
    data/introGranular.stride \
    data/introRandom.stride \
    data/introSequencer.stride \
    data/introStreamRate.stride \
    data/introVocoder.stride \
    data/introModulation.stride \
    data/01_header.stride \
    data/02_basic_blocks.stride \
    data/03_basic_bundle.stride \
    data/04_basic_stream.stride \
    data/05_basic_functions.stride \
    data/07_bundle_indeces.stride \
    data/06_basic_noneswitch.stride \
    data/P01_platform_objects.stride \
    data/P02_check_duplicates.stride \
    data/P03_bundle_resolution.stride \
    data/E01_constant_res.stride \
    data/E02_stream_expansions.stride \
    data/E03_multichn_streams.stride \
    data/E04_rates.stride \
    data/08_namespace.stride \
    data/09_lists.stride

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


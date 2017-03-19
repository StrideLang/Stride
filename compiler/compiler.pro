QT += core
QT -= gui

CONFIG += c++11

TARGET = compiler
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp


INCLUDEPATH += $$PWD/../parser
DEPENDPATH += $$PWD/../parser

win32-msvc2015:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../codegen/release/ -lcodegen
else:win32-msvc2015:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../codegen/debug/ -lcodegen
else:unix: LIBS += -L$$OUT_PWD/../codegen/ -lcodegen

INCLUDEPATH += $$PWD/../codegen
DEPENDPATH += $$PWD/../codegen

win32-msvc2015:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../codegen/release/codegen.lib
else:win32-msvc2015:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../codegen/debug/codegen.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../codegen/libcodegen.a


# Link to parser library
win32-msvc2015:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../parser/release/ -lStrideParser
else:win32-msvc2015:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../parser/debug/ -lStrideParser
else:unix: LIBS += -L$$OUT_PWD/../parser/ -lStrideParser

INCLUDEPATH += $$PWD/../parser
DEPENDPATH += $$PWD/../parser

win32-msvc2015:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../parser/release/StrideParser.lib
else:win32-msvc2015:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../parser/debug/StrideParser.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../parser/libStrideParser.a

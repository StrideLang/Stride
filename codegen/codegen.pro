#-------------------------------------------------
#
# Project created by QtCreator 2015-02-12T16:43:31
#
#-------------------------------------------------

QT -= gui
QT += core

TARGET = codegen
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++11

message(Building $${TARGET})

SOURCES += \
    pythonproject.cpp \
    codevalidator.cpp \
    coderesolver.cpp \
    strideframework.cpp \
    stridelibrary.cpp \
    stridesystem.cpp \
    systemconfiguration.cpp \
    languagesyntax.cpp

HEADERS += \
    pythonproject.h \
    codevalidator.h \
    coderesolver.h \
    builder.h \
    strideframework.hpp \
    stridelibrary.hpp \
    porttypes.h \
    stridesystem.hpp \
    systemconfiguration.hpp \
    languagesyntax.hpp \
    codeentities.hpp

win32-msvc2015:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../parser/release/ -lStrideParser
else:win32-msvc2015:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../parser/debug/ -lStrideParser
else:unix: LIBS += -L$$OUT_PWD/../parser/ -lStrideParser

INCLUDEPATH += $$PWD/../parser
DEPENDPATH += $$PWD/../parser

#message($$PWD/../parser)

win32-msvc2015:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../parser/release/StrideParser.lib
else:win32-msvc2015:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../parser/debug/StrideParser.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../parser/libStrideParser.a

DISTFILES += \
    ../platforms/Gamma/1.0/scripts/run.py

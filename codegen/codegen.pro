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

message(Building $${TARGET})

SOURCES += \
    pythonproject.cpp \
    codevalidator.cpp \
    coderesolver.cpp \
    stridelibrary.cpp \
    strideplatform.cpp

HEADERS += \
    pythonproject.h \
    codevalidator.h \
    coderesolver.h \
    builder.h \
    stridelibrary.hpp \
    strideplatform.hpp

#unix {
#    target.path = /usr/local/lib
#    INSTALLS += target
#}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../parser/release/ -lStrideParser
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../parser/debug/ -lStrideParser
else:unix: LIBS += -L$$OUT_PWD/../parser/ -lStrideParser

INCLUDEPATH += $$PWD/../parser
DEPENDPATH += $$PWD/../parser

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../parser/release/libStrideParser.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../parser/debug/libStrideParser.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../parser/release/StrideParser.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../parser/debug/StrideParser.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../parser/libStrideParser.a

DISTFILES += \
    ../platforms/common/builtin_types.json \
    ../platforms/PufferFish/common/types.json \
    ../platforms/Gamma/1.0/scripts/run.py

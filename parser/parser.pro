
TARGET = StreamParser
TEMPLATE = lib

CONFIG += staticlib

SOURCES += ast.cpp \
           platformnode.cpp \
           streamnode.cpp \
    valuenode.cpp \
    bundlenode.cpp \
    propertynode.cpp \
    namenode.cpp \
    functionnode.cpp \
    expressionnode.cpp \
    blocknode.cpp \
    listnode.cpp \
    importnode.cpp \
    fornode.cpp \
    rangenode.cpp \
    langerror.cpp

HEADERS += ast.h \
           platformnode.h \
           streamnode.h \
    valuenode.h \
    bundlenode.h \
    propertynode.h \
    namenode.h \
    functionnode.h \
    expressionnode.h \
    blocknode.h \
    listnode.h \
    streamparser.h \
    importnode.h \
    fornode.h \
    rangenode.h \
    langerror.h

BISONSOURCES = lang_stream.y
FLEXSOURCES = lang_stream.l

include(../config.pri)
include(parser.pri)

win32 {
LIBS += -L${FLEX_LIB_PATH}
}
LIBS += -lfl

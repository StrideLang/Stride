
TARGET = StreamParser
TEMPLATE = lib

CONFIG += staticlib

SOURCES += ast.cpp \
           objectnode.cpp \
           platformnode.cpp \
           streamnode.cpp

HEADERS += ast.h \
           objectnode.h \
           platformnode.h \
           streamnode.h

BISONSOURCES = lang_stream.y
FLEXSOURCES = lang_stream.l

include(parser.pri)
include(../config.pri)

win32 {
LIBS += -L${FLEX_LIB_PATH}
}
LIBS += -lfl

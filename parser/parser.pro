
TARGET = StrideParser
TEMPLATE = lib

CONFIG += staticlib
CONFIG += c++11

SOURCES += ast.cpp \
           streamnode.cpp \
    valuenode.cpp \
    bundlenode.cpp \
    propertynode.cpp \
    functionnode.cpp \
    expressionnode.cpp \
    listnode.cpp \
    importnode.cpp \
    fornode.cpp \
    rangenode.cpp \
    langerror.cpp \
    keywordnode.cpp \
    declarationnode.cpp \
    blocknode.cpp \
    scopenode.cpp \
    platformnode.cpp

HEADERS += ast.h \
           streamnode.h \
    valuenode.h \
    bundlenode.h \
    propertynode.h \
    functionnode.h \
    expressionnode.h \
    listnode.h \
    importnode.h \
    fornode.h \
    rangenode.h \
    langerror.h \
    strideparser.h \
    declarationnode.h \
    blocknode.h \
    scopenode.h \
    keywordnode.h \
    platformnode.h

BISONSOURCES = lang_stride.y
FLEXSOURCES = lang_stride.l

include(../config.pri)
include(parser.pri)

win32 {
    LIBS += -L${FLEX_LIB_PATH}
    LIBS += -lfl
}

unix {
    !macx {
        LIBS += -lfl
    }
    macx {
#        LIBS += -L${FLEX_LIB_PATH}
#        LIBS += -ll
    }
}

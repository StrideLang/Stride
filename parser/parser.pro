TARGET = StrideParser
TEMPLATE = lib

CONFIG += staticlib
CONFIG += c++11

win32-msvc2015 {
    CONFIG += stl_off
    CONFIG += exceptions_off
    QMAKE_CXXFLAGS += -clr
    QMAKE_CXXFLAGS += -EHa
}

message(Building $${TARGET})

SOURCES += ast.cpp \
           streamnode.cpp \
    valuenode.cpp \
    bundlenode.cpp \
    propertynode.cpp \
    functionnode.cpp \
    expressionnode.cpp \
    listnode.cpp \
    importnode.cpp \
    rangenode.cpp \
    langerror.cpp \
    keywordnode.cpp \
    declarationnode.cpp \
    blocknode.cpp \
    scopenode.cpp \
    platformnode.cpp \
    portpropertynode.cpp

HEADERS += ast.h \
           streamnode.h \
    valuenode.h \
    bundlenode.h \
    propertynode.h \
    functionnode.h \
    expressionnode.h \
    listnode.h \
    importnode.h \
    rangenode.h \
    langerror.h \
    strideparser.h \
    declarationnode.h \
    blocknode.h \
    scopenode.h \
    keywordnode.h \
    platformnode.h \
    portpropertynode.h

BISONSOURCES = lang_stride.y
FLEXSOURCES = lang_stride.l

include(../config.pri)
include(parser.pri)

unix {
    !macx {
        LIBS += -lfl
    }
}

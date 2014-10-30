#-------------------------------------------------
#
# Project created by QtCreator 2014-02-21T06:00:39
#
#-------------------------------------------------

QT       += core gui qml quick

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

lessThan(QT_MAJOR_VERSION, 5): message("Qt 5 required!")

include(../config.pri)

TARGET = StreamStacker
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    projectwindow.cpp \
    baseproject.cpp \
    codeeditor.cpp \
    ugeninterface.cpp \
    ugen.cpp \
    languagehighlighter.cpp \
    xmosproject.cpp

HEADERS  += mainwindow.h \
    projectwindow.h \
    baseproject.h \
    codeeditor.h \
    ugeninterface.h \
    ugen.h \
    languagehighlighter.h \
    xmosproject.h

FORMS    += mainwindow.ui \
    projectwindow.ui

OTHER_FILES += \
    qml/Editor.qml \
    templates/simple/Makefile \
    templates/simple/src/main.xc \
    templates/simple/lua_scripts/generator.lua \
    templates/simple/lua_scripts/process_template.lua \
    templates/simple/lua_scripts/build_project.lua \
    templates/simple/lua_scripts/parse_code.lua \
    templates/simple/lua_scripts/re.lua \
    templates/simple/code/code.st

folder_01.source = qml
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

RESOURCES += \
    qmlfiles.qrc

INCLUDEPATH += $${LUA_INCLUDE_PATH}
unix|win32: LIBS += -L$${LUA_LIB_PATH} -l$${LUA_LIB}

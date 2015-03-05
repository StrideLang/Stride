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
#    xmosproject.cpp \
#    platform.cpp
    linenumberarea.cpp

HEADERS  += mainwindow.h \
    projectwindow.h \
    baseproject.h \
    codeeditor.h \
    ugeninterface.h \
    ugen.h \
    languagehighlighter.h \
#    xmosproject.h \
#    platform.h
    linenumberarea.h

FORMS    += mainwindow.ui \
    projectwindow.ui

#win32 {
#LIBS += -L${FLEX_LIB_PATH}
#}
#LIBS += -lfl

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

RESOURCES += \
    qmlfiles.qrc

INCLUDEPATH += $${LUA_INCLUDE_PATH}
unix|win32: LIBS += -L$${LUA_LIB_PATH} -l$${LUA_LIB}

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

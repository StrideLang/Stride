
QT += core gui #qml quick

lessThan(QT_MAJOR_VERSION, 5): error("Qt 5 required!")

greaterThan(QT_MINOR_VERSION, 4): QT += webenginewidgets
greaterThan(QT_MINOR_VERSION, 4): DEFINES += USE_WEBENGINE

lessThan(QT_MINOR_VERSION, 5):QT += webkitwidgets

include(../config.pri)

TARGET = StreamStacker
TEMPLATE = app

SOURCES += main.cpp\
    projectwindow.cpp \
    codeeditor.cpp \
    languagehighlighter.cpp \
#    xmosproject.cpp \
#    platform.cpp
    linenumberarea.cpp \
    configdialog.cpp \
    savechangeddialog.cpp \
    errormarker.cpp \
    searchwidget.cpp \
    tooltip.cpp \
    codemodel.cpp \
    autocompletemenu.cpp

HEADERS  += \
    projectwindow.h \
    codeeditor.h \
    languagehighlighter.h \
#    xmosproject.h \
#    platform.h
    linenumberarea.h \
    configdialog.h \
    savechangeddialog.h \
    errormarker.h \
    searchwidget.h \
    tooltip.hpp \
    codemodel.hpp \
    autocompletemenu.hpp

FORMS    += \
    projectwindow.ui \
    configdialog.ui \
    savechangeddialog.ui \
    searchwidget.ui

#win32 {
#LIBS += -L${FLEX_LIB_PATH}
#}
#LIBS += -lfl

# Link to parser library
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

use_lua {
INCLUDEPATH += $${LUA_INCLUDE_PATH}
unix|win32: LIBS += -L$${LUA_LIB_PATH} -l$${LUA_LIB}
}

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

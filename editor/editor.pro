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
    xmosproject.cpp \
    platform.cpp

HEADERS  += mainwindow.h \
    projectwindow.h \
    baseproject.h \
    codeeditor.h \
    ugeninterface.h \
    ugen.h \
    languagehighlighter.h \
    xmosproject.h \
    platform.h

FORMS    += mainwindow.ui \
    projectwindow.ui

BISONSOURCES = parser/lang_stream.y
FLEXSOURCES = parser/lang_stream.l

bison.name = Bison ${QMAKE_FILE_IN}
bison.input = BISONSOURCES
bison.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.parser.cpp
bison.commands = bison -o${QMAKE_FILE_OUT} -d ${QMAKE_FILE_IN}
#lex_compile.depend_command = g++ -E -M ${QMAKE_FILE_NAME} | sed "s,^.*: ,,"
bison.CONFIG += target_predeps
bison.variable_out = GENERATED_SOURCES
QMAKE_EXTRA_COMPILERS += bison
#bison_header.input = BISONSOURCES
#bison_header.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.parser.hpp
#bison_header.commands = bison -d -o ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.parser.cpp ${QMAKE_FILE_IN}
#bison_header.CONFIG += target_predeps no_link
#silent:bison_header.commands = @echo Bison ${QMAKE_FILE_IN} && $$bison.commands
#QMAKE_EXTRA_COMPILERS += bison_header

flex.name = Flex ${QMAKE_FILE_IN}
flex.input = FLEXSOURCES
flex.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.lexer.cpp
flex.commands = flex -o ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.lexer.cpp ${QMAKE_FILE_IN}
flex.CONFIG += target_predeps
flex.variable_out = GENERATED_SOURCES
silent:flex.commands = @echo Lex ${QMAKE_FILE_IN} && $$flex.commands
QMAKE_EXTRA_COMPILERS += flex

LIBS += -lfl

OTHER_FILES += \
    qml/Editor.qml \
    templates/simple/Makefile \
    templates/simple/src/main.xc \
    templates/simple/lua_scripts/generator.lua \
    templates/simple/lua_scripts/process_template.lua \
    templates/simple/lua_scripts/build_project.lua \
    templates/simple/lua_scripts/parse_code.lua \
    templates/simple/lua_scripts/re.lua \
    templates/simple/code/code.st \
    $${BISONSOURCES} \
    $${FLEXSOURCES}

folder_01.source = qml
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

RESOURCES += \
    qmlfiles.qrc

INCLUDEPATH += $${LUA_INCLUDE_PATH}
unix|win32: LIBS += -L$${LUA_LIB_PATH} -l$${LUA_LIB}

TARGET = lpeg
TEMPLATE = lib
CONFIG += plugin no_plugin_name_prefix

include(../config.pri)

DESTDIR = ../../StreamStack/editor/templates/simple/lua_scripts
LIBS += -L$${LUA_LIB_PATH} -lluajit-5.1

SOURCES += lpcap.c  lpcode.c  lpprint.c  lptree.c  lpvm.c

use_lua {
TARGET = lpeg
TEMPLATE = lib
CONFIG += plugin no_plugin_name_prefix

message(Building lpeg)

include(../../config.pri)

DESTDIR = ../../../StreamStack/editor/templates/simple/lua_scripts
INCLUDEPATH += $${LUA_INCLUDE_PATH}
LIBS += -L$${LUA_LIB_PATH} -l$${LUA_LIB}

SOURCES += lpcap.c  lpcode.c  lpprint.c  lptree.c  lpvm.c
}

TARGET = lpeg
TEMPLATE = lib
CONFIG += plugin no_plugin_name_prefix

INCLUDEPATH += /usr/include/luajit-2.0

DESTDIR = ../../StreamStack/editor/templates/simple/lua_scripts

QMAKE_CFLAGS += -Wall -Wextra -pedantic -Waggregate-return -Wcast-align -Wcast-qual \
 -Wdisabled-optimization -Wpointer-arith -Wshadow -Wsign-compare -Wundef -Wwrite-strings \
 -Wbad-function-cast -Wdeclaration-after-statement -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes  -O2 -ansi

LUA_LIB_PATH = /usr/lib/x86_64-linux-gnu
unix|win32: LIBS += -L$${LUA_LIB_PATH} -lluajit-5.1

SOURCES += lpcap.c  lpcode.c  lpprint.c  lptree.c  lpvm.c

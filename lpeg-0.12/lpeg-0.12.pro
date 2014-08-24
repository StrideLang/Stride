TARGET = lpeg
TEMPLATE = lib
#CONFIG += dll

INCLUDEPATH += /usr/include/luajit-2.0

LUA_LIB_PATH = /usr/lib/x86_64-linux-gnu
unix|win32: LIBS += -L$${LUA_LIB_PATH} -lluajit-5.1

SOURCES += lpcap.c  lpcode.c  lpprint.c  lptree.c  lpvm.c

include(../config.pri)
unix {
    INCLUDEPATH += /usr/include/luajit-2.0
    LUA_LIB_PATH = /usr/lib/x86_64-linux-gnu
    QMAKE_CFLAGS += -Wall -Wextra -pedantic -Waggregate-return -Wcast-align -Wcast-qual \
 -Wdisabled-optimization -Wpointer-arith -Wshadow -Wsign-compare -Wundef -Wwrite-strings \
 -Wbad-function-cast -Wdeclaration-after-statement -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes  -O2 -ansi

}
win32 { # These need to be revised for windows.
    INCLUDEPATH += /usr/include/luajit-2.0
    LUA_LIB_PATH = /usr/lib/x86_64-linux-gnu
    QMAKE_CFLAGS += -Wall -Wextra -pedantic -Waggregate-return -Wcast-align -Wcast-qual \
 -Wdisabled-optimization -Wpointer-arith -Wshadow -Wsign-compare -Wundef -Wwrite-strings \
 -Wbad-function-cast -Wdeclaration-after-statement -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes  -O2 -ansi
}

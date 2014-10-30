
unix {
    LUA_INCLUDE_PATH += /usr/include/luajit-2.0
    LUA_LIB_PATH = /usr/lib/x86_64-linux-gnu
    LUA_LIB = luajit-5.1
    QMAKE_CFLAGS += -Wall -Wextra -pedantic -Waggregate-return -Wcast-align -Wcast-qual \
 -Wdisabled-optimization -Wpointer-arith -Wshadow -Wsign-compare -Wundef -Wwrite-strings \
 -Wbad-function-cast -Wdeclaration-after-statement -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes  -O2 -ansi

}
win32 { # These need to be revised for windows.
    LUA_INCLUDE_PATH += ../../lua-5.1.5/src ../../lua-5.1.5/etc
    LUA_LIB_PATH = ../../lua-5.1.5/src
    LUA_LIB = lua51
    QMAKE_CFLAGS += -Wall -Wextra -pedantic -Waggregate-return -Wcast-align -Wcast-qual \
 -Wdisabled-optimization -Wpointer-arith -Wshadow -Wsign-compare -Wundef -Wwrite-strings \
 -Wbad-function-cast -Wdeclaration-after-statement -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes  -O2 -ansi
}

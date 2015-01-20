
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

    FLEX_BIN_PATH = C:\Tools\flex-2.5.4\bin
    FLEX_LIB_PATH = C:\Tools\flex-2.5.4\lib
    BISON_BIN_PATH = C:\Tools\bison-2.4.1\bin

    QMAKE_CFLAGS += -Wall -Wextra -pedantic -Waggregate-return -Wcast-align -Wcast-qual \
 -Wdisabled-optimization -Wpointer-arith -Wshadow -Wsign-compare -Wundef -Wwrite-strings \
 -Wbad-function-cast -Wdeclaration-after-statement -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes  -O2 -ansi
}

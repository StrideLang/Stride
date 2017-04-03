unix {
    !macx {
        use_lua {
          LUA_INCLUDE_PATH += /usr/include/luajit-2.0
          LUA_LIB_PATH = /usr/lib/x86_64-linux-gnu
          LUA_LIB = luajit-5.1
          QMAKE_CFLAGS += -Wall -Wextra -pedantic -Waggregate-return -Wcast-align -Wcast-qual \
            -Wdisabled-optimization -Wpointer-arith -Wshadow -Wsign-compare -Wundef -Wwrite-strings \
            -Wbad-function-cast -Wdeclaration-after-statement -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes  -O2 -ansi
        }
    }

    macx {
        BISON_BIN_PATH = /usr/local/opt/bison/bin
        FLEX_BIN_PATH = /usr/local/opt/flex/bin
        FLEX_LIB_PATH = /usr/local/opt/flex/lib
        use_lua {
            message("No lua support on OS X.")
        }
    }
}

win32-msvc2015 {
    FLEX_BIN_PATH = C:\Tools\winflexbison\bin\Release
    BISON_BIN_PATH = C:\Tools\winflexbison\bin\Release
    use_lua {
# These need to be revised for windows.
      LUA_INCLUDE_PATH += C:\Tools\Lua\5.1\include
      LUA_LIB_PATH = C:\Tools\Lua\5.1\lib
      LUA_LIB = lua51
      QMAKE_CFLAGS += -Wall -Wextra -pedantic -Waggregate-return -Wcast-align -Wcast-qual \
         -Wdisabled-optimization -Wpointer-arith -Wshadow -Wsign-compare -Wundef -Wwrite-strings \
         -Wbad-function-cast -Wdeclaration-after-statement -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes  -O2 -ansi
    }
}



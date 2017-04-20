unix {
    !macx {
    }

    macx {
        BISON_BIN_PATH = /usr/local/opt/bison/bin
        FLEX_BIN_PATH = /usr/local/opt/flex/bin
        FLEX_LIB_PATH = /usr/local/opt/flex/lib
    }
}

win32-msvc2015 {
    FLEX_BIN_PATH = C:\Tools\winflexbison\bin\Release
    BISON_BIN_PATH = C:\Tools\winflexbison\bin\Release
}



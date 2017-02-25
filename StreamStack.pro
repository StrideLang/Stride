TEMPLATE = subdirs

SUBDIRS = parser \
          editor \
          codegen \
          tests \
          compiler

codegen.depends = parser
tests.depends = parser codegen
editor.depends = parser codegen
compiler.depends = parser codegen

use_lua {
    message(Building LUA support)
    DEFINES += USE_LUA
    SUBDIRS += "lua/lpeg-0.12"
    editor.depends += lua/lpeg-0.12
}

exists( plugins/pufferfishplugin) {
    SUBDIRS += plugins/pufferfishplugin
}

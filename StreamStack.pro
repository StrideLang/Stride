TEMPLATE = subdirs

SUBDIRS = parser \
          editor \
          codegen \
          tests


editor.depends = parser codegen
codegen.depends = parser
tests.depends = parser codegen

use_lua {
    message(Building LUA support)
    DEFINES += USE_LUA
    SUBDIRS += "lua/lpeg-0.12"
    editor.depends += lua/lpeg-0.12
}

exists( plugins/pufferfishplugin) {
    SUBDIRS += plugins/pufferfishplugin
}

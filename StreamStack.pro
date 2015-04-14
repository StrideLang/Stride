TEMPLATE = subdirs

use_lua {
    message(Building LUA support)
    DEFINES += USE_LUA
}

use_lua: SUBDIRS = "lua/lpeg-0.12"

SUBDIRS += parser \
          editor \
          codegen \
          tests

use_lua: editor.depends = lua/lpeg-0.12

editor.depends += parser codegen
codegen.depends = parser
tests.depends = parser codegen

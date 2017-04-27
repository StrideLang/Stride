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

exists( plugins/pufferfishplugin) {
    SUBDIRS += plugins/pufferfishplugin
}

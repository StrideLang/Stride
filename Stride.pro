message("Building Stride")

TEMPLATE = subdirs

SUBDIRS = parser \
          codegen \
          tests \
          compiler

codegen.depends = parser
tests.depends = parser codegen
compiler.depends = parser codegen

# Editor requires Qt 5.7 for WebEngine widgets
greaterThan(QT_MINOR_VERSION, 7) {
  SUBDIRS +=          editor
  editor.depends = parser codegen
}

# Build/Deploy Plugins
exists( plugins/pufferfishplugin) {
    SUBDIRS += plugins/pufferfishplugin
}

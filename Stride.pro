message("Building Stride")

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = parser \
          codegen \
          tests \
          compiler\
          stridemanager


codegen.depends = parser
tests.depends = parser codegen
compiler.depends = parser codegen

# Editor requires Qt 5.7 for WebEngine widgets
greaterThan(QT_MINOR_VERSION, 7) {
  SUBDIRS +=          editor
  editor.depends = parser codegen
}

# Build/Deploy Plugins
exists(plugins/pufferfishplugin) {
    SUBDIRS += plugins/pufferfishplugin
    pufferfishplugin.depends = codegen
}

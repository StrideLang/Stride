TEMPLATE = subdirs

#CONFIG += ordered

SUBDIRS = "lua/lpeg-0.12" \
          parser \
          editor \
          codegen \
          tests

editor.depends = lua/lpeg-0.12 parser codegen
codegen.depends = parser
tests.depends = parser codegen

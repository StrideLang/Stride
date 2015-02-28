TEMPLATE = subdirs

#CONFIG += ordered

SUBDIRS = lpeg-0.12 \
          parser \
          editor \
          codegen \
          tests

editor.depends = lpeg-0.12 parser codegen
codegen.depends = parser
tests.depends = parser codegen

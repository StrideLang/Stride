TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = lpeg-0.12 \
          parser \
          editor \
          tests \
    codegen

editor.depends = lpeg-0.12 parser codegen
codegen.depends = parser
tests.depends = parser codegen

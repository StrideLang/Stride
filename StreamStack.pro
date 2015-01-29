TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = lpeg-0.12 \
          parser \
          editor \
          tests

editor.depends = lpeg-0.12 parser
tests.depends = parser

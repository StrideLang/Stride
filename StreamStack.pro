TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = lpeg-0.12 \
          editor \
          tests

editor.depends = lpeg-0.12

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = lpeg-0.12 \
          editor
editor.depends = lpeg-0.12

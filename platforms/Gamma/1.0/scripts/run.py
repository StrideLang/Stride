# -*- coding: utf-8 -*-

from __future__ import print_function
from __future__ import division

import os
import sys
from subprocess import check_output as ck_out

#try:
#    from PySide import QtWidgets
#except:
#    from PyQt5 import QtWidgets


class Runner:
    def __init__(self):
        pass

out_dir = sys.argv[1]
app_name = "app"

def log(text):
    print(text)

os.chdir(out_dir)
log("Running in directory: " + out_dir)

args = ["./" + app_name]
outtext = ck_out(args)

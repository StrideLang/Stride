# -*- coding: utf-8 -*-

from __future__ import print_function
from __future__ import division

import os
import sys
from subprocess import check_output as ck_out


class Runner:
    def __init__(self):
        pass

if __name__ == '__main__':
    out_dir = sys.argv[1]
    app_name = sys.argv[2]

    def log(text):
        print(text)

    print("Running python.")

    os.chdir(out_dir)
    log("Running in directory: " + out_dir)

    args = [app_name]
    outtext = ck_out(args)

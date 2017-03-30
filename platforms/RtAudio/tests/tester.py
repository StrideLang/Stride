# -*- coding: utf-8 -*-
"""
Created on Mon May 23 07:52:11 2016

@author: andres
"""


import glob
import os
import subprocess


platform_dir = os.path.abspath(os.getcwd() + "/Gamma/1.0")

files = glob.glob("Gamma/tests/*.stride")

report = {}
for fname in files:
    products_dir = os.path.abspath(fname + "_Products")
    if not os.path.exists(products_dir):
        os.mkdir(products_dir)
    print(platform_dir, products_dir)
    result = subprocess.call(["python", "library/1.0/python/build.py", products_dir, platform_dir])
    report[fname] = result
        
for f in report:
    result = "Passed" if report[f] == 0 else "FAILED"
    print(f.ljust(40) + ":" + result)
    
print("Done.")


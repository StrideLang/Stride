# -*- coding: utf-8 -*-

from __future__ import print_function
from __future__ import division

import sys
import os
import json

# ---------------------
class Builder(object):
    def __init__(self, jsonfilename, strideroot, products_dir, debug = False):
        self.strideroot = strideroot
        self.products_dir = products_dir
        self.debug = debug

        jsonfile = open(jsonfilename)
        tree = json.load(jsonfile)

        platform_dir = None
        for node in tree:
            if "system" in node:
                platform_dir = node['system']['platforms'][0]['path']
                break

        # Add python path inside strideroot to module search paths
        sys.path.append(self.strideroot + "/library/1.0/python")
        # Add platform scritps path to python module search paths
        sys.path.append(platform_dir + "/scripts")

        print("Using strideroot:" + strideroot)
        print("Using platform: " + platform_dir)
        # Get gerenator

        from platformGenerator import Generator

        self.gen = Generator(products_dir, strideroot, platform_dir, tree, debug)

    def build(self):
        self.gen.generate_code()
        print("Building done...")
        self.gen.compile()

    def run(self):
        self.gen.run()

    def stop(self):
        self.gen.stop()

if __name__ == '__main__':
    import argparse
    cur_path = os.getcwd()
    default_file = '/home/andres/Documents/src/Stride/Stride/strideroot/frameworks/RtAudio/1.0/_tests/module/16_recursive_module_shared.stride'
#    default_file = '/home/andres/Documents/src/Stride/Stride/strideroot/frameworks/RtAudio/1.0/_tests/reactions/02_module_in_reaction.stride'
    # First parse command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("jsonfile",
                        help="JSON file containing parsed tree",
                        nargs='?',
                        default = default_file + '_Products/tree-RtAudio.json'
                        )
    parser.add_argument("products_dir",
                        help="The directory where stride products where generated",
                        nargs='?',
                        default=default_file + '_Products'

                        )
    parser.add_argument("strideroot",
                        help="The directory of the strideroot",
                        nargs='?',
                        default = cur_path
                        )
    parser.add_argument("command",
                        help="Commands to execute",
                        nargs='?',
                        default = "build&run"
                        )
    args = parser.parse_args()

    builder = Builder(args.jsonfile, args.strideroot, args.products_dir, True)

    commands = args.command.split("&")
    print(commands)
    for command in commands:
        if command == "build":
            builder.build()
        elif command == "run":
            builder.run()
        else:
            builder.custom_command(command)



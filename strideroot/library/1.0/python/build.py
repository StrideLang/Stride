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
    # First parse command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("jsonfile",
                        help="JSON file containing parsed tree",
                        nargs='?',
                        default = cur_path + '/../examples/eoys/Beating.stride_Products/tree-RtAudio.json'
                        )
    parser.add_argument("products_dir",
                        help="The directory where stride products where generated",
                        nargs='?',
#                        default= cur_path + '/Gamma/tests/reaction.stride_Products'
#                        default= cur_path + '/Gamma/tests/pan.stride_Products'
#                        default= cur_path + '/RtAudio/tests/modulation.stride_Products'
#                        default= cur_path + '/Gamma/tests/greater.stride_Products'
#                        default= cur_path + '/library/1.0/_tests/module/01_out.stride_Products'
#                        default= cur_path + '/library/1.0/_tests/module/02_in.stride_Products'
#                        default= cur_path + '/library/1.0/_tests/module/03_in_and_out.stride_Products'
#                        default= cur_path + '/library/1.0/_tests/module/04_port.stride_Products'
#                        default= cur_path + '/library/1.0/_tests/module/05_port_domain.stride_Products'
#                        default= cur_path + '/library/1.0/_tests/module/06_port_domain.stride_Products'
#                        default= cur_path + '/library/1.0/_tests/module/09_bundle_out.stride_Products'
#                        default = cur_path + '/library/1.0/_tests/module/10_bundle_in.stride_Products'
#                        default = cur_path + '/library/1.0/_tests/modulation.stride_Products'

#                        default= cur_path + '/RtAudio/icmc/ICMC_06.stride_Products'
                        default= cur_path + '/../examples/eoys/Beating.stride_Products'
#                        default= cur_path + '/RtAudio/tests/bundles.stride_Products'
#                        default= cur_path + '/../platforms/Wiring/examples/test.stride_Products'
#                        default='/home/andres/Documents/src/Stride/StreamStack/platforms/Arduino/examples/test.stride_Products'
#                        default= cur_path + '/STM32F7/examples/test.stride_Products'
#                        default= cur_path + '/../tests/data/P06_domains.stride_Products'
                        )
    parser.add_argument("strideroot",
                        help="The directory of the strideroot",
                        nargs='?',
#                        default = cur_path + '/Wiring/1.0'
#                       default = cur_path + '/STM32F7/1.0'
                        default = cur_path
                        )
    parser.add_argument("command",
                        help="Commands to execute",
                        nargs='?',
#                        default = cur_path + '/Wiring/1.0'
#                       default = cur_path + '/STM32F7/1.0'
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



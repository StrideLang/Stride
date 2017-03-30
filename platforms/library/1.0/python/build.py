# -*- coding: utf-8 -*-

from __future__ import print_function
from __future__ import division

import sys, os

# ---------------------

def build(platform_dir, products_dir, debug = False):
    
    # Add platform scritps path to python module search paths
    sys.path.append(platform_dir + "/scripts")
    
    print("Using :" + platform_dir + "/scripts")
    # Get gerenator
    from platformGenerator import Generator
    
    gen = Generator(products_dir, platform_dir, debug)
    gen.generate_code()
    print("Building done...")
    gen.compile()

if __name__ == '__main__':
    import argparse
    cur_path = os.getcwd()
    sys.path.append(cur_path + "/library/1.0/python")
    # First parse command line arguments
    parser = argparse.ArgumentParser()
    
    parser.add_argument("products_dir",
                        help="The directory where stride products where generated",
                        nargs='?',
                        default= cur_path + '/Gamma/tests/reaction.stride_Products'
#                        default= cur_path + '/Gamma/tests/pan.stride_Products'
#                        default= cur_path + '/Gamma/tests/modulation.stride_Products'
#                        default= cur_path + '/Gamma/tests/greater.stride_Products'
#                        default= cur_path + '/library/1.0/_tests/module/01_out.stride_Products'
#                        default= cur_path + '/library/1.0/_tests/module/02_in.stride_Products'
#                        default= cur_path + '/library/1.0/_tests/module/03_in_and_out.stride_Products'
#                        default= cur_path + '/library/1.0/_tests/module/04_port.stride_Products'
#                        default= cur_path + '/library/1.0/_tests/module/05_port_domain.stride_Products'
#                        default= cur_path + '/library/1.0/_tests/module/06_port_domain.stride_Products'
#                        default= cur_path + '/library/1.0/_tests/module/09_bundle_out.stride_Products'
#                        default = cur_path + '/library/1.0/_tests/module/10_bundle_in.stride_Products'

#                        default= cur_path + '/Gamma/icmc/ICMC_01.stride_Products'
#                        default= cur_path + '/Wiring/examples/test.stride_Products'
#                        default='/home/andres/Documents/src/Stride/StreamStack/platforms/Arduino/examples/test.stride_Products'
#                        default= cur_path + '/Discovery_M7/examples/test.stride_Products'
#                        default= cur_path + '/../tests/data/P06_domains.stride_Products'
                        )
    parser.add_argument("platform_dir",
                        help="The directory of the platform to be used",
                        nargs='?',
#                        default= cur_path + '/Wiring/1.0'
#                       default = cur_path + '/STM32F7/1.0'
                        default = cur_path + '/Gamma/1.0'
                        )
    args = parser.parse_args()
    
    platform_dir = args.platform_dir
    products_dir = args.products_dir
    
    build(platform_dir, products_dir, True)
    

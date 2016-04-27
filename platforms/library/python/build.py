# -*- coding: utf-8 -*-

from __future__ import print_function
from __future__ import division

import sys, os

import argparse

# ---------------------

if __name__ == '__main__':
    this_path = os.path.dirname(os.path.abspath(__file__))
    # First parse command line arguments
    parser = argparse.ArgumentParser()
    
    parser.add_argument("products_dir",
                        help="The directory where stride products where generated",
                        nargs='?',
                        default='/home/andres/Documents/src/Stride/StreamStack/platforms/Gamma/examples/passthru.stride_Products'
#                        default='/home/andres/Documents/src/Stride/StreamStack/platforms/Arduino/examples/test.stride_Products'
                        )
    parser.add_argument("platform_dir",
                        help="The directory of the platform to be used",
                        nargs='?',
                        default=this_path + '/../../Gamma/1.0'
#                        default=this_path + '/../../Arduino/1.0'
                        )
    args = parser.parse_args()
    
    # Add platform scritps path to python module search paths
    sys.path.append(args.platform_dir + "/scripts")
    
    
    # Get gerenator
    from platformGenerator import Generator
    
    gen = Generator(args.products_dir, args.platform_dir)
    gen.generate_code()
    gen.compile()

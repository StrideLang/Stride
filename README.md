# Stride

[![Build Status](https://travis-ci.org/StrideLanguage/Stride.svg?branch=master)](https://travis-ci.org/StrideLanguage/Stride)

Stride Language

# Installation

Requirements for all platforms:
 * Qt 5.14
 * cmake 3.15

## MacOS

Requirements:
 * XCode (and command line tools)
 * bison, flex (brew install bison flex). (No need to fllow symlink instructions provided by brew)

## Linux

apt install flex bison freeglut3-dev

## Windows

Download and install:

 * flex: http://gnuwin32.sourceforge.net/packages/flex.htm
 * bison: http://gnuwin32.sourceforge.net/packages/bison.htm
 
 When calling cmake add the following to the cmake call: 
 
     -DBISON_EXECUTABLE='C:\Program Files (x86)\GnuWin32\bin\bison.exe' -DFLEX_EXECUTABLE='C:\Program Files (x86)\GnuWin32\bin\flex.exe'
 
 Tested with Visual Studio 2019.

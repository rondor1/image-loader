# Image Loader


This repository contains source code for a library that currently supports loading and storing of a TRUEVISION (.tga) image.

Future versions will provide means for manipulation of the image, such as:
1. Horizontal/Vertical flipping;
1. Manipulation in spatial and frequency domain.

## Prerequisites

In order to build this library, the following is needed:

1. C++ compiler that supports C++17 standard;
1. CMake, Version 3.15.0 or greater;
1. Conan package manager (no specific version is needed).

**NOTE: In order to successfully install conan, python3 is needed!**

## Building the library

In order to build this library, do the following:

        mkdir build;
        cd build;
        cmake ..
        make -j8

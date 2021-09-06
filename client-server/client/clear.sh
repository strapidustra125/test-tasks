#!/bin/bash

echo -e "\nClearing CMake temp files...\n"

rm -R CMakeFiles
rm cmake_install.cmake
rm CMakeCache.txt
rm Makefile
rm zmq_app

echo -e "\nDone!\n"

#!/bin/bash

g++ prog00.cpp
if [ $? -eq 0 ]
then
    ./a.out
    xdg-open ./out.ppm
    rm a.out
fi

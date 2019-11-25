#!/bin/bash

g++ prog00.cpp
if [ $? -eq 0 ]
then
    ./a.out
    if [[ $? -eq 0 ]]
    then
        xdg-open ./out.ppm
    fi
    rm a.out
fi

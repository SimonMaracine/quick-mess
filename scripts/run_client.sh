#! /bin/bash

clear
cd ../build
cmake --build . -j 8 --target quick_mess

if [ "$?" -ne 0 ]; then
    exit 1
fi

client/quick_mess

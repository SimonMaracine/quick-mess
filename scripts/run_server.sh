#! /bin/bash

clear
cd ../build
cmake --build . -j 8 --target quick_mess_server

if [ "$?" -ne 0 ]; then
    exit 1
fi

server/quick_mess_server

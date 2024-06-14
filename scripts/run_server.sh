#! /bin/bash

./build.sh quick_mess_server

if [ "$?" -ne 0 ]; then
    exit 1
fi

cd ../build
server/quick_mess_server

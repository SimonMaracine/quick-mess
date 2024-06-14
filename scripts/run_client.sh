#! /bin/bash

./build.sh quick_mess

if [ "$?" -ne 0 ]; then
    exit 1
fi

cd ../build
client/quick_mess

#!/bin/bash
FILE=consola
make $FILE
if test -f "./$FILE"; then
    ./$FILE $1 $2
fi


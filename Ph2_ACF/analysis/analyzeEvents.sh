#!/bin/bash

if [ "$1" == "" ]; then
    echo Syntax $0 fileName
    exit -1
fi

fileName=$1

if [ ! -f $fileName ]; then
    echo Cannot find file $fileName
    exit -1
fi

#echo "TDC/I:nHits/I:thresh/I" > tempfile.dat
#egrep '^EVENT ' $fileName | gawk '{print $4" "$6" "$8}' >> tempfile.dat
#cp $filename tempfile.dat

root -l analyzeEventsSlice.cpp+

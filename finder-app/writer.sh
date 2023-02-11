#!/bin/bash

if [ $# -ne 2 ]; then
    echo "error: Two parameters must be provided"
    echo "    arg1 : a full path to a file (including filename) on the filesystem"
    echo "    arg2 : a text string which will be written within this file"
    exit 1
fi

writefile=$1
writestr=$2

# get the path of the directory containing the file
writefileDir=${writefile%/*}
# create the directory and all the needed higher directories in the hierarchy
# make sure we avoid printing the an additional newline in the output file
mkdir -p $writefileDir && \
    echo -n $writestr > $writefile

if [ $? -ne 0 ]; then
    exit 1
fi

exit 0

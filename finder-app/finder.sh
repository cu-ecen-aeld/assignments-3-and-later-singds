#!/bin/sh

if [ $# -ne 2 ]; then
    echo "error: Two parameters must be provided"
    echo "    arg1 : path to a directory on the filesystem"
    echo "    arg2 : text string which will be searched"
    exit 1
fi

filesdir=$1
searchstr=$2

if [ ! -d $filesdir ]; then
    echo "error: arg1 is not a directory"
    exit 1
fi

numFiles=$(grep -lr $searchstr $filesdir | wc -l)
numLines=$(grep -r $searchstr $filesdir | wc -l)

echo "The number of files are $numFiles and the number of matching lines are $numLines"

exit 0

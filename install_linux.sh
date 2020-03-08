#!/bin/sh

if [ $# -eq 1 ]
then
    setcap cap_kill+ep $1/rscgui 2> /dev/null 
    setcap cap_kill+ep $1/rsccli
else
    echo "You need to specify the binaries directory"
    echo "$0 path/to/bin_dir"
fi

#!/bin/sh

export LD_LIBRARY_PATH='.'
export CFLAGS='-O0 -g -DCO_VALGRIND'
export LDFLAGS=''
export RLFLAGS=''

make clean

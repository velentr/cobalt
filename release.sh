#!/bin/sh

export LD_LIBRARY_PATH='.'
export CFLAGS='-O2 -march=native -DNDEBUG'
export LDFLAGS='-O1'
export RLFLAGS='-G2'

make clean

#!/bin/sh

export LD_LIBRARY_PATH='.'
export CFLAGS='-O0 -g -fsanitize=undefined'
export LDFLAGS='-fsanitize=undefined'
export RLFLAGS=''

make clean

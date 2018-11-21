#!/bin/sh

export LD_LIBRARY_PATH='.'
export CFLAGS='-O0 -g -fsanitize=undefined -DCO_VALGRIND'
export LDFLAGS='-fsanitize=undefined'
export RLFLAGS=''

make clean

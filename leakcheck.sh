#!/bin/bash

LD_PRELOAD=/usr/lib64/libc_malloc_debug.so.0 MALLOC_TRACE=/tmp/t \
./main $1
mtrace main /tmp/t

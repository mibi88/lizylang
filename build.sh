#!/bin/bash

cc src/main.c src/lisp.c src/var.c src/platform.c src/call.c src/builtin.c \
   src/tree.c -o main -ansi -Isrc -g -Wall -Wextra -Wpedantic -lm
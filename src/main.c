/* A small interpreter for a lisp like language, targetting embedded systems.
 * by Mibi88
 *
 * This software is licensed under the BSD-3-Clause license:
 *
 * Copyright 2024 Mibi88
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* CHANGELOG
 *
 * 2024/09/28: Started developement. File loading and error handler.
 * 2024/10/12: Avoid segfault if the file isn't found. Error message if the
 *             file isn't found.
 */

#include <lisp.h>

#include <stdio.h>
#include <stdlib.h>

char *file = NULL;

void onerror(char *message, void *data) {
    TinyLisp *lisp = data;
    fprintf(stderr, "%s:%ld: Error: %s\n", file, lisp->line, message);
}

int main(int argc, char **argv) {
    FILE *fp;
    TinyLisp lisp;
    size_t sz;
    char *buffer;
    int rc;
    if(argc < 2){
        fputs("USAGE: tinylisp [INPUT]\n", stderr);
        return EXIT_FAILURE;
    }
    file = argv[1];
    fp = fopen(argv[1], "r");
    if(!fp){
        fprintf(stderr, "[tinylisp] File not found!\n");
        return EXIT_FAILURE;
    }
    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    rewind(fp);
    buffer = malloc(sz*sizeof(char));
    if(!buffer){
        fprintf(stderr, "[tinylisp] Failed to allocate code buffer!\n");
        return EXIT_FAILURE;
    }
    fread(buffer, 1, sz, fp);
    fclose(fp);
    tl_init(&lisp, buffer, sz);
    rc = tl_run(&lisp, onerror, &lisp);
    tl_free(&lisp);
    free(buffer);
    return rc;
}

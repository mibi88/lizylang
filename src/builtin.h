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
 * 2024/10/03: Created file.
 * 2024/10/04: Adding some functions.
 * 2024/10/09: Started adding function definition.
 * 2024/10/18: Fixed the prototypes.
 */

#ifndef BUILTIN_H
#define BUILTIN_H

#include <call.h>

int builtin_register_funcs(LizyLang *lisp);
int builtin_comment(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_strdef(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_numdef(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_set(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_del(void *_lisp, void *_node, size_t argnum,  void *_returned);
int builtin_print(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_printraw(void *_lisp, void *_node, size_t argnum,
                     void *_returned);
int builtin_input(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_add(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_merge(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_params(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_list(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_fncdef(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_defend(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_if(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_smaller(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_bigger(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_smaller_or_equal(void *_lisp, void *_node, size_t argnum,
                             void *_returned);
int builtin_bigger_or_equal(void *_lisp, void *_node, size_t argnum,
                            void *_returned);
int builtin_equal(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_not_equal(void *_lisp, void *_node, size_t argnum,
                      void *_returned);
int builtin_substract(void *_lisp, void *_node, size_t argnum,
                      void *_returned);
int builtin_multiply(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_divide(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_modulo(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_floor(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_ceil(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_parsenum(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_callif(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_len(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_strlen(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_get(void *_lisp, void *_node, size_t argnum, void *_returned);
int builtin_strget(void *_lisp, void *_node, size_t argnum, void *_returned);

#endif

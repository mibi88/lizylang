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
 * 2024/09/28: Started developement. String parsing, started parsing calls.
 * 2024/09/29: Coded parser and lexer.
 * 2024/09/30: Nearly working lexer and parser. Adding calling: pushing args to
 *             argstack.
 * 2024/10/04: Added function calling.
 * 2024/10/09: Do not call functions inside a function definition. Started
 *             adding builtin function definition and calling.
 * 2024/10/12: Call user defined functions with arguments. Return a value from
 *             user functions.
 * 2024/10/15: Store the tree and start generating it. Store the currently
 *             executed node.
 * 2024/10/19: Preparing call-by-need evaluation.
 * 2024/10/20: New stack.
 * 2024/10/21: Perform calls in the right context.
 */

#ifndef LISP_H
#define LISP_H

#include <tree.h>
#include <defs.h>
#include <var.h>

typedef struct {
    char *buffer;
    size_t sz;
    Var *vars;
    String *var_names;
    size_t var_num;
    struct{
        Function *function;
        void *call;
        Var *args;
        char *evaluated;
        size_t parent;
    }stack[TL_STACK_SZ];
    size_t stack_cur;
    Call fstack[TL_FSTACK_SZ];
    size_t fstack_cur;
    Var argstack[TL_ARGSTACK_SZ];
    size_t argstack_fnc[TL_ARGSTACK_SZ];
    size_t argstack_cur;
    size_t line;
    Var last;
    Node node;
    void *current_node;
    size_t context;
} LizyLang;

int tl_init(LizyLang *lisp, char *buffer, size_t sz);
int tl_add_var(LizyLang *lisp, Var *var, String *name);
int tl_set_var(LizyLang *lisp, Var *var, String *name);
int tl_del_var(LizyLang *lisp, String *name);
int tl_run(LizyLang *lisp, void error(char*, void*), void *data);
int tl_free(LizyLang *lisp);

#endif

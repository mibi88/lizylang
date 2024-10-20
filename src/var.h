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
 * 2024/09/28: Started developement. Created some data structures.
 * 2024/09/29: Added string and call creation and free functions.
 * 2024/09/30: Pushing args to argstack.
 * 2024/10/02: Save argstack cursor instead of item number on call.
 * 2024/10/03: Fixed memory leaks.
 * 2024/10/07: Return value of functions.
 * 2024/10/08: Add macros to handle lists, starting to add list support.
 * 2024/10/14: A call is now a variable type.
 * 2024/10/15: Adapt some structs to the tree. Define TL_T_CALL.
 * 2024/10/16: Removed useless values in structs.
 * 2024/10/18: Fixed builtin function prototype.
 * 2024/10/20: Better name.
 */

#ifndef VAR_H
#define VAR_H

#include <defs.h>

#define VAR_LEN(var) (var)->size
#define VAR_GET_ITEM(var, i) (var)->items[i]
#define VAR_STR_DATA(item) (item).string.data
#define VAR_STR_LEN(item) (item).string.len
#define VAR_NUM(item) (item).num
#define VAR_BUILTIN_FUNC(item) (item).function.ptr.f
#define VAR_IS_BUILTIN(item) (item).function.builtin
#define VAR_PARSEARGS(item) (item).function.parseargs
#define VAR_USER_FUNC(item) (item).function.ptr.start

enum {
    TL_T_FUNC,
    TL_T_STR,
    TL_T_NUM,
    TL_T_NAME,
    TL_T_CALL
};

typedef struct {
    char *data;
    size_t len;
} String;

typedef struct {
    union {
        void *fncdef;
        int (*f)(void *lisp, void* node, size_t argnum, void* returned);
    } ptr;
    char builtin;
    char parseargs;
    void *params;
} Function;

typedef struct {
    String function;
    char has_func;
} Call;

typedef union {
    float num;
    String string;
    Function function;
    Call call;
} Item;

typedef struct {
    Item *items;
    size_t size;
    unsigned char type;
    char null;
} Var;

int var_auto(Var *var, char *data, size_t len);
int var_str(Var *var, char *data, size_t len);
int var_str_concat(Var *var, Var *str1, Var *str2);
int var_str_add(Var *var, char *data, size_t len);
int var_raw_str(String *string, char *data, size_t len);
int var_builtin_func(Var *var, int f(void*, void*, size_t, void*),
                     char parse);
int var_user_func(Var *var, void *fncdef, Var *params);
char var_isnum(char *data, size_t len);
int var_num(Var *var, char *data, size_t len);
int var_num_from_float(Var *var, float num);
char var_isname(char *data, size_t len);
int var_copy(Var *src, Var *dest);
int var_call(Var *var, char *name, size_t len);

int var_free_call(Call *call);
int var_free_str(String *string);
int var_free(Var *var);

int var_append(Var *src, Var *dest);

#endif

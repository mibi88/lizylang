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
 * 2024/10/04: Load functions and added strdef.
 * 2024/10/07: Add print function.
 * 2024/10/08: Use macros when reading from variables and added merge and
 *             numdef functions.
 * 2024/10/09: Parse value in numdef and strdef.
 * 2024/10/12: Fix user function calling bugs. Call user defined function with
 *             arguments. Return value from function. Added conditions.
 * 2024/10/13: Added list management functions.
 * 2024/10/16: Started adding calling back.
 * 2024/10/18: Update the prototypes.
 * 2024/10/19: Updated some functions. Removed defend.
 * 2024/10/20: Finish user function definition.
 * 2024/10/21: Fixed functions.
 */

#include <builtin.h>

#define TL_REGISTER_FUNC(s, parse, f) rc = var_raw_str(&name, s, \
                                                       sizeof(s)-1); \
                                      if(rc) return rc; \
                                      rc = var_builtin_func(&var, f, parse); \
                                      if(rc) return rc; \
                                      rc = tl_add_var(lisp, &var, &name); \
                                      if(rc) return rc

int builtin_register_funcs(LizyLang *lisp) {
    int rc;
    Var var;
    String name;
    /* strdef */
    TL_REGISTER_FUNC("strdef", 0, builtin_strdef);
    TL_REGISTER_FUNC("numdef", 0, builtin_numdef);
    TL_REGISTER_FUNC("set", 0, builtin_set);
    TL_REGISTER_FUNC("del", 0, builtin_del);
    /* comment */
    TL_REGISTER_FUNC("comment", 0, builtin_comment);
    TL_REGISTER_FUNC("print", 1, builtin_print);
    TL_REGISTER_FUNC("printraw", 0, builtin_printraw);
    TL_REGISTER_FUNC("input", 1, builtin_input);
    TL_REGISTER_FUNC("+", 1, builtin_add);
    TL_REGISTER_FUNC("++", 1, builtin_merge);
    TL_REGISTER_FUNC("params", 0, builtin_params);
    TL_REGISTER_FUNC("list", 1, builtin_list);
    TL_REGISTER_FUNC("fncdef", 0, builtin_fncdef);
    TL_REGISTER_FUNC("if", 1, builtin_if);
    TL_REGISTER_FUNC("<", 1, builtin_smaller);
    TL_REGISTER_FUNC(">", 1, builtin_bigger);
    TL_REGISTER_FUNC("<=", 1, builtin_smaller_or_equal);
    TL_REGISTER_FUNC(">=", 1, builtin_bigger_or_equal);
    TL_REGISTER_FUNC("=", 1, builtin_equal);
    TL_REGISTER_FUNC("!=", 1, builtin_not_equal);
    TL_REGISTER_FUNC("-", 1, builtin_substract);
    TL_REGISTER_FUNC("*", 1, builtin_multiply);
    TL_REGISTER_FUNC("/", 1, builtin_divide);
    TL_REGISTER_FUNC("%", 1, builtin_modulo);
    TL_REGISTER_FUNC("floor", 1, builtin_floor);
    TL_REGISTER_FUNC("ceil", 1, builtin_ceil);
    TL_REGISTER_FUNC("parsenum", 1, builtin_parsenum);
    TL_REGISTER_FUNC("callif", 0, builtin_callif);
    TL_REGISTER_FUNC("len", 1, builtin_len);
    TL_REGISTER_FUNC("get", 1, builtin_get);
    TL_REGISTER_FUNC("strlen", 1, builtin_strlen);
    TL_REGISTER_FUNC("strget", 1, builtin_strget);
    /* TODO: numstr: Convert float to string. */
    /* TODO: head and tail */
    return TL_SUCCESS;
}

int builtin_comment(void *_lisp, void *_node, size_t argnum, void *_returned) {
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_node);
    TL_UNUSED(argnum);
    TL_UNUSED(_returned);
    rc = var_str(_returned, "", 0);
    if(rc) return rc;
    return TL_SUCCESS;
}

int builtin_strdef(void *_lisp, void *_node, size_t argnum, void *_returned) {
    LizyLang *lisp = _lisp;
    Node *node = _node;
    Var varname;
    Var value;
    String name;
    int rc;
    TL_UNUSED(_node);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    rc = call_get_arg(lisp, node, 0, &varname, 0);
    if(rc) return rc;
    rc = call_get_arg(lisp, node, 1, &value, 1);
    if(rc){
        var_free(&value);
        return rc;
    }
    if(varname.type != TL_T_NAME) return TL_ERR_BAD_TYPE;
    if(VAR_LEN(&varname) != 1) return TL_ERR_INVALID_LIST_SIZE;
    if(value.type != TL_T_STR) return TL_ERR_BAD_TYPE;
    rc = var_raw_str(&name, VAR_STR_DATA(VAR_GET_ITEM(&varname, 0)),
                     VAR_STR_LEN(VAR_GET_ITEM(&varname, 0)));
    var_free(&varname);
    if(rc){
        var_free(&value);
        free(name.data);
        return rc;
    }
    rc = tl_add_var(lisp, &value, &name);
    if(rc){
        var_free(&value);
        free(name.data);
        return rc;
    }
    rc = var_copy(&value, _returned);
    if(rc){
        var_free(&value);
        free(name.data);
        return rc;
    }
    return TL_SUCCESS;
}

int builtin_numdef(void *_lisp, void *_node, size_t argnum, void *_returned) {
    LizyLang *lisp = _lisp;
    Node *node = _node;
    Var varname;
    Var value;
    String name;
    int rc;
    TL_UNUSED(_node);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    rc = call_get_arg(lisp, node, 0, &varname, 0);
    if(rc) return rc;
    rc = call_get_arg(lisp, node, 1, &value, 1);
    if(rc){
        var_free(&value);
        return rc;
    }
    if(varname.type != TL_T_NAME) return TL_ERR_BAD_TYPE;
    if(VAR_LEN(&varname) != 1) return TL_ERR_INVALID_LIST_SIZE;
    if(value.type != TL_T_NUM) return TL_ERR_BAD_TYPE;
    rc = var_raw_str(&name, VAR_STR_DATA(VAR_GET_ITEM(&varname, 0)),
                     VAR_STR_LEN(VAR_GET_ITEM(&varname, 0)));
    var_free(&varname);
    if(rc){
        var_free(&value);
        free(name.data);
        return rc;
    }
    rc = tl_add_var(lisp, &value, &name);
    if(rc){
        var_free(&value);
        free(name.data);
        return rc;
    }
    rc = var_copy(&value, _returned);
    if(rc){
        var_free(&value);
        free(name.data);
        return rc;
    }
    return TL_SUCCESS;
}

int builtin_set(void *_lisp, void *_node, size_t argnum, void *_returned) {
    LizyLang *lisp = _lisp;
    Var *args = NULL; /* TODO: Fix required! */
    Var value;
    int rc;
    TL_UNUSED(_node);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(args[0].type != TL_T_NAME) return TL_ERR_BAD_TYPE;
    if(VAR_LEN(args) != 1) return TL_ERR_INVALID_LIST_SIZE;
    rc = call_parse_arg(lisp, args+1, &value, lisp->stack_cur);
    if(rc) return rc;
    rc = tl_set_var(lisp, &value, &args->items->string);
    if(rc){
        var_free(&value);
        return rc;
    }
    rc = var_free(&value);
    if(rc) return rc;
    rc = var_num_from_float(_returned, 0);
    return rc;
}

int builtin_del(void *_lisp, void *_node, size_t argnum,  void *_returned) {
    LizyLang *lisp = _lisp;
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    TL_UNUSED(_node);
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    if(args[0].type != TL_T_NAME) return TL_ERR_BAD_TYPE;
    if(VAR_LEN(args) != 1) return TL_ERR_INVALID_LIST_SIZE;
    rc = tl_del_var(lisp, &args->items->string);
    if(rc){
        return rc;
    }
    rc = var_num_from_float(_returned, 0);
    return rc;
}

int builtin_print(void *_lisp, void *_node, size_t argnum, void *_returned) {
    LizyLang *lisp = _lisp;
    Node *node = _node;
    int rc;
    size_t i;
    Var data;
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    rc = call_get_arg(lisp, node, 0, &data, 1);
    if(rc) return rc;
    if(VAR_LEN(&data) < 1){
        puts("()");
        var_free(&data);
        return TL_SUCCESS;
    }
    if(VAR_LEN(&data) > 1) fputc('(', stdout);
    for(i=0;i<VAR_LEN(&data);i++){
        switch(data.type){
            case TL_T_STR:
                if(VAR_LEN(&data) > 1) fputc('"', stdout);
                fwrite(VAR_STR_DATA(VAR_GET_ITEM(&data, i)), 1,
                       VAR_STR_LEN(VAR_GET_ITEM(&data, i)), stdout);
                if(VAR_LEN(&data) > 1) fputc('"', stdout);
                if(i < VAR_LEN(&data)-1) fputc(' ', stdout);
                break;
            case TL_T_NUM:
                /* TODO: Custom number conversion function. */
                printf("%f", VAR_NUM(VAR_GET_ITEM(&data, i)));
                if(i < VAR_LEN(&data)-1) fputc(' ', stdout);
                break;
            default:
                var_free(&data);
                return TL_ERR_BAD_TYPE;
        }
    }
    if(VAR_LEN(&data) > 1) fputc(')', stdout);
    fputc('\n', stdout);
    rc = var_copy(&data, _returned);
    var_free(&data);
    if(rc) return rc;
    return TL_SUCCESS;
}

int builtin_printraw(void *_lisp, void *_node, size_t argnum,
                     void *_returned) {
    LizyLang *lisp = _lisp;
    Node *node = _node;
    int rc;
    size_t i;
    Var data;
    TL_UNUSED(_lisp);
    TL_UNUSED(_node);
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    rc = call_get_arg(lisp, node, 0, &data, 0);
    if(rc) return rc;
    if(VAR_LEN(&data) < 1){
        puts("()");
        var_free(&data);
        return TL_SUCCESS;
    }
    if(VAR_LEN(&data) > 1) fputc('(', stdout);
    for(i=0;i<VAR_LEN(&data);i++){
        switch(data.type){
            case TL_T_STR:
                if(VAR_LEN(&data) > 1) fputc('"', stdout);
                fwrite(VAR_STR_DATA(VAR_GET_ITEM(&data, i)), 1,
                       VAR_STR_LEN(VAR_GET_ITEM(&data, i)), stdout);
                if(VAR_LEN(&data) > 1) fputc('"', stdout);
                if(i < VAR_LEN(&data)-1) fputc(' ', stdout);
                break;
            case TL_T_NAME:
                fputs("<variable: ", stdout);
                fwrite(VAR_STR_DATA(VAR_GET_ITEM(&data, i)), 1,
                       VAR_STR_LEN(VAR_GET_ITEM(&data, i)), stdout);
                fputc('>', stdout);
                if(i < VAR_LEN(&data)-1) fputc(' ', stdout);
                break;
            case TL_T_NUM:
                /* TODO: Custom number conversion function. */
                printf("%f", VAR_NUM(VAR_GET_ITEM(&data, i)));
                if(i < VAR_LEN(&data)-1) fputc(' ', stdout);
                break;
            default:
                var_free(&data);
                return TL_ERR_BAD_TYPE;
        }
    }
    if(VAR_LEN(&data) > 1) fputc(')', stdout);
    fputc('\n', stdout);
    rc = var_copy(&data, _returned);
    var_free(&data);
    if(rc) return rc;
    return TL_SUCCESS;
}

int builtin_input(void *_lisp, void *_node, size_t argnum, void *_returned) {
    LizyLang *lisp = _lisp;
    Node *node = _node;
    int rc;
    char c;
    Var str;
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    rc = call_get_arg(lisp, node, 0, &str, 1);
    if(rc) return rc;
    if(str.type != TL_T_STR){
        var_free(&str);
        return TL_ERR_BAD_TYPE;
    }
    if(VAR_LEN(&str) != 1){
        var_free(&str);
        return TL_ERR_INVALID_LIST_SIZE;
    }
    fwrite(VAR_STR_DATA(VAR_GET_ITEM(&str, 0)), 1,
           VAR_STR_LEN(VAR_GET_ITEM(&str, 0)), stdout);
    var_str(_returned, "", 0);
    while((c = getc(stdin)) != '\n'){
        rc = var_str_add(_returned, &c, 1);
        if(rc){
            var_free(&str);
            return rc;
        }
    }
    var_free(&str);
    return TL_SUCCESS;
}

int builtin_add(void *_lisp, void *_node, size_t argnum, void *_returned) {
    LizyLang *lisp = _lisp;
    Node *node = _node;
    int rc;
    Var a;
    Var b;
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    rc = call_get_arg(lisp, node, 0, &a, 1);
    if(rc) return rc;
    rc = call_get_arg(lisp, node, 1, &b, 1);
    if(rc){
        var_free(&a);
        return rc;
    }
    else if(a.type != b.type){
        var_free(&a);
        var_free(&b);
        return TL_ERR_BAD_TYPE;
    }
    if(VAR_LEN(&a) != 1){
        var_free(&a);
        var_free(&b);
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(VAR_LEN(&b) != 1){
        var_free(&a);
        var_free(&b);
        return TL_ERR_INVALID_LIST_SIZE;
    }
    switch(a.type){
        case TL_T_STR:
            rc = var_str_concat(_returned, &a, &b);
            var_free(&a);
            var_free(&b);
            if(rc) return rc;
            break;
        case TL_T_NUM:
            rc = var_num_from_float(_returned, VAR_NUM(VAR_GET_ITEM(&a, 0))+
                                    VAR_NUM(VAR_GET_ITEM(&b, 0)));
            var_free(&a);
            var_free(&b);
            if(rc) return rc;
            break;
        default:
            var_free(&a);
            var_free(&b);
            return TL_ERR_BAD_TYPE;
    }
    return TL_SUCCESS;
}

int builtin_merge(void *_lisp, void *_node, size_t argnum, void *_returned) {
    int rc;
    Var *args = NULL; /* TODO: Fix required! */
    TL_UNUSED(_lisp);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    rc = var_copy(args, _returned);
    if(rc) return rc;
    rc = var_append(args+1, _returned);
    if(rc) return rc;
    return TL_SUCCESS;
}

int builtin_params(void *_lisp, void *_node, size_t argnum, void *_returned) {
    LizyLang *lisp = _lisp;
    Node *node = _node;
    Var param;
    int rc;
    size_t i;
    if(!argnum){
        ((Var*)_returned)->null = 0;
        ((Var*)_returned)->size = 0;
        ((Var*)_returned)->items = NULL;
        ((Var*)_returned)->type = TL_T_NAME;
        return TL_SUCCESS;
    }
    rc = call_get_arg(lisp, node, 0, _returned, 0);
    if(rc) return rc;
    for(i=1;i<argnum;i++){
        rc = call_get_arg(lisp, node, i, &param, 0);
        if(rc){
            var_free(_returned);
            return rc;
        }
        rc = var_append(&param, _returned);
        var_free(&param);
        if(rc){
            var_free(_returned);
            return rc;
        }
    }
    return TL_SUCCESS;
}

int builtin_list(void *_lisp, void *_node, size_t argnum, void *_returned) {
    int rc;
    Var *args = NULL; /* TODO: Fix required! */
    size_t i;
    TL_UNUSED(_lisp);
    TL_UNUSED(_node);
    if(!argnum){
        ((Var*)_returned)->null = 0;
        ((Var*)_returned)->size = 0;
        ((Var*)_returned)->items = NULL;
        ((Var*)_returned)->type = TL_T_NUM;
        return TL_SUCCESS;
    }
    rc = var_copy(args, _returned);
    if(rc) return rc;
    for(i=1;i<argnum;i++){
        rc = var_append(args+i, _returned);
        if(rc){
            var_free(_returned);
            return rc;
        }
    }
    return TL_SUCCESS;
}

int builtin_fncdef(void *_lisp, void *_node, size_t argnum, void *_returned) {
    LizyLang *lisp = _lisp;
    Node *node = _node;
    int rc;
    size_t i;
    Var function;
    Var fncname;
    Var params;
    Var *raw;
    String name;
    if(argnum < 3) return TL_ERR_TOO_FEW_ARGS;
    for(i=2;i<argnum;i++){
        rc = call_get_arg_raw(node, i, &raw);
        if(raw->type != TL_T_CALL) return TL_ERR_BAD_TYPE;
        if(VAR_LEN(raw) != 1) return TL_ERR_INVALID_LIST_SIZE;
        if(rc) return rc;
    }
    rc = call_get_arg(lisp, node, 0, &fncname, 0);
    if(rc) return rc;
    rc = call_get_arg(lisp, node, 1, &params, 0);
    if(rc){
        var_free(&fncname);
        return rc;
    }
    if(VAR_LEN(&fncname) != 1){
        var_free(&fncname);
        var_free(&params);
        return TL_ERR_INVALID_LIST_SIZE;
    }
    rc = var_user_func(&function, node, &params);
    if(rc){
        var_free(&fncname);
        var_free(&params);
        return rc;
    }
    rc = var_raw_str(&name, VAR_STR_DATA(VAR_GET_ITEM(&fncname, 0)),
                     VAR_STR_LEN(VAR_GET_ITEM(&fncname, 0)));
    if(rc){
        var_free(&function);
        var_free(&fncname);
        var_free(&params);
        free(name.data);
        return rc;
    }
    rc = tl_add_var(lisp, &function, &name);
    if(rc){
        var_free(&function);
        var_free(&fncname);
        var_free(&params);
        free(name.data);
        return rc;
    }
    /* TODO: Store calls. */
    var_free(&fncname);
    var_free(&params);
    rc = var_num_from_float(_returned, 0);
    return rc;
}

int builtin_if(void *_lisp, void *_node, size_t argnum, void *_returned) {
    Var condition;
    int rc;
    TL_UNUSED(_lisp);
    if(argnum < 3) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 3) return TL_ERR_TOO_MANY_ARGS;
    rc = call_get_arg(_lisp, _node, 0, &condition, 1);
    if(rc) return rc;
    if(VAR_LEN(&condition) != 1){
        var_free(&condition);
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(condition.type != TL_T_NUM){
        var_free(&condition);
        return TL_ERR_BAD_TYPE;
    }
    if(condition.items->num != 0){
        rc = call_get_arg(_lisp, _node, 1, _returned, 1);
        var_free(&condition);
        return rc;
    }
    rc = call_get_arg(_lisp, _node, 2, _returned, 1);
    var_free(&condition);
    return rc;
}

int builtin_smaller(void *_lisp, void *_node, size_t argnum, void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    TL_UNUSED(_lisp);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM || args[1].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    if(args[0].items->num < args[1].items->num){
        rc = var_num_from_float(_returned, 1);
        return rc;
    }
    rc = var_num_from_float(_returned, 0);
    return rc;
}

int builtin_bigger(void *_lisp, void *_node, size_t argnum, void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    TL_UNUSED(_lisp);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM || args[1].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    if(args[0].items->num > args[1].items->num){
        rc = var_num_from_float(_returned, 1);
        return rc;
    }
    rc = var_num_from_float(_returned, 0);
    return rc;
}

int builtin_smaller_or_equal(void *_lisp, void *_node, size_t argnum,
                             void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    TL_UNUSED(_lisp);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM || args[1].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    if(args[0].items->num <= args[1].items->num){
        rc = var_num_from_float(_returned, 1);
        return rc;
    }
    rc = var_num_from_float(_returned, 0);
    return rc;
}

int builtin_bigger_or_equal(void *_lisp, void *_node, size_t argnum,
                            void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    TL_UNUSED(_lisp);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM || args[1].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    if(args[0].items->num >= args[1].items->num){
        rc = var_num_from_float(_returned, 1);
        return rc;
    }
    rc = var_num_from_float(_returned, 0);
    return rc;
}

int builtin_equal(void *_lisp, void *_node, size_t argnum, void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    TL_UNUSED(_lisp);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != args[1].type){
        return TL_ERR_BAD_TYPE;
    }
    switch(args[0].type){
        case TL_T_STR:
            if(args[0].items->string.len == args[1].items->string.len){
                if(!memcmp(args[0].items->string.data,
                           args[1].items->string.data,
                           args[0].items->string.len)){
                    rc = var_num_from_float(_returned, 1);
                    return rc;
                }
            }
            rc = var_num_from_float(_returned, 0);
            return rc;
        case TL_T_NUM:
            if(args[0].items->num == args[1].items->num){
                rc = var_num_from_float(_returned, 1);
                return rc;
            }
            rc = var_num_from_float(_returned, 0);
            return rc;
        default:
            return TL_ERR_BAD_TYPE;
    }
    return TL_SUCCESS;
}

int builtin_not_equal(void *_lisp, void *_node, size_t argnum,
                      void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    TL_UNUSED(_lisp);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != args[1].type){
        return TL_ERR_BAD_TYPE;
    }
    switch(args[0].type){
        case TL_T_STR:
            if(args[0].items->string.len == args[1].items->string.len){
                if(!memcmp(args[0].items->string.data,
                           args[1].items->string.data,
                           args[0].items->string.len)){
                    rc = var_num_from_float(_returned, 0);
                    return rc;
                }
            }
            rc = var_num_from_float(_returned, 1);
            return rc;
        case TL_T_NUM:
            if(args[0].items->num == args[1].items->num){
                rc = var_num_from_float(_returned, 0);
                return rc;
            }
            rc = var_num_from_float(_returned, 1);
            return rc;
        default:
            return TL_ERR_BAD_TYPE;
    }
    return TL_SUCCESS;
}

int builtin_substract(void *_lisp, void *_node, size_t argnum,
                      void *_returned) {
    LizyLang *lisp = _lisp;
    Node *node = _node;
    int rc;
    Var a;
    Var b;
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    rc = call_get_arg(lisp, node, 0, &a, 1);
    if(rc) return rc;
    rc = call_get_arg(lisp, node, 1, &b, 1);
    if(rc){
        var_free(&a);
        return rc;
    }
    if(VAR_LEN(&a) != 1 || VAR_LEN(&b) != 1){
        var_free(&a);
        var_free(&b);
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(a.type != TL_T_NUM || b.type != TL_T_NUM){
        var_free(&a);
        var_free(&b);
        return TL_ERR_BAD_TYPE;
    }
    rc = var_num_from_float(_returned, a.items->num-b.items->num);
    var_free(&a);
    var_free(&b);
    return rc;
}

int builtin_multiply(void *_lisp, void *_node, size_t argnum,
                     void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    TL_UNUSED(_lisp);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM || args[1].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    rc = var_num_from_float(_returned, args[0].items->num*args[1].items->num);
    return rc;
}

int builtin_divide(void *_lisp, void *_node, size_t argnum, void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    TL_UNUSED(_lisp);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM || args[1].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    if(args[1].items->num == 0){
        return TL_ERR_DIVISION_BY_ZERO;
    }
    rc = var_num_from_float(_returned, args[0].items->num/args[1].items->num);
    return rc;
}

int builtin_modulo(void *_lisp, void *_node, size_t argnum, void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    TL_UNUSED(_lisp);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM || args[1].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    if(args[1].items->num == 0){
        return TL_ERR_DIVISION_BY_ZERO;
    }
    rc = var_num_from_float(_returned, fmod(args[0].items->num,
                                            args[1].items->num));
    return rc;
}

int builtin_floor(void *_lisp, void *_node, size_t argnum, void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    TL_UNUSED(_lisp);
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    rc = var_num_from_float(_returned, floor(args[0].items->num));
    return rc;
}

int builtin_ceil(void *_lisp, void *_node, size_t argnum, void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    TL_UNUSED(_lisp);
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    rc = var_num_from_float(_returned, ceil(args[0].items->num));
    return rc;
}

int builtin_parsenum(void *_lisp, void *_node, size_t argnum,
                     void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    TL_UNUSED(_lisp);
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_STR){
        return TL_ERR_BAD_TYPE;
    }
    if(!var_isnum(args[0].items->string.data, args[0].items->string.len)){
        return TL_ERR_BAD_INPUT;
    }
    rc = var_num(_returned, args[0].items->string.data,
                 args[0].items->string.len);
    return rc;
}

int builtin_callif(void *_lisp, void *_node, size_t argnum, void *_returned) {
    /* TODO: Rewrite it! */
    return TL_SUCCESS;
}

int builtin_len(void *_lisp, void *_node, size_t argnum, void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    TL_UNUSED(_lisp);
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    rc = var_num_from_float(_returned, VAR_LEN(args));
    return rc;
}

int builtin_strlen(void *_lisp, void *_node, size_t argnum, void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    TL_UNUSED(_lisp);
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1) return TL_ERR_INVALID_LIST_SIZE;
    if(args->type != TL_T_STR) return TL_ERR_BAD_TYPE;
    rc = var_num_from_float(_returned, args->items->string.len);
    return rc;
}

int builtin_get(void *_lisp, void *_node, size_t argnum, void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    int index;
    TL_UNUSED(_lisp);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(args[1].type != TL_T_NUM) return TL_ERR_BAD_TYPE;
    if(VAR_LEN(args+1) != 1) return TL_ERR_INVALID_LIST_SIZE;
    index = (int)args[1].items->num;
    if(index < 0 || (size_t)index >= VAR_LEN(args)){
        return TL_ERR_OUT_OF_RANGE;
    }
    switch(args->type){
        case TL_T_NAME:
            rc = var_str(_returned, args->items[index].string.data,
                         args->items[index].string.len);
            if(!rc) ((Var*)_returned)->type = TL_T_NAME;
            return rc;
        case TL_T_STR:
            rc = var_str(_returned, args->items[index].string.data,
                         args->items[index].string.len);
            return rc;
        case TL_T_NUM:
            rc = var_num_from_float(_returned, args->items[index].num);
            return rc;
        default:
            return TL_ERR_BAD_TYPE;
    }
    return TL_SUCCESS;
}

int builtin_strget(void *_lisp, void *_node, size_t argnum, void *_returned) {
    Var *args = NULL; /* TODO: Fix required! */
    int rc;
    int index;
    TL_UNUSED(_lisp);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1) return TL_ERR_INVALID_LIST_SIZE;
    if(VAR_LEN(args+1) != 1) return TL_ERR_INVALID_LIST_SIZE;
    if(args->type != TL_T_STR) return TL_ERR_BAD_TYPE;
    if(args[1].type != TL_T_NUM) return TL_ERR_BAD_TYPE;
    index = (int)args[1].items->num;
    if(index < 0 || (size_t)index >= args->items->string.len){
        return TL_ERR_OUT_OF_RANGE;
    }
    rc = var_str(_returned, args->items->string.data+index, sizeof(char));
    return rc;
}

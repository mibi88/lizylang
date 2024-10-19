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
 * 2024/09/30: Created file.
 * 2024/10/03: Adding argument parsing.
 * 2024/10/04: Added types and finished argument parsing.
 * 2024/10/07: Fixed function calling and parsing.
 * 2024/10/09: Parse single argument with call_parse_arg.
 * 2024/10/12: Fix user function calling bugs. Pass arguments to user defined
 *             functions.
 * 2024/10/16: Started adding calling back.
 * 2024/10/19: Adding builtin function calling back.
 */

#include <call.h>

#define TL_MIN(a, b) ((a) < (b) ? (a) : (b))

int call_exec(TinyLisp *lisp, Node *node, Var *returned) {
    Function *function;
    char found;
    size_t i;
    if(node->var->type != TL_T_CALL){
        return TL_ERR_VALUE_OUTSIDE_OF_CALL;
    }
    if(node->var->size != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
#if TL_DEBUG_CALL
    fputs("Calling \"", stdout);
    fwrite(node->var->items->call.function.data, 1,
           node->var->items->call.function.len, stdout);
    puts("\"");
#endif
    /* Find the function */
    found = 0;
    for(i=0;i<lisp->var_num;i++){
        if(lisp->vars[i].type == TL_T_FUNC){
#if TL_DEBUG_VARS
            fputc('"', stdout);
            fwrite(lisp->var_names[i].data, 1,
                   lisp->var_names[i].len, stdout);
            fputs("\", \"", stdout);
            fwrite(node->var->items->call.function.data, 1,
                   node->var->items->call.function.len, stdout);
            fputs("\"\n", stdout);
#endif
            if(lisp->var_names[i].len != node->var->items->call.function.len){
                continue;
            }
            if(!memcmp(lisp->var_names[i].data,
                       node->var->items->call.function.data,
                       lisp->var_names[i].len)){
                function = &lisp->vars[i].items->function;
                found = 1;
            }
        }
    }
    if(!found){
        return TL_ERR_FUNC_NOT_DEF;
    }
    if(function->builtin){
        /* Call the right builtin function. */
        function->ptr.f(lisp, node, node->childnum, returned);
    }
    return TL_SUCCESS;
}

int call_get_arg(TinyLisp *lisp, Node *node, size_t idx, Var *dest,
                 char parse) {
    Var parsed;
    Var *src;
    int rc;
    if(idx >= node->childnum) return TL_ERR_TOO_FEW_ARGS;
    src = ((Node**)node->childs)[idx]->var;
    if(src->type == TL_T_CALL){
        rc = call_exec(lisp, ((Node**)node->childs)[idx], dest);
        if(rc) return rc;
    }else{
        var_copy(src, dest);
    }
    if(parse){
        rc = call_parse_arg(lisp, dest, &parsed);
        var_free(dest);
        if(rc){
            return rc;
        }
        var_copy(&parsed, dest);
    }
    return TL_SUCCESS;
}

int call_parse_arg(TinyLisp *lisp, Var *src, Var *dest) {
    char found = 0;
    int rc;
    size_t n;
    if(!src->size){
        dest->size = 0;
        dest->items = NULL;
        if(src->type == TL_T_NAME){
            return TL_ERR_INVALID_NAME;
        }
        dest->type = src->type;
    }
    if(src->type == TL_T_NAME){
        if(lisp->stack_cur){
            for(n=0;n<lisp->stack[lisp->stack_cur-1].params.size;n++){
                if(src->items[0].string.len !=
                   lisp->stack[lisp->stack_cur-1]
                   .params.items[n].string.len){
                    continue;
                }
                if(!memcmp(lisp->stack[lisp->stack_cur-1]
                           .params.items[n].string.data,
                           src->items[0].string.data,
                           src->items[0].string.len)){
                    rc = var_copy(lisp->stack[lisp->stack_cur-1].args+n,
                                  dest);
                    if(rc){
                        return rc;
                    }
                    found = 1;
                    break;
                }
            }
        }
        if(!found){
            for(n=0;n<lisp->var_num;n++){
#if TL_DEBUG_VARS
                fwrite(lisp->var_names[n].data, 1, lisp->var_names[n].len,
                       stdout);
                fputs(", ", stdout);
                fwrite(src->items[0].string.data, 1,
                       src->items[0].string.len, stdout);
                fputc('\n', stdout);
#endif
                if(lisp->var_names[n].len != src->items[0].string.len){
                    continue;
                }
                if(!memcmp(lisp->var_names[n].data,
                           src->items[0].string.data,
                           lisp->var_names[n].len)){
                    rc = var_copy(lisp->vars+n, dest);
                    if(rc){
                        return rc;
                    }
                    found = 1;
                    break;
                }
            }
        }
        if(!found){
            return TL_ERR_NOT_DEF;
        }
    }else{
        rc = var_copy(src, dest);
        if(rc){
            return rc;
        }
    }
    return EXIT_SUCCESS;
}

#undef TL_MIN

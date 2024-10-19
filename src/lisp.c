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
 * 2024/09/30: Nearly working lexer and parser + adding calling.
 * 2024/10/02: Working calling.
 * 2024/10/03: Free args and added error checking, fixed memory leaks.
 * 2024/10/04: Added argument parsing and fixed memory leaks.
 * 2024/10/08: Started adding list support.
 * 2024/10/09: Do not call functions inside a function definition.
 *             Error when redefining variable. Added void list support.
 * 2024/10/12: Call user defined functions with arguments. Return value from
 *             user defined function. Fix leak. Set and delete variables.
 * 2024/10/13: Added list management functions.
 * 2024/10/14: Moved variable management functions down.
 * 2024/10/15: Start generating a tree.
 * 2024/10/16: Add data to the nodes. Finish generating the tree. Better tree
 *             debugging.
 * 2024/10/19: Handle errors when calling functions.
 */

#include <lisp.h>
#include <platform.h>
#include <call.h>
#include <builtin.h>
#include <tree.h>

int tl_init(TinyLisp *lisp, char *buffer, size_t sz) {
    lisp->buffer = buffer;
    lisp->sz = sz;
    lisp->var_num = 0;
    lisp->vars = NULL;
    lisp->var_names = NULL;
    lisp->stack_cur = 0;
    lisp->fstack_cur = 0;
    lisp->argstack_cur = 0;
    lisp->perform_calls = 1;
    node_init(&lisp->node, NULL);
#if TL_LEAK_CHECK
    mtrace();
#endif
    builtin_register_funcs(lisp);
    return var_num_from_float(&lisp->last, 0);
}

#define TL_ERROR(err) error((char*)messages[err], data); return err
#define TL_TOK_ADD(c) token[token_cur++] = c; \
                      if(token_cur >= TL_TOKEN_SZ){ \
                          TL_ERROR(TL_ERR_TOKFULL); \
                          return TL_ERR_TOKFULL; \
                      }

int tl_run(TinyLisp *lisp, void error(char*, void*), void *data) {
    char c;
    char token[TL_TOKEN_SZ];
    size_t token_cur = 0;
    char in_string = 0;
    char escaped = 0;
    char in_hex = 0;
    char hexnum;
    int rc;
    size_t i;
    Node *allocated;
    Node *current = &lisp->node;
    Var *node_data;
    Var returned;
    const char *messages[TL_RC_AMOUNT] = {
        "Unknown error!",
        "Token full!",
        "Missing function!",
        "String outside of call!",
        "Out of memory!",
        "Mem. copy error!",
        "FStack overflow!",
        "Extra parenthesis!",
        "Argstack overflow!",
        "Internal error, please report it!",
        "Unknown type!",
        "Name not defined!",
        "Function not defined!",
        "Too few arguments!",
        "Too many arguments!",
        "Bad type!",
        "Invalid list size!",
        "Already defined name!",
        "Function definition not ended!",
        "Invalid name!",
        "Stack overflow!",
        "Division by zero!",
        "Bad input!",
        "Index out of range!",
        "Value outside of call!"
    };
    lisp->line = 1;
    for(i=0;i<lisp->sz;i++){
        c = lisp->buffer[i];
#if TL_DEBUG_CHAR
        printf("%ld%ld, %c\n", lisp->fstack_cur,
               lisp->fstack[lisp->fstack_cur].argstack_cur, c);
#endif
        if(c == '\\' && !escaped){
            escaped = 1;
            continue;
        }
        if(!escaped){
            if(c == '"' && !in_string){
                in_string = 1;
                continue;
            }
            /* Handle arguments and parantheses */
            if(!in_string){
                if(c == '(' || c == ')' || c == ' ' || c == '\t' || c == '\n'){
                    if(token_cur){
                        /* Update top call */
                        if(current == &lisp->node){
                            TL_ERROR(TL_ERR_VALUE_OUTSIDE_OF_CALL);
                        }
                        if(token_cur){
                            if(current->var->items->call.has_func){
                                /* Add a node for the value */
#if TL_DEBUG_TREE
                                fputs(" |_ Argument \"", stdout);
                                fwrite(token, 1, token_cur, stdout);
                                puts("\"");
#endif
                                allocated = malloc(sizeof(Node));
                                if(!allocated){
                                    TL_ERROR(TL_ERR_OUT_OF_MEM);
                                }
                                node_data = malloc(sizeof(Node));
                                if(!node_data){
                                    free(allocated);
                                    TL_ERROR(TL_ERR_OUT_OF_MEM);
                                }
                                rc = var_auto(node_data, token, token_cur);
                                if(rc){
                                    TL_ERROR(rc);
                                }
                                rc = node_init(allocated, node_data);
                                if(rc){
                                    free(allocated);
                                    free(node_data);
                                    TL_ERROR(rc);
                                }
                                rc = node_add_child(current, allocated);
                                if(rc){
                                    free(allocated);
                                    free(node_data);
                                    TL_ERROR(rc);
                                }
                            }else{
                                /* Set the function name. */
    #if TL_DEBUG_TREE
                                fputs(" |_ Function name \"", stdout);
                                fwrite(token, 1, token_cur, stdout);
                                puts("\"");
    #endif
                                var_free_str(&current->var->items->call.function);
                                var_raw_str(&current->var->items->call.function,
                                            token, token_cur);
                                current->var->items->call.has_func = 1;
                            }
                            token_cur = 0;
                        }
                    }
                }else{
                    TL_TOK_ADD(c)
                }
                if(c == '('){
                    /* Create new call. */
                    allocated = malloc(sizeof(Node));
                    if(!allocated){
                        TL_ERROR(TL_ERR_OUT_OF_MEM);
                    }
                    node_data = malloc(sizeof(Node));
                    if(!node_data){
                        free(allocated);
                        TL_ERROR(TL_ERR_OUT_OF_MEM);
                    }
                    rc = var_call(node_data, "", 0);
                    if(rc){
                        TL_ERROR(rc);
                    }
                    rc = node_init(allocated, node_data);
                    if(rc){
                        free(allocated);
                        free(node_data);
                        TL_ERROR(rc);
                    }
                    rc = node_add_child(current, allocated);
                    if(rc){
                        free(allocated);
                        free(node_data);
                        TL_ERROR(rc);
                    }
                    current = allocated;
                    token_cur = 0;
#if TL_DEBUG_TREE
                    puts("-> New node");
#endif
                }else if(c == ')'){
                    /* Check if the call has a function name. */
                    /* Add call to the parent call, or to the list if it has no
                     * parent */
                    if(current == &lisp->node){
                        TL_ERROR(TL_ERR_END_PARANTHESIS);
                    }
                    current = current->parent;
#if TL_DEBUG_TREE
                    if(current == &lisp->node){
                        puts("<- Go to the root node");
                    }else{
                        puts("<- Go to the parent node");
                    }
#endif
                }
            }else{
                if(c == '"'){
                    /* Check if the string is in a call. If it is, add it to
                     * the current call. */
#if TL_DEBUG_TREE
                    fputs(" |_ String argument \"", stdout);
                    fwrite(token, 1, token_cur, stdout);
                    puts("\"");
#endif
                    if(current == &lisp->node){
                        TL_ERROR(TL_ERR_STR_OUT_OF_CALL);
                    }
                    /* Add a node for the value */
                    allocated = malloc(sizeof(Node));
                    if(!allocated){
                        TL_ERROR(TL_ERR_OUT_OF_MEM);
                    }
                    node_data = malloc(sizeof(Node));
                    if(!node_data){
                        free(allocated);
                        TL_ERROR(TL_ERR_OUT_OF_MEM);
                    }
                    rc = var_str(node_data, token, token_cur);
                    if(rc){
                        TL_ERROR(rc);
                    }
                    rc = node_init(allocated, node_data);
                    if(rc){
                        free(allocated);
                        free(node_data);
                        TL_ERROR(rc);
                    }
                    rc = node_add_child(current, allocated);
                    if(rc){
                        free(allocated);
                        free(node_data);
                        TL_ERROR(rc);
                    }
                    token_cur = 0;
                    in_string = 0;
                    in_hex = 0;
                    continue;
                }
            }
        }
        if(in_string){
            if(in_hex < 3 && in_hex > 0){
                if(c >= '0' && c <= '9'){
                    hexnum += (c-'0')<<(4*(1-(in_hex-1)));
                }else if(c >= 'A' && c <= 'F'){
                    hexnum += (c-'A'+10)<<(4*(1-(in_hex-1)));
                }else if(c >= 'a' && c <= 'f'){
                    hexnum += (c-'a'+10)<<(4*(1-(in_hex-1)));
                }
                in_hex++;
                if(in_hex > 2){
                    TL_TOK_ADD(hexnum);
                    in_hex = 0;
                }
            }else if(escaped){
                /* Handle escape sequences */
                in_hex = 0;
                switch(c){
                    case '"':
                        /* FALLTHRU */
                    case '\\':
                        TL_TOK_ADD(c);
                        break;
                    case 'n':
                        TL_TOK_ADD('\n');
                        break;
                    case 'r':
                        TL_TOK_ADD('\r');
                        break;
                    case 'a':
                        TL_TOK_ADD('\a');
                        break;
                    case 'x':
                        in_hex = 1;
                        hexnum = 0;
                        break;
                    default:
                        TL_TOK_ADD('\\');
                        TL_TOK_ADD(c);
                }
            }else{
                TL_TOK_ADD(c);
            }
        }
        if(c == '\n'){
            lisp->line++;
        }
        escaped = 0;
    }
    for(i=0;i<lisp->node.childnum;i++){
        rc = call_exec(lisp, ((Node**)lisp->node.childs)[i], &returned);
        if(rc){
            TL_ERROR(rc);
        }
    }
    return TL_SUCCESS;
}

#undef TL_TOK_ADD
#undef TL_ERROR

void lisp_free_nodes(Node *node, void *_lisp) {
    TinyLisp *lisp = _lisp;
    free(node->var);
    node->var = NULL;
    if(node != &lisp->node) free(node);
}

int tl_free(TinyLisp *lisp) {
    size_t i, n;
    int out = TL_SUCCESS;
    node_free_childs(&lisp->node, lisp_free_nodes, lisp);
    for(i=0;i<lisp->var_num;i++){
        var_free(lisp->vars+i);
        var_free_str(lisp->var_names+i);
    }
    for(i=0;i<lisp->stack_cur;i++){
        if(lisp->stack[i].argnum){
            if(lisp->stack[i].args){
                for(n=0;n<lisp->stack[i].argnum;n++){
                    var_free(lisp->stack[i].args+n);
                }
                free(lisp->stack[i].args);
            }
        }
        var_free(&lisp->stack[i].params);
    }
    free(lisp->vars);
    free(lisp->var_names);
    var_free(&lisp->last);
#if TL_LEAK_CHECK
    muntrace();
#endif
    return out;
}

int tl_add_var(TinyLisp *lisp, Var *var, String *name) {
    Var *var_ptr;
    String *name_ptr;
    size_t i;
    for(i=0;i<lisp->var_num;i++){
        if(lisp->var_names[i].len != name->len) continue;
        if(!memcmp(name->data, lisp->var_names[i].data, name->len)){
            return TL_ERR_NAME_EXISTS;
        }
    }
    lisp->var_num++;
    var_ptr = realloc(lisp->vars, lisp->var_num*sizeof(Var));
    if(!var_ptr){
        lisp->var_num--;
        return TL_ERR_OUT_OF_MEM;
    }
    lisp->vars = var_ptr;
    name_ptr = realloc(lisp->var_names, lisp->var_num*sizeof(String));
    if(!name_ptr){
        lisp->var_num--;
        return TL_ERR_OUT_OF_MEM;
    }
    lisp->var_names = name_ptr;
    lisp->vars[lisp->var_num-1] = *var;
    lisp->var_names[lisp->var_num-1] = *name;
    return TL_SUCCESS;
}

int tl_set_var(TinyLisp *lisp, Var *var, String *name) {
    size_t i;
    char found = 0;
    int rc;
    if(lisp->stack_cur){
        for(i=0;i<lisp->stack[lisp->stack_cur-1].params.size;i++){
            if(name->len !=
               lisp->stack[lisp->stack_cur-1].params.items[i].string.len){
                continue;
            }
            if(!memcmp(lisp->stack[lisp->stack_cur-1]
                       .params.items[i].string.data, name->data, name->len)){
                /* Set the variable */
                if(var->type != lisp->stack[lisp->stack_cur-1].args[i].type){
                    return TL_ERR_BAD_TYPE;
                }
                rc = var_free(lisp->stack[lisp->stack_cur-1].args+i);
                if(rc) return rc;
                rc = var_copy(var, lisp->stack[lisp->stack_cur-1].args+i);
                if(rc) return rc;
                found = 1;
                break;
            }
        }
    }
    if(!found){
        for(i=0;i<lisp->var_num;i++){
            if(lisp->var_names[i].len != name->len) continue;
            if(!memcmp(name->data, lisp->var_names[i].data, name->len)){
                /* Set the variable */
                if(var->type != lisp->vars[i].type){
                    return TL_ERR_BAD_TYPE;
                }
                rc = var_free(lisp->vars+i);
                if(rc) return rc;
                rc = var_copy(var, lisp->vars+i);
                if(rc) return rc;
                found = 1;
                break;
            }
        }
    }
    if(!found){
        return TL_ERR_NOT_DEF;
    }
    return TL_SUCCESS;
}

int tl_del_var(TinyLisp *lisp, String *name) {
    size_t i;
    char found = 0;
    int rc;
    for(i=0;i<lisp->var_num;i++){
        if(lisp->var_names[i].len != name->len) continue;
        if(!memcmp(name->data, lisp->var_names[i].data, name->len)){
            /* Delete the variable */
            rc = var_free(lisp->vars+i);
            if(rc) return rc;
            rc = var_free_str(lisp->var_names+i);
            if(rc) return rc;
            if(i < lisp->var_num-1){
                memmove(lisp->vars+i, lisp->vars+i+1,
                        sizeof(Var)*lisp->var_num-i-1);
                memmove(lisp->var_names+i, lisp->var_names+i+1,
                        sizeof(String)*lisp->var_num-i-1);
            }
            lisp->var_num--;
            found = 1;
            break;
        }
    }
    if(!found){
        return TL_ERR_NOT_DEF;
    }
    return TL_SUCCESS;
}

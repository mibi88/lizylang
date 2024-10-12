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
 *             user defined function. Fix leak.
 */

#include <lisp.h>
#include <platform.h>
#include <call.h>
#include <builtin.h>

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

int tl_run(TinyLisp *lisp, void error(char*, void*), void *data) {
    size_t n;
    char c;
    char token[TL_TOKEN_SZ];
    size_t token_cur = 0;
    char in_string = 0;
    char escaped = 0;
    char in_hex = 0;
    char hexnum;
    int rc;
    Var *argstart;
    Var returned;
    size_t *fncinfo;
    size_t argnum;
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
        "Bad input!"
    };
    lisp->line = 1;
    for(lisp->i=0;lisp->i<lisp->sz;lisp->i++){
        c = lisp->buffer[lisp->i];
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
                        /* Push arg or function name to stack */
#if TL_DEBUG_TOKENS
                        fwrite(token, 1, token_cur, stdout);
                        fputc('\n', stdout);
#endif
                        if(lisp->fstack[lisp->fstack_cur].has_func){
                            /* It is an argument */
                            var_auto(lisp->argstack+lisp->argstack_cur, token,
                                    token_cur);
                            lisp->argstack_fnc[lisp->argstack_cur] =
                                                            lisp->fstack_cur;
                            lisp->argstack_cur++;
                            if(lisp->argstack_cur >= TL_ARGSTACK_SZ){
                                var_free_call(lisp->fstack+lisp->fstack_cur);
                                TL_ERROR(TL_ERR_ARGSTACK_OVERFLOW);
                            }
                        }else{
                            /* It is a function name */
                            if((rc = var_call(lisp->fstack+lisp->fstack_cur,
                                              token, token_cur))){
                                TL_ERROR(rc);
                            }
                            lisp->fstack[lisp->fstack_cur].has_func = 1;
                        }
                        token_cur = 0;
                    }
                }else{
                    TL_TOK_ADD(c)
                }
                if(c == '('){
                    lisp->fstack_cur++;
                    if(lisp->fstack_cur >= TL_FSTACK_SZ){
                        var_free_call(lisp->fstack+lisp->fstack_cur);
                        TL_ERROR(TL_ERR_FSTACK_OVERFLOW);
                    }
                    lisp->fstack[lisp->fstack_cur].function.data = NULL;
                    lisp->fstack[lisp->fstack_cur].function.len = 0;
                    lisp->fstack[lisp->fstack_cur].has_func = 0;
                    lisp->fstack[lisp->fstack_cur].argstack_cur =
                                                            lisp->argstack_cur;
                    token_cur = 0;
                }else if(c == ')'){
                    if(!lisp->fstack[lisp->fstack_cur].has_func){
                        TL_ERROR(TL_ERR_NOFUNC);
                    }
                    /* Run the function */
                    if(lisp->argstack_cur <
                       lisp->fstack[lisp->fstack_cur].argstack_cur){
                        TL_ERROR(TL_ERR_INTERNAL);
                    }
                    argnum = lisp->argstack_cur-
                                lisp->fstack[lisp->fstack_cur].argstack_cur;
                    argstart = lisp->argstack+lisp->argstack_cur-argnum;
                    fncinfo = lisp->argstack_fnc+lisp->argstack_cur-argnum;
#if TL_DEBUG_FSTACK
                    printf("fstack (%ld): ", lisp->fstack_cur);
                    for(n=1;n<lisp->fstack_cur+1;n++){
                        fputc('\"', stdout);
                        fwrite(lisp->fstack[n].function.data, 1,
                               lisp->fstack[n].function.len, stdout);
                        fputs("\" ", stdout);
                    }
                    fputc('\n', stdout);
#endif
#if TL_DEBUG_ARGSTACK
                    printf("argstack (%ld-%ld): ", lisp->argstack_cur, argnum);
                    for(n=0;n<lisp->argstack_cur;n++){
                        if(lisp->argstack[n].items){
                            if(lisp->argstack[n].type == TL_T_NUM){
                                printf("%f ", lisp->argstack[n].items->num);
                            }else{
                                fputc('\"', stdout);
                                fwrite(lisp->argstack[n].items[0].string.data,
                                       1,
                                       lisp->argstack[n].items[0].string.len,
                                       stdout);
                                fputs("\" ", stdout);
                            }
                        }else{
                            fputs("(null) ", stdout);
                        }
                    }
                    fputc('\n', stdout);
#endif
                    for(n=0;n<argnum;n++){
                        if(fncinfo[n] != lisp->fstack_cur){
                            TL_ERROR(TL_ERR_INTERNAL);
                        }
                    }
                    if((rc = call_exec(lisp, lisp->fstack+lisp->fstack_cur,
                                       argstart, argnum, &returned))){
                        var_free_call(lisp->fstack+lisp->fstack_cur);
                        TL_ERROR(rc);
                    }
                    for(n=0;n<argnum;n++){
                        if((rc =
                            var_free(lisp->argstack+lisp->argstack_cur-n-1))){
                            TL_ERROR(rc);
                        }
                    }
                    lisp->argstack_cur -= argnum;
                    /* Push return value on argstack */
                    if(lisp->fstack_cur > 1){
                        lisp->argstack[lisp->argstack_cur] = returned;
                        lisp->argstack_fnc[lisp->argstack_cur] =
                                                            lisp->fstack_cur-1;
                        lisp->argstack_cur++;
                        if(lisp->argstack_cur >= TL_ARGSTACK_SZ){
                            var_free_call(lisp->fstack+lisp->fstack_cur);
                            TL_ERROR(TL_ERR_ARGSTACK_OVERFLOW);
                        }
                    }else{
                        if((rc = var_free(&returned))){
                            var_free_call(lisp->fstack+lisp->fstack_cur);
                            TL_ERROR(rc);
                        }
                    }
                    /* Free the call data */
                    lisp->fstack[lisp->fstack_cur].has_func = 0;
                    if(lisp->fstack_cur <= 0){
                        TL_ERROR(TL_ERR_END_PARANTHESIS);
                    }
                    var_free_call(lisp->fstack+lisp->fstack_cur);
                    lisp->fstack_cur--;
                }
            }else{
                if(c == '"'){
                    if(lisp->fstack_cur > 0){
                        if(lisp->fstack[lisp->fstack_cur].has_func){
                            /* Push arg to stack */
#if TL_DEBUG_TOKENS
                            fwrite(token, 1, token_cur, stdout);
                            fputc('\n', stdout);
#endif
                            var_str(lisp->argstack+lisp->argstack_cur, token,
                                    token_cur);
                            lisp->argstack_fnc[lisp->argstack_cur] =
                                                            lisp->fstack_cur;
                            lisp->argstack_cur++;
                            if(lisp->argstack_cur >= TL_ARGSTACK_SZ){
                                var_free_call(lisp->fstack+lisp->fstack_cur);
                                TL_ERROR(TL_ERR_ARGSTACK_OVERFLOW);
                            }
                        }else{
                            TL_ERROR(TL_ERR_NOFUNC);
                        }
                    }else{
                        TL_ERROR(TL_ERR_STR_OUT_OF_CALL);
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
    if(!lisp->perform_calls){
        TL_ERROR(TL_ERR_FNCDEF_NO_END);
    }
    return TL_SUCCESS;
}

#undef TL_TOK_ADD
#undef TL_ERROR

int tl_free(TinyLisp *lisp) {
    size_t i, n;
    int rc;
    int out = TL_SUCCESS;
    for(i=0;i<lisp->var_num;i++){
        var_free(lisp->vars+i);
        var_free_str(lisp->var_names+i);
    }
    for(i=0;i<lisp->argstack_cur;i++){
        if((rc = var_free(lisp->argstack+i))){
            out = rc;
        }
    }
    for(i=0;i<lisp->fstack_cur;i++){
        if((rc = var_free_call(lisp->fstack+i))){
            out = rc;
        }
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

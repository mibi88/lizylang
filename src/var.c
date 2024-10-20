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
 * 2024/10/03: Fixed memory leaks.
 * 2024/10/04: Adding types.
 * 2024/10/07: Added return value for functions, string concatenation, creation
 *             of numbers from float and adding data to a string.
 * 2024/10/08: Append to list.
 * 2024/10/15: Store the user function node. Handle calls in var_copy.
 *             var_call: initialize a Var.
 * 2024/10/18: Fixed builtin function prototype.
 * 2024/10/20: Better name.
 */

#include <var.h>

int var_auto(Var *var, char *data, size_t len) {
    if(var_isnum(data, len)){
        var_num(var, data, len);
        return TL_SUCCESS;
    }else if(var_isname(data, len)){
        var_str(var, data, len);
        var->type = TL_T_NAME;
        return TL_SUCCESS;
    }
    return TL_ERR_UNKNOWN_TYPE;
}

int var_str(Var *var, char *data, size_t len) {
    var->type = TL_T_STR;
    var->items = malloc(sizeof(Item));
    if(!var->items){
        return TL_ERR_OUT_OF_MEM;
    }
    var->size = 1;
    var->items->string.data = malloc(len);
    if(!var->items->string.data){
        return TL_ERR_OUT_OF_MEM;
    }
    var->items->string.len = len;
    if(!memcpy(var->items->string.data, data, len)){
        return TL_ERR_CPY;
    }
    var->null = 0;
    return TL_SUCCESS;
}

int var_str_concat(Var *var, Var *str1, Var *str2) {
    var->type = TL_T_STR;
    var->items = malloc(sizeof(Item));
    if(!var->items){
        return TL_ERR_OUT_OF_MEM;
    }
    var->size = 1;
    var->items->string.len = str1->items->string.len+str2->items->string.len;
    var->items->string.data = malloc(var->items->string.len);
    if(!var->items->string.data){
        return TL_ERR_OUT_OF_MEM;
    }
    if(!memcpy(var->items->string.data, str1->items->string.data,
               str1->items->string.len)){
        return TL_ERR_CPY;
    }
    if(!memcpy(var->items->string.data+str1->items->string.len,
               str2->items->string.data, str2->items->string.len)){
        return TL_ERR_CPY;
    }
    var->null = 0;
    return TL_SUCCESS;
}

int var_str_add(Var *var, char *data, size_t len) {
    char *tmp;
    if(var->type != TL_T_STR) return TL_ERR_BAD_TYPE;
    tmp = realloc(var->items->string.data, var->items->string.len+len);
    if(!tmp) return TL_ERR_OUT_OF_MEM;
    var->items->string.data = tmp;
    if(!memcpy(var->items->string.data+var->items->string.len, data, len)){
        return TL_ERR_CPY;
    }
    var->items->string.len += len;
    return TL_SUCCESS;
}

int var_raw_str(String *string, char *data, size_t len) {
    string->data = malloc(len);
    if(!string->data){
        return TL_ERR_OUT_OF_MEM;
    }
    string->len = len;
    if(!memcpy(string->data, data, len)){
        return TL_ERR_CPY;
    }
    return TL_SUCCESS;
}

int var_builtin_func(Var *var, int f(void*, void*, size_t, void*),
                     char parse) {
    var->type = TL_T_FUNC;
    var->items = malloc(sizeof(Item));
    if(!var->items){
        return TL_ERR_OUT_OF_MEM;
    }
    var->size = 1;
    var->null = 0;
    var->items->function.ptr.f = f;
    var->items->function.builtin = 1;
    var->items->function.parseargs = parse;
    return TL_SUCCESS;
}

int var_user_func(Var *var, void *fncdef, Var *params) {
    int rc;
    var->type = TL_T_FUNC;
    var->items = malloc(sizeof(Item));
    if(!var->items){
        return TL_ERR_OUT_OF_MEM;
    }
    var->size = 1;
    var->null = 0;
    var->items->function.ptr.fncdef = fncdef;
    var->items->function.builtin = 0;
    var->items->function.parseargs = 1;
    var->items->function.params = malloc(sizeof(Var));
    if(!var->items->function.params){
        free(var->items);
        return TL_ERR_OUT_OF_MEM;
    }
    rc = var_copy(params, var->items->function.params);
    if(rc) return rc;
    return TL_SUCCESS;
}

char var_isnum(char *data, size_t len) {
    char hasdot = 0;
    char c;
    size_t i;
    if(len < 1){
        return 0;
    }
    if(data[0] == '-'){
        data++;
        len--;
    }
    for(i=0;i<len;i++){
        c = data[i];
        if((c < '0' || c > '9') && c != '.') return 0;
        if(c == '.'){
            if(hasdot) return 0;
            hasdot = 1;
        }
    }
    return 1;
}

int var_num(Var *var, char *data, size_t len) {
    /* TODO: Add support for exponent */
    float sign = 1;
    float d = 0.1;
    float out = 0;
    char c;
    size_t i;
    var->type = TL_T_NUM;
    var->items = malloc(sizeof(Item));
    if(!var->items){
        return TL_ERR_OUT_OF_MEM;
    }
    var->size = 1;
    if(data[0] == '-'){
        sign = -1;
        data++;
        len--;
    }
    for(i=0;i<len;i++){
        c = data[i];
        if(c == '.') break;
        d *= 10;
    }
    for(i=0;i<len;i++){
        c = data[i];
        if(c != '.'){
            out += (c-'0')*d;
            d /= 10;
        }
    }
    var->items->num = out*sign;
    var->null = 0;
    return TL_SUCCESS;
}

int var_num_from_float(Var *var, float num) {
    var->type = TL_T_NUM;
    var->items = malloc(sizeof(Item));
    if(!var->items){
        return TL_ERR_OUT_OF_MEM;
    }
    var->size = 1;
    var->items->num = num;
    var->null = 0;
    return TL_SUCCESS;
}

char var_isname(char *data, size_t len) {
    char c;
    if(len < 1) return 0;
    c = data[0];
    if(c >= '0' && c <= '9') return 0;
    return 1;
}

int var_copy(Var *src, Var *dest) {
    size_t i;
    if(!src->size || !src->items){
        dest->size = 0;
        dest->items = NULL;
        return TL_SUCCESS;
    }
    switch(src->type){
        case TL_T_NAME:
            dest->type = TL_T_NAME;
            dest->items = malloc(src->size*sizeof(Item));
            if(!dest->items){
                return TL_ERR_OUT_OF_MEM;
            }
            dest->size = src->size;
            for(i=0;i<src->size;i++){
                dest->items[i].string.data = malloc(src->items[i].string.len);
                if(!dest->items[i].string.data){
                    return TL_ERR_OUT_OF_MEM;
                }
                dest->items[i].string.len = src->items[i].string.len;
                if(!memcpy(dest->items[i].string.data,
                           src->items[i].string.data,
                           src->items[i].string.len)){
                    return TL_ERR_CPY;
                }
            }
            dest->null = 0;
            break;
        case TL_T_STR:
            dest->type = TL_T_STR;
            dest->items = malloc(src->size*sizeof(Item));
            if(!dest->items){
                return TL_ERR_OUT_OF_MEM;
            }
            dest->size = src->size;
            for(i=0;i<src->size;i++){
                dest->items[i].string.data = malloc(src->items[i].string.len);
                if(!dest->items[i].string.data){
                    return TL_ERR_OUT_OF_MEM;
                }
                dest->items[i].string.len = src->items[i].string.len;
                if(!memcpy(dest->items[i].string.data,
                           src->items[i].string.data,
                           src->items[i].string.len)){
                    return TL_ERR_CPY;
                }
            }
            dest->null = 0;
            break;
        case TL_T_NUM:
            dest->type = TL_T_NUM;
            dest->items = malloc(src->size*sizeof(Item));
            if(!dest->items){
                return TL_ERR_OUT_OF_MEM;
            }
            dest->size = src->size;
            dest->null = 0;
            for(i=0;i<src->size;i++){
                dest->items[i].num = src->items[i].num;
            }
            break;
        case TL_T_FUNC:
            dest->type = TL_T_FUNC;
            dest->items = malloc(src->size*sizeof(Item));
            if(!dest->items){
                return TL_ERR_OUT_OF_MEM;
            }
            dest->size = src->size;
            dest->null = 0;
            for(i=0;i<src->size;i++){
                dest->items[i].function = src->items[i].function;
            }
            break;
        case TL_T_CALL:
            dest->type = TL_T_CALL;
            dest->items = malloc(src->size*sizeof(Item));
            if(!dest->items){
                return TL_ERR_OUT_OF_MEM;
            }
            dest->size = src->size;
            dest->null = 0;
            for(i=0;i<src->size;i++){
                dest->items[i].call = src->items[i].call;
            }
            break;
        default:
            return TL_ERR_BAD_TYPE;
    }
    return TL_SUCCESS;
}

int var_call(Var *var, char *name, size_t len) {
    var->type = TL_T_CALL;
    var->items = malloc(sizeof(Item));
    if(!var->items){
        return TL_ERR_OUT_OF_MEM;
    }
    var->size = 1;
    var->null = 0;
    var->items->call.function.data = malloc(len);
    if(!var->items->call.function.data){
        return TL_ERR_OUT_OF_MEM;
    }
    var->items->call.function.len = len;
    if(!memcpy(var->items->call.function.data, name, len)){
        return TL_ERR_CPY;
    }
    var->items->call.has_func = 0;
    return TL_SUCCESS;
}

int var_free_str(String *string) {
    free(string->data);
    string->data = NULL;
    return TL_SUCCESS;
}

int var_free(Var *var) {
    size_t i;
    if(!var->items || !var->size) return TL_SUCCESS;
    switch(var->type){
        case TL_T_NAME:
            /* FALLTHRU */
        case TL_T_STR:
            for(i=0;i<var->size;i++){
                free(var->items[i].string.data);
                var->items[i].string.data = NULL;
            }
            break;
        case TL_T_NUM:
            break;
        case TL_T_FUNC:
            for(i=0;i<var->size;i++){
                if(!var->items[i].function.builtin){
                    var_free(var->items[i].function.params);
                    free(var->items[i].function.params);
                    var->items[i].function.params = NULL;
                }
            }
            break;
        case TL_T_CALL:
            free(var->items->call.function.data);
            var->items->call.function.data = NULL;
            var->items->call.function.len = 0;
            break;
        default:
            return TL_ERR_UNKNOWN_TYPE;
    }
    free(var->items);
    var->items = NULL;
    var->size = 0;
    return TL_SUCCESS;
}

int var_append(Var *src, Var *dest) {
    Item *tmp;
    Var src_copy;
    int rc;
    rc = var_copy(src, &src_copy);
    if(rc) return rc;
    if(src->type != dest->type) return TL_ERR_BAD_TYPE;
    tmp = realloc(dest->items, (dest->size+src->size)*sizeof(Item));
    if(!tmp) return TL_ERR_OUT_OF_MEM;
    dest->items = tmp;
    if(!memcpy(dest->items+dest->size, src_copy.items,
               src->size*sizeof(Item))){
        return TL_ERR_CPY;
    }
    dest->size += src->size;
    free(src_copy.items);
    return TL_SUCCESS;
}

/* TODO: Head and tail. */

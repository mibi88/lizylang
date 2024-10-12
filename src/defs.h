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
 * 2024/09/30: Working lexer and parser.
 * 2024/10/03: Added error codes, check for memory leaks.
 * 2024/10/04: Adding function calling.
 * 2024/10/08: Started adding list support.
 * 2024/10/09: Error when redefining variables and if the function definition
 *             has no end. Added void list support.
 */

#ifndef DEFS_H
#define DEFS_H

#include <platform.h>

#define TL_UNUSED(var) ((void)(var))
#define TL_TOKEN_SZ    512
#define TL_STACK_SZ    256
#define TL_FSTACK_SZ   128
#define TL_ARGSTACK_SZ 128

enum {
    TL_SUCCESS,
    TL_ERR_TOKFULL,
    TL_ERR_NOFUNC,
    TL_ERR_STR_OUT_OF_CALL,
    TL_ERR_OUT_OF_MEM,
    TL_ERR_CPY,
    TL_ERR_FSTACK_OVERFLOW,
    TL_ERR_END_PARANTHESIS,
    TL_ERR_ARGSTACK_OVERFLOW,
    TL_ERR_INTERNAL,
    TL_ERR_UNKNOWN_TYPE,
    TL_ERR_NOT_DEF,
    TL_ERR_FUNC_NOT_DEF,
    TL_ERR_TOO_FEW_ARGS,
    TL_ERR_TOO_MANY_ARGS,
    TL_ERR_BAD_TYPE,
    TL_ERR_INVALID_LIST_SIZE,
    TL_ERR_NAME_EXISTS,
    TL_ERR_FNCDEF_NO_END,
    TL_ERR_INVALID_NAME,
    TL_ERR_STACK_OVERFLOW,
    TL_ERR_DIVISION_BY_ZERO,
    TL_ERR_BAD_INPUT,
    TL_RC_AMOUNT
};

#endif

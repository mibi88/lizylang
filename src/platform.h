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
 * 2024/09/28: Started developement. Added required includes.
 * 2024/09/30: Added debug defines.
 * 2024/10/03: Check for memory leaks.
 * 2024/10/04: Debug function searching.
 * 2024/10/15: Debug the tree generation.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <mcheck.h>

/*
 * This header should provide:
 * void *malloc(size_t size);
 * void *realloc(void *ptr, size_t new_size);
 * void free(void *ptr);
 * void *memcpy(void *dest, void *src, size_t size);
 */

#define TL_DEBUG_CHAR     0
#define TL_DEBUG_ARGSTACK 0
#define TL_DEBUG_FSTACK   0
#define TL_DEBUG_TOKENS   0
#define TL_DEBUG_CALL     0
#define TL_DEBUG_VARS     0
#define TL_DEBUG_TREE     0
#define TL_LEAK_CHECK     1

#endif

                                LIZYLANG
                                by Mibi88

A lisp dialect with lazy evaluation.

    THE NAME

Some time ago, I watched a video by Tantan (https://youtu.be/Qg8OGAfiG7M),
where he ranked programming language names. I wanted to give my programming
language a cool name, and after some thinking I came up with this name by
trying to combine "lisp" and "lazy".

It is composed of "Li" for lisp, "zy" for lazy and "lang" because it is a
programming language.

It should be pronounced [lizjˈleɪŋɡ]

The file extension is .lzy

    THE INTERPRETER

Currently it is in a very early state, so it is quite inefficient and it may
have some bugs.

It is released under the BSD-3-Clause license.

Here is a TODO list of what will come next:

    TODO

[x] Set and delete variables (currently they can only be defined).
[ ] List management with head and tail.
[ ] Optimize tail recursion.
[ ] Variable amount of arguments passed to user defined functions.
[ ] Integer type.
[ ] User friendly way to define builtin functions.
[ ] Define to use fixed point math instead of floating point arithmetic (for
    higher performance on CPU without FPUs).
[ ] File importing.
[ ] Pattern matching?
[ ] Scopes?
[ ] Foreign function interface.
[ ] Generate bytecode?

    KNOWN BUGS

[ ] Memory leak when name not defined.
[ ] Multiple values returned from user function.

    CHANGELOG

2024/10/12: Created this file.

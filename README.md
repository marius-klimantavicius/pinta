This is a ECMAScript like language parser/compiler and a corresponding virtual machine/execution engine.

The execution engine was created for embedded devices with the following requirements:

1. Must not allocate dynamic memory - VM only uses the provided memory region and some static memory. `malloc` and similar functions are not permitted in the core engine.
1. The executed scripts must be memory safe - no out of bounds errors, cannot read native memory.
1. Must not use `setjmp`/`longjmp` for anything (including error/exception handling).
1. Must support decimal based real numbers (implemented as fixed point numbers).
1. Must have minimal support for string based operations (`substring`, concatenation, indexing, comparisons).
1. Must have minimal support for binary data.
1. Output of a script is either binary blob or text string.
1. Script is compiled to a binary representation to limit size.
1. It should be cross platform (it is being used in ARM devices).

Currently it is used by incorporating core files directly into the project (both for execution engine and parser/compiler).

Code from the following projects were used:

- ECMAScript parser from https://github.com/sebastienros/jint (BSD 2-Clause License)
- (Optional for debugger) Web server from https://github.com/deplinenoise/webby (BSD 2-Clause License)
- (Optional for debugger) TCL interpreter from https://github.com/zserge/partcl (MIT License)
- (No longer used) Gppg and gplex from https://archive.codeplex.com/?p=gppg (New BSD License)
- (No longer used) Jay from https://github.com/mono/mono/tree/master/mcs/jay (BSD 4-Clause License)

Debugger is not usable (partly due to bugs in webby, partly due to lack of features and time).
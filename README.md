\# Assembler Project (ANSI C)



\## Overview

This project implements a \*\*two-pass assembler\*\* written in \*\*ANSI C\*\*.  

It translates a simplified assembly language into machine code, while handling macros, symbol tables, and memory images.



This repository contains the \*\*first version\*\* of the project:  

`000\_Project\_pre\_assembler\_and\_first\_pass`,  

which includes the \*\*pre-assembler\*\* and \*\*first pass\*\* modules.



\## Project Structure

assembler/

├── error\_list.c / error\_list.h

├── errors.c / errors.h

├── first\_pass.c / first\_pass.h

├── instruction\_set.h

├── macro\_table.c / macro\_table.h

├── memory\_image.c / memory\_image.h

├── pre\_assembler.c / pre\_assembler.h

├── symbol\_table.c / symbol\_table.h

├── utils.c / utils.h

├── main.c

├── makefile

├── sample.as / test.as / test.am

└── test\_first\_pass.c



\## Compilation

You can build the assembler using:

```bash

make

This produces an executable called assembler.



\## Usage



Run the assembler by providing an assembly source file (without the .as extension):

./assembler test



It will generate:

.am — after macro expansion

.ob — object file (machine code)

.ent — entries

.ext — externals



\## Features

Full macro preprocessing

Symbol and label table construction

First-pass error detection

Instruction parsing and addressing modes

Modular design (ANSI C compliant)

Makefile-based compilation



\## Author



Developed by Yoavbh10(C) 2025



\## MIT License



Copyright (c) 2025 Yoavbh10



Permission is hereby granted, free of charge, to any person obtaining a copy

of this software and associated documentation files (the "Software"), to deal

in the Software without restriction, including without limitation the rights

to use, copy, modify, merge, publish, distribute, sublicense, and/or sell

copies of the Software, and to permit persons to whom the Software is

furnished to do so, subject to the following conditions:



The above copyright notice and this permission notice shall be included in all

copies or substantial portions of the Software.



THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR

IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,

FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE

AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER

LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,

OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE

SOFTWARE.



README – Assembler Project (MAMAN 14)

Compilation
Use the provided makefile:
make clean
make assembler
This produces the assembler executable.

Running
Run the assembler on .as source files:
./assembler file.as
If the file is valid, the assembler outputs:
file.ob – machine code (base-4 encoding).
file.ent – entry symbols.
file.ext – external symbols.
If errors are found, no output files are created.

Test Files
ps.as – basic example with code, data, and string.
example_valid_1.as – multiple .data/.string, instruction coverage.
example_valid_2.as – branching, matrix addressing, .entry/.extern.
example_valid_3.as – broad sweep across instructions and directives.
example_invalid_1.as – too many operands, illegal register.
example_invalid_2.as – duplicate labels, .entry/.extern conflict.

Code Structure
main.c – entry point.
pre_assembler.c – macro expansion.
first_pass.c – builds symbol table, encodes preliminaries.
second_pass.c – resolves symbols, finalizes code, writes outputs.
symbol_table.c / memory_image.c – abstractions for labels and memory.
instruction_encoder.c – encodes machine instructions.
error_list.c – collects and reports all errors.
output_files.c – generates .ob, .ent, .ext.

Assumptions & Limitations
Line length limited to 80 chars.
No nested macros; macros must end with mcroend.
Only standard C libraries used.
Linker/loader stages are not implemented (scope limited to assembler).



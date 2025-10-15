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


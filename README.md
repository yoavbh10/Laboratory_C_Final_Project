## 🧠 Laboratory C Final Project — Two-Pass Assembler (ANSI C)

---

### 📘 Overview

This project is a **two-pass assembler** implemented in **strict ANSI C** as part of a university laboratory course.
It translates a custom assembly language into machine code, producing several output files (`.am`, `.ob`, `.ent`, `.ext`).
The program is modular, efficient, and well-documented, following the C89 standard.

---

### ⚙️ Features

✅ **Pre-Assembler phase**

* Expands user-defined macros and outputs an `.am` file
* Handles nested and repeated macro definitions gracefully

✅ **First Pass**

* Parses `.am` files to build symbol and instruction tables
* Detects syntax and semantic errors
* Calculates memory addresses for symbols and data

✅ **Second Pass**

* Resolves symbol addresses and generates machine code
* Creates output files:

  * `.ob` — object (machine code)
  * `.ent` — entries
  * `.ext` — externals

✅ **Error Handling**

* Comprehensive error detection across all stages
* Each module reports clear, descriptive messages

✅ **Modular Design**

* Code divided across multiple C source and header files
* Clean separation between logic, memory management, and error reporting

---

### 📁 Project Structure

```
Laboratory_C_Final_Project/
├── src/
│   ├── main.c
│   ├── pre_assembler.c
│   ├── first_pass.c
│   ├── memory_image.c
│   ├── symbol_table.c
│   ├── macro_table.c
│   ├── error_list.c
│   ├── errors.c
│   ├── utils.c
│   └── ...
├── include/
│   ├── pre_assembler.h
│   ├── first_pass.h
│   ├── memory_image.h
│   ├── symbol_table.h
│   ├── macro_table.h
│   ├── error_list.h
│   ├── errors.h
│   ├── instruction_set.h
│   └── utils.h
├── tests/
│   ├── sample.as
│   ├── test.as
│   ├── test.am
│   └── test_first_pass.c
├── makefile
├── README.md
└── LICENSE
```

---

### 🧩 Compilation & Execution

#### 🧱 Build the assembler

```bash
make
```

This compiles all source files and creates an executable named:

```
assembler
```

#### ▶️ Run the assembler

```bash
./assembler filename.as
```

The assembler will automatically generate:

* `filename.am` (after macro expansion)
* `filename.ob` (object code)
* `filename.ent` (entries)
* `filename.ext` (externals)

---

### 🧪 Example Usage

```bash
./assembler sample.as
```

**Output:**

```
Assembling file: sample.as
Pre-assembly completed → sample.am
First pass completed successfully
Second pass completed successfully
Output files generated:
  sample.ob
  sample.ent
  sample.ext
```

---

### 💡 Implementation Notes

* Written in **strict ANSI C (C89)** — portable across Unix and Windows.
* Robust **error management** and **memory handling**.
* All logic tested using custom `.as` files.
* Designed with maintainability and modularity in mind.

---

### 🧑‍💻 Author

**Yoavbh10**
Developed as part of the **Laboratory in C** final course project.

---

### 📜 License

This project is released under the **MIT License**.
See the [`LICENSE`](LICENSE) file for more details.
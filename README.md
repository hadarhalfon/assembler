# 🧩 Assembler Project (C89)

A complete **two‑pass assembler** implemented in **ANSI C89**, designed to translate assembly‑like source files into a **custom base‑4 encoded machine code** (letters `a`–`d`). The project demonstrates core concepts of assembler design — **macro expansion**, **symbol resolution**, **instruction encoding**, and **file generation** — all built from scratch without external libraries.

---

## 🚀 Features

- **Two‑pass architecture**
  - **First pass:** builds the symbol table, processes directives, and computes instruction/data counters.
  - **Second pass:** resolves symbol references and generates the final output files.

- **Macro preprocessor**
  - Expands macros to `.am` prior to assembly.
  - Supports repeated/nested macro calls safely.

- **Custom encoding (NOT binary)**
  - Output is **not binary**/hex — it is a **base‑4 textual code** that uses the letters `a`, `b`, `c`, `d` to represent digits.
  - The object file (`.ob`) lists **addresses in base‑4** and **machine words in base‑4** (see format below).

- **Directives & opcodes**
  - Directives: `.data`, `.string`, `.mat`, `.entry`, `.extern`.
  - Representative opcodes: `mov`, `cmp`, `add`, `sub`, `lea`, `clr`, `not`, `inc`, `dec`, `jmp`, `bne`, `jsr`, `red`, `prn`, `rts`, `stop`.

- **Linked‑list data structures**
  - Dynamic **symbol table**, **instruction (order) list**, and **data list** for flexible memory usage.

- **Outputs**
  - `*.am` — expanded source (after macro pass)
  - `*.ob` — **base‑4 encoded** object
  - `*.ent` — entry symbols
  - `*.ext` — external references

---

## 📁 Project Structure

```
Assembler_Project/
│
├── main.c                     # Program entry point
├── preprocessor.c/h           # Macro expansion
├── first_scan.c/h             # Pass 1: symbols, IC/DC
├── second_scan.c/h            # Pass 2: resolution, code emission
├── symbolTable.c/h            # Symbol table management
├── order.c/h                  # Instruction word management
├── decode.c/h                 # Base‑4 encoding helpers
├── word.c/h                   # Word representation helpers
├── helpers.c/h                # Parsing & validation utilities
├── output.c/h                 # Emit .ob/.ent/.ext
├── constants.h                # Global constants
└── Makefile                   # Build script (gcc -std=c89 -Wall -pedantic)
```

---

## 🧠 How It Works

1. **Macro Expansion** → inputs `*.as` → outputs `*.am`.
2. **First Pass** → parses lines, handles directives, updates **IC/DC**, builds symbol/data/instruction lists.
3. **Second Pass** → resolves labels/externs/entries, encodes each word in **base‑4**, and writes `*.ob`, `*.ent`, `*.ext`.

---

## 📦 Output Formats (important)

### `.ob` — base‑4 object file
- Two columns: **base‑4 address** and **base‑4 machine code word**.
- Both columns are text using **letters `a`–`d`** (no decimal/hex digits).
- Example snippet (illustrative):

```
Base 4 address   Base 4 code
bcba              bccb
bcbb              bcdc
bcbc              bdaa
bcbd              bdab
...
```

> Notes:
> - Addresses are the instruction/data addresses rendered in **base‑4** letters.
> - Each encoded word is emitted in the custom base‑4 alphabet (`a`–`d`).

### `.ent` — entries
A text list of **entry symbols** and their **addresses** (address formatting consistent with the project spec).

### `.ext` — externals
A text list of **external symbol usages** and the **addresses** where they are referenced.

---

## ⚙️ Build & Run

```bash
# Build
make
# or
gcc -std=c89 -Wall -pedantic *.c -o assembler

# Assemble one or more sources (without the .as suffix)
./assembler prog1 prog2
```
Expected inputs:
```
prog1.as   prog2.as
```
Generated outputs (per input base name):
```
prog1.am   # after macro expansion
prog1.ob   # object in custom base‑4
prog1.ent  # entries (if any)
prog1.ext  # externals (if any)
```

---

## 🧩 Minimal Example

### Input (`example.as`)
```asm
.entry LOOP
.extern EXT

MAIN:  mov  ARR[r2][r7], EXT
       add  r2, STR
LOOP:  jmp  EXT
       prn  #-5
       sub  r1, r4
       inc  K
       stop
STR:   .string "abcdef"
LENGTH:.data 6, -9, 15
K:     .data 22
ARR:   .mat [2][2] 1,2,3,4
```

### Output (`example.ob`, schematic)
```
Base 4 address   Base 4 code
bcba              bccb
bcbb              bcdc
bcbc              bdaa
bcbd              bdab
...
```

`.ent` and `.ext` will include the relevant symbols with their addresses.

---

## 🧾 Error Handling

The assembler validates:
- Illegal labels / syntax issues / undefined symbols
- Invalid addressing modes per opcode
- Duplicate symbol definitions
- Malformed directives or overflows

All errors are reported with **line numbers** and a clear message.

---

## 📘 Implementation Highlights

- **ANSI C89**, portable and warning‑clean with `-Wall -pedantic`.
- Strict **modular design**: clear separation between passes, encoding, and output.
- Manual memory management via **linked lists** for symbols, orders, and data.
- **Custom base‑4 textual encoding** (letters `a`–`d`) instead of binary/hex.

---

## 🧑‍💻 Author

**Hadar Halfon**  
Computer Science Student — The Open University of Israel

---

## 🪶 License

Released under the **MIT License** for educational and personal use.

# CPU simulator in C

English · [Français](README.fr.md)

A university project exploring a simplified CPU: registers, segmented memory, addressing modes and instruction handling. The implementation is written in C and uses a hash table for named CPU state.

## What is in the repository?

| Component | Responsibility |
| --- | --- |
| [CPU.c](CPU.c), [CPU.h](CPU.h) | CPU setup, addressing resolution, instruction handlers, stack operations and extra-segment allocation. |
| [Memoire.c](Memoire.c), [Memoire.h](Memoire.h) | Memory segments and allocation management. |
| [Hachage.c](Hachage.c), [Hachage.h](Hachage.h) | Hash table used for registers and named state. |
| [Parser.c](Parser.c), [Parser.h](Parser.h) | Parsing of the simplified assembly input. |
| [main.c](main.c), [input.txt](input.txt) | Demonstration entry point and example input. |

The CPU models registers including `AX`, `BX`, `CX`, `DX`, `IP` and status flags. Addressing helpers cover immediate values, registers, direct memory, register-indirect access and segment overrides. Instruction handlers include `MOV`, `ADD`, `CMP`, `JMP`, `JZ`, `JNZ`, `HALT`, `ALLOC` and `FREE`. Stack push/pop helpers are demonstrated separately.

## Build and run

Use a POSIX environment with **GCC and GNU Make**, such as Linux or Debian under WSL. The source uses POSIX regular expressions (`regex.h`). Run from the repository root:

```sh
make
./main input.txt
```

The program prints addressing examples, register/memory state, a push/pop demonstration and allocation/free output. Representative values from the checked run include:

```text
Test environment initialized.
POP => 42, SP = 1023
```

To remove build artifacts:

```sh
make clean
```

## How the demonstration works

The entry point parses the file passed as its first argument, then frees the parse result. It creates a separate test CPU and exercises addressing, memory transfers, stack helpers and segment allocation.

**The current entry point does not load the parsed instructions into the CPU's code segment.** Consequently, running `./main input.txt` demonstrates the built-in examples; it does not execute that assembly file as a complete program. Instruction handlers and a fetch/execute loop exist in the source, but the entry point does not connect the full path.

## Verification and limits

On **17 November 2026**, `make` and `./main input.txt` completed with exit code 0 under Debian/WSL. The build emitted no warnings in that run. The example includes a successful push/pop of `42`.

These are compilation and demonstration checks, not an automated assertion suite or validation of every instruction. The entry point expects an existing input file and does not check the argument count before using `argv[1]`; supply the filename as shown. The simulator is an educational model, not an implementation of a real CPU instruction set.

[More projects by Illia](https://github.com/IBZua/project-showcase)

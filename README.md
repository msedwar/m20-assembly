# M20 Assembly

A custom instruction set architecture, assembler, linker, and simulator.
The purpose of the project was to gain further insight into operating systems
and low-level processor principles. The fictional processor and architecture is
based on a 32-bit ARM design.

The compiler and simulator are able to run basic assembly programs
(found in the `assembly` directory).

Created by Matthew Edwards \
February 26, 2018

## Instruction Set Architecture

### Modes

| Mode       | Abbreviation | Purpose                            |
|------------|--------------|------------------------------------|
| User       | usr          | Normal Execution                   |
| Supervisor | svr          | Protected OS Execution             |
| Interrupt  | int          | General-Purpose interrupt handling |
| Abort      | abt          | Entered after fatal abort          |

### Registers

* 26 total registers
* 21 general-purpose registers (r0-r12, SP, LP)
* 5 special-purpose registers (PC, ST, SV_svr, SV_int, SV_abt)
* SP, LP, SV registers are banked per mode


|        | User   | Supervisor | Interrupt | Abort  |
|--------|--------|------------|-----------|--------|
| r0-r12 | -      | -          | -         | -      |
| SP     | SP_usr | SP_svr     | SP_int    | SP_abt |
| LP     | LP_usr | LP_svr     | LP_int    | LP_abt |
| PC     | -      | -          | -         | -      |
| ST     | -      | -          | -         | -      |
| SV     | -      | SV_svr     | SV_int    | SV_abt |

### Data Processing

| Opcode | Mnemonic | Name                      | Arguments |
|--------|----------|---------------------------|-----------|
| 0      | NOOP     | No operation              | 0         |
| 1      | ADD      | Add                       | 3         |
| 10     | ADC      | Add with Carry            | 3         |
| 11     | SUB      | Subtract                  | 3         |
| 100    | SBC      | Subtract with Carry       | 3         |
| 101    | MUL      | Multiply                  | 3         |
| 110    | DIV      | Signed Divide             | 3         |
| 111    | UDV      | Unsigned Divide           | 3         |
| 1000   | OR       | Logical Or                | 3         |
| 1001   | AND      | Logical And               | 3         |
| 1010   | XOR      | Logical Xor               | 3         |
| 1011   | NOR      | Logical Nor               | 3         |
| 1100   | BIC      | Bit Clear                 | 3         |
| 1101   | ROR      | Rotate Right              | 3         |
| 1110   | LSL      | Logical Shift Left        | 3         |
| 1111   | LSR      | Logical Shift Right       | 3         |
| 10000  | ASR      | Arithmetic Shift Right    | 3         |
| 10001  | MOV      | Move                      | 2         |
| 10010  | MVN      | Move Negative             | 2         |
| 10011  | CMP      | Compare                   | 2         |
| 10100  | CMN      | Compare Negative          | 2         |
| 10101  | TST      | Test                      | 2         |
| 10110  | TEQ      | Test Equality             | 2         |
| 10111  | PUSH     | Push onto Stack           | 1         |
| 11000  | POP      | Pop from Stack            | 1         |
| 11001  | SRL      | Load from Status Register | 1         |
| 11010  | SRS      | Store to Status Register  | 1         |
| 11111  | HALT     | Halt CPU                  | 0         |

### Data Loading

| Opcode | Mnemonic | Name                   | Arguments |
|--------|----------|------------------------|-----------|
| 0      | LDR      | Load                   | 2-3       |
| 1      | LDRB     | Load Unsigned Byte     | 2-3       |
| 10     | LDRH     | Load Unsigned Halfword | 2-3       |
| 11     | LDRSB    | Load Signed Byte       | 2-3       |
| 100    | LDRSH    | Load Signed Halfword   | 2-3       |
| 101    | STR      | Store                  | 2-3       |
| 110    | STRB     | Store Byte             | 2-3       |
| 111    | STRH     | Store Halfword         | 2-3       |

### Branching

| L Bit | Mnemonic | Name             |
|-------|----------|------------------|
| 0     | B        | Branch           |
| 1     | BWL      | Branch with Link |

### Interrupt

| Mnemonic | Name               |
|----------|--------------------|
| SWI      | Software Interrupt |


## Assembler Directives
    
* global - Declare a global label
* extern - Declare an external label
* entry - Declare the entry point of a program
* section - Declare the start of a section (either text or data)
* space - Create n empty bytes
* $ - Insert final 32-bit address
* db - Insert byte data
* dh - Insert halfword data
* dw - Insert word data
* dd - Insert double data

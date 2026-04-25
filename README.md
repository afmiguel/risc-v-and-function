# AND Gate — RISC-V Assembly Example

Educational example of a logical AND gate implemented in RISC-V Assembly for Raspberry Pi Pico 2 W.

## Description

This project demonstrates how to read two GPIO inputs and combine them with AND logic using only RISC-V conditional branch instructions — no logical (`and`/`or`/`xor`) instructions are used. The assembly code calls a thin C API layer (`pico_gpio_api.c`) that wraps the Pico SDK. This two-layer design keeps the assembly code simple and focused on control flow and calling conventions.

## Hardware

- **Board:** Raspberry Pi Pico 2 W
- **Input A:** GPIO 12
- **Input B:** GPIO 13
- **Output:** GPIO 15
- **Architecture:** RISC-V

## Features

- RISC-V Assembly entry point with clean, readable control flow
- Simplified C API layer callable directly from assembly
- GPIO abstraction through `pico_gpio_init` / `pico_gpio_write` / `pico_gpio_read`
- Configurable pin assignments via `.equ` constants
- AND logic implemented exclusively with conditional branches (`beqz`) — no logical instructions
- RISC-V calling convention (ABI) in practice: argument registers, callee-saved registers, and function calls
- Continuous output loop with no delay — result is driven at maximum speed
- Bare-metal programming educational example

## Code Structure

### Architecture

```
main.S  ──calls──►  pico_gpio_api.c  ──calls──►  Pico SDK  ──►  Hardware
```

### `main.S` — Assembly entry point

Defines configuration constants and implements `main`:

1. **GPIO Initialisation:**
   - Calls `pico_gpio_init(PIN_IN_A, DIR_IN)` — configures pin 12 as input
   - Calls `pico_gpio_init(PIN_IN_B, DIR_IN)` — configures pin 13 as input
   - Calls `pico_gpio_init(PIN_OUT, DIR_OUT)` — configures pin 15 as output

2. **Main Loop (`and_loop`):**
   - Read pin 12 via `pico_gpio_read(PIN_IN_A)` → if 0, jump immediately to `LED_OFF` (pin 13 is not read)
   - Read pin 13 via `pico_gpio_read(PIN_IN_B)` → if 0, jump to `LED_OFF`
   - Fall through to `LED_ON`: write 1 to pin 15, jump back to `and_loop`
   - `LED_OFF`: write 0 to pin 15, jump back to `and_loop`

### AND logic via conditional branches

Instead of a single `and` instruction, a `beqz` placed right after each read short-circuits the loop as soon as a 0 is found:

```asm
and_loop:
    li   a0, PIN_IN_A
    call pico_gpio_read
    beqz a0, LED_OFF         # pin12 == 0 → jump to LED_OFF

    li   a0, PIN_IN_B
    call pico_gpio_read
    beqz a0, LED_OFF         # pin13 == 0 → jump to LED_OFF

LED_ON:
    li   a0, PIN_OUT
    li   a1, 1               # both inputs are 1: drive output high
    call pico_gpio_write
    j    and_loop

LED_OFF:
    li   a0, PIN_OUT
    li   a1, 0               # at least one input is 0: drive output low
    call pico_gpio_write
    j    and_loop
```

Because each `beqz` follows its `call` immediately, `a0` still holds the return value — no extra register is needed to preserve it. If pin 12 is 0, pin 13 is not even read.

Truth table:

| pin12 | pin13 | label reached | output |
|:---:|:---:|:---:|:---:|
| 0 | — | `LED_OFF` | 0 |
| 1 | 0 | `LED_OFF` | 0 |
| 1 | 1 | `LED_ON` | 1 |

If either `beqz` fires, execution jumps to `LED_OFF`. Only when both inputs are non-zero does execution fall through to `LED_ON`.

### Why no extra register is needed

Each `beqz` is placed immediately after its `pico_gpio_read` call, before any other `call` can overwrite `a0`. The first reading is either discarded (jump to `LED_OFF`) or no longer needed (pin12 is 1, so we proceed to read pin13 into the same `a0`). This eliminates the `mv s0, a0` instruction and avoids using any callee-saved register.

### `pico_gpio_api.c` — C GPIO wrapper

Provides three integer-based functions designed to be called from RISC-V Assembly following the standard calling convention (arguments in `a0`–`a7`, return value in `a0`):

| Function | Signature | Description |
|---|---|---|
| `pico_gpio_init` | `(int pin, int is_output)` | Initialises a pin and sets its direction |
| `pico_gpio_write` | `(int pin, int value)` | Drives a pin high (1) or low (0) |
| `pico_gpio_read` | `(int pin) → int` | Returns the current level of a pin (1 or 0) |

Each function wraps the corresponding Pico SDK call (`gpio_init`, `gpio_set_dir`, `gpio_put`, `gpio_get`) and translates between integer arguments and SDK types.

## Minimum Required Program

While `main.S` contains ~50 lines to implement the AND gate, the **smallest valid program** that compiles and runs safely on the Pico 2 W is just three lines:

```asm
.global main
main:
    j main
```

Each line has a specific and mandatory role:

| Line | Role | What happens without it |
|------|------|------------------------|
| `.global main` | Exports the `main` symbol so the linker can find it. The Pico SDK startup code (part of `pico_stdlib`) runs first, initialises the hardware, and then calls `main` by name. | Linker error: `undefined reference to 'main'` |
| `main:` | Defines the label that marks the entry point of the program. | The `.global` directive has nothing to point to; the symbol does not exist. |
| `j main` | Infinite loop — jumps back to `main` unconditionally. Without it, the processor would continue executing whatever bytes follow in memory, causing unpredictable behaviour. | Undefined behaviour: the program falls off the end of the code. |

> **Why does the template have so many more lines?**
> The extra lines in `main.S` implement real functionality: three GPIO initialisations, two reads, AND logic via branches, and a write loop. None of that is required just to *compile* — but all of it is required to *implement the AND gate correctly*.

## Requirements

- Raspberry Pi Pico SDK (version 2.2.0)
- RISC-V Toolchain (RISCV_ZCB_RPI_2_2_0_2)
- CMake (version 3.13 or higher)
- picotool (version 2.2.0-a4)

## Getting Started

### 1. Clone the repository

```bash
git clone https://github.com/afmiguel/risc-v-and-function.git
```

### 2. Open the project in VS Code

**Windows:**
```cmd
cd risc-v-and-function
code .
```

**macOS / Linux:**
```bash
cd risc-v-and-function
code .
```

> **Note:** The `code .` command opens VS Code in the current folder. If it is not recognized, open VS Code manually and use **File → Open Folder** to select the cloned folder.

## Building

The recommended way to build this project is through VS Code with the **Raspberry Pi Pico** extension, which automatically manages the SDK, toolchain, CMake and Ninja on all platforms.

### All platforms (VS Code — recommended)

1. Install [VS Code](https://code.visualstudio.com/) and the **Raspberry Pi Pico** extension
2. Open the project folder in VS Code
3. The extension detects `CMakeLists.txt` and configures everything automatically
4. Click **Compile Project** in the bottom status bar

### Windows (terminal)

Open the **Developer Command Prompt** or **PowerShell** with the Pico environment loaded:

```cmd
mkdir build
cd build
cmake -G "Ninja" ..
ninja
```

### macOS (terminal)

```bash
mkdir build
cd build
cmake ..
ninja
```

### Linux (terminal)

```bash
mkdir build
cd build
cmake ..
ninja
```

The build process will generate:
- `and_function.elf` — Executable
- `and_function.uf2` — File for uploading to Pico
- `and_function.bin` — Binary image
- `and_function.hex` — HEX file

## Uploading to Pico

1. Press and hold the BOOTSEL button on the Pico
2. Connect the Pico to your computer via USB
3. Release the BOOTSEL button (the Pico will appear as a USB drive)
4. Copy the `build/and_function.uf2` file to the Pico drive
5. The Pico will automatically reboot and start reading the inputs continuously

## Configuration

Modify the following constants at the top of `main.S`:

| Constant | Default | Description |
|---|---|---|
| `PIN_IN_A` | `12` | GPIO pin for input A |
| `PIN_IN_B` | `13` | GPIO pin for input B |
| `PIN_OUT` | `15` | GPIO pin for the AND result output |

## Project Structure

```
risc-v-and-function/
├── main.S                # RISC-V Assembly entry point (main)
├── pico_gpio_api.c       # C GPIO wrapper callable from assembly
├── CMakeLists.txt        # CMake build configuration
├── pico_sdk_import.cmake # Pico SDK import helper
├── build/                # Build output directory
└── .vscode/              # VS Code and Pico extension settings
```

## Additional Resources

- [pico_stdlib RISC-V ABI Reference](https://afmiguel.github.io/pico-riscv-asm-reference/) — How to call Pico SDK functions from RISC-V Assembly, with argument-to-register mapping and annotated examples for each function.

## Learning Objectives

This project is ideal for understanding:
- RISC-V Assembly programming and control flow
- RISC-V calling conventions (ABI): argument registers, return values, callee-saved registers
- Cross-language function calls between assembly and C
- Implementing logic functions using only conditional branches
- GPIO input reading and output control through a layered software abstraction
- Pico SDK project structure and CMake build system

## License

Open-source educational project.

## Author

Developed as an educational example for the Microprocessor Systems Design course.

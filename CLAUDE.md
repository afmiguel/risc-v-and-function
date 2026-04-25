# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Educational RISC-V Assembly example for the Raspberry Pi Pico 2 W (RP2350 chip), developed for a Microprocessor Systems Design course. The program implements a logical AND gate: reads GPIO 12 and GPIO 13 as inputs, computes their AND, and drives the result continuously to GPIO 15 as output — with no pause between iterations.

## Build

**Recommended:** VS Code with the [Raspberry Pi Pico extension](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico) — it auto-configures SDK, toolchain, CMake, and Ninja. Click **Compile Project** in the status bar.

**Terminal (macOS/Linux):**
```bash
mkdir build && cd build
cmake ..
ninja
```

**Terminal (Windows):** Use the Developer Command Prompt or PowerShell with the Pico environment loaded, then run the same `cmake`/`ninja` commands.

The SDK and toolchain are expected at `~/.pico-sdk/` (installed by the VS Code extension). Required versions: SDK 2.2.0, toolchain `RISCV_ZCB_RPI_2_2_0_2`, picotool 2.2.0-a4.

Build outputs land in `build/`: `and_function.uf2` (flash to Pico), `and_function.elf`, `and_function.bin`, `and_function.hex`.

**Flash to device:** Hold BOOTSEL, connect USB, release BOOTSEL, copy `build/and_function.uf2` to the Pico mass-storage drive.

## Architecture

The project uses a two-layer design so students can write clean assembly without dealing with raw SDK calling conventions:

- **`main.S`** — RISC-V assembly entry point (`main`). Uses the simplified C API below. Pin constants `PIN_IN_A` (12), `PIN_IN_B` (13), and `PIN_OUT` (15) are defined as `.equ` at the top. No stack prologue is needed because `main` never returns.
- **`pico_gpio_api.c`** — Thin C wrapper around the Pico SDK. Exposes three functions callable from assembly with simple integer arguments:
  - `pico_gpio_init(pin, is_output)` — calls `gpio_init` + `gpio_set_dir`
  - `pico_gpio_write(pin, value)` — calls `gpio_put`
  - `pico_gpio_read(pin)` — calls `gpio_get`, returns 0 or 1
- **`CMakeLists.txt`** — Builds both files as a single `and_function` target, links `pico_stdlib`. Board is `pico2_w`.

## AND Logic in Assembly

The loop (`and_loop`) executes indefinitely without pause:

1. Read PIN_IN_A (12) → save result in `s0` (callee-saved, preserved across calls)
2. Read PIN_IN_B (13) → result in `a0`
3. `beqz s0, LED_OFF` — if pin12 is 0, jump to `LED_OFF`
4. `beqz a0, LED_OFF` — if pin13 is 0, jump to `LED_OFF`
5. Fall through to `LED_ON`: write 1 to PIN_OUT (15), jump back to `and_loop`
6. `LED_OFF`: write 0 to PIN_OUT (15), jump back to `and_loop`

`s0` is used to hold the first reading across the second `call` because `a0`–`a7` are caller-saved and would be overwritten. Since `main` loops forever, `s0` is never restored.

The AND is intentionally implemented without logical instructions (`and`/`or`/`xor`): the two `beqz` branches jump directly to `LED_OFF` (output 0) whenever either input is 0; only when both inputs are non-zero does execution fall through to `LED_ON` (output 1).

## RISC-V / Pico ABI Notes

- Arguments pass in `a0`–`a7`; return value in `a0`.
- `s0`–`s11` are callee-saved: functions you call must preserve them, making them safe to hold values across calls.
- The SDK startup code (`pico_crt0`) runs before `main`; `main` must be exported with `.global main`.
- Because `main` never returns, no stack prologue (save `ra`, adjust `sp`) is required.
- Without a final `j and_loop` (or equivalent infinite loop), the processor falls off the end of code — undefined behaviour on bare metal.

## Reference

[pico_stdlib RISC-V ABI Reference](https://afmiguel.github.io/pico-riscv-asm-reference/) — argument-to-register mapping and annotated examples for calling SDK functions from assembly.

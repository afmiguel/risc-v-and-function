/**
 * @file pico_gpio_api.c
 * @brief Simplified GPIO API for RISC-V Assembly programs on the Raspberry Pi Pico 2 W.
 *
 * @authors Prof. Afonso Ferreira Miguel
 *          Prof. Marcelo do Carmo Camargo Gaiotto
 * @date    2026-04-24
 *
 * This module wraps the Pico SDK GPIO functions behind a minimal integer-based
 * interface designed to be called directly from RISC-V Assembly code following
 * the standard RISC-V calling convention (arguments in a0–a7, return value in a0).
 *
 * It acts as an educational bridge: students write assembly control flow and
 * practise the ABI without needing to handle SDK-specific types or multi-step
 * GPIO initialisation sequences in assembly.
 *
 * Calling convention summary (RISC-V integer ABI):
 *   - First argument  → a0
 *   - Second argument → a1
 *   - Return value    → a0
 */

#include "pico/stdlib.h"

/**
 * @brief Initialise a GPIO pin and configure its direction.
 *
 * Resets the pin to a known state via gpio_init(), then sets it as an
 * input or output via gpio_set_dir().
 *
 * Assembly call example (output, pin 15):
 * @code
 *   li  a0, 15   # pin
 *   li  a1, 1    # is_output = 1 (output)
 *   call pico_gpio_init
 * @endcode
 *
 * @param pin       GPIO pin number (0–29).
 * @param is_output Direction: 1 to configure as output, 0 as input.
 */
void pico_gpio_init(int pin, int is_output) {
    gpio_init(pin);
    gpio_set_dir(pin, is_output ? GPIO_OUT : GPIO_IN);
}

/**
 * @brief Drive a GPIO output pin to a logic level.
 *
 * The pin must have been previously configured as an output with
 * pico_gpio_init().
 *
 * Assembly call example (set pin 15 high):
 * @code
 *   li  a0, 15   # pin
 *   li  a1, 1    # value = 1 (high)
 *   call pico_gpio_write
 * @endcode
 *
 * @param pin   GPIO pin number (0–29).
 * @param value Logic level: 1 for high, 0 for low.
 */
void pico_gpio_write(int pin, int value) {
    gpio_put(pin, value ? true : false);
}

/**
 * @brief Read the current logic level of a GPIO pin.
 *
 * Returns an integer (0 or 1) so the result can be used directly in
 * assembly without type conversion.
 *
 * Assembly call example (read pin 15):
 * @code
 *   li  a0, 15   # pin
 *   call pico_gpio_read
 *   # result in a0: 1 = high, 0 = low
 * @endcode
 *
 * @param pin GPIO pin number (0–29).
 * @return    1 if the pin is high, 0 if the pin is low.
 */
int pico_gpio_read(int pin) {
    return gpio_get(pin) ? 1 : 0;
}

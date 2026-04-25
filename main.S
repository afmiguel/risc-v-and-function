/**
 * RISC-V Assembly AND Function Example
 * Usando API simplificada para estudantes
 */

.section .text
.global main
.type main, @function

# Definições para os estudantes
.equ PIN_IN_A, 12
.equ PIN_IN_B, 13
.equ PIN_OUT,  15
.equ DIR_IN,   0
.equ DIR_OUT,  1

main:
    # Configura pino 12 como entrada: pico_gpio_init(pin=12, is_output=0)
    li   a0, PIN_IN_A
    li   a1, DIR_IN
    call pico_gpio_init

    # Configura pino 13 como entrada: pico_gpio_init(pin=13, is_output=0)
    li   a0, PIN_IN_B
    li   a1, DIR_IN
    call pico_gpio_init

    # Configura pino 15 como saída: pico_gpio_init(pin=15, is_output=1)
    li   a0, PIN_OUT
    li   a1, DIR_OUT
    call pico_gpio_init

and_loop:
    # Lê pino 12: pico_gpio_read(pin=12) → a0; guarda em s0
    li   a0, PIN_IN_A
    call pico_gpio_read
    mv   s0, a0

    # Lê pino 13: pico_gpio_read(pin=13) → a0
    li   a0, PIN_IN_B
    call pico_gpio_read

    # Se qualquer entrada for 0, desliga; se ambas forem 1, liga
    beqz s0, LED_OFF         # pin12 == 0 → salta para LED_OFF
    beqz a0, LED_OFF         # pin13 == 0 → salta para LED_OFF

LED_ON:
    # Ambas entradas são 1: aciona pino 15
    li   a0, PIN_OUT
    li   a1, 1
    call pico_gpio_write
    j    and_loop

LED_OFF:
    # Pelo menos uma entrada é 0: desliga pino 15
    li   a0, PIN_OUT
    li   a1, 0
    call pico_gpio_write
    j    and_loop

.size main, .-main

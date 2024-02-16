[SECTION .text]
[BITS 32]
extern kernel_main

global _start
_start:
; 8259a
.config_8a59a:
    ; send ICW1
    mov al, 11h
    out 20h, al

    ; send ICW1
    out 0a0h, al

    ; send ICW2
    mov al, 20h
    out 21h, al

    ; send ICW2
    mov al, 28h
    out 0a1h, al

    ; send ICW3
    mov al, 04h
    out 21h, al

    ; send ICW3
    mov al, 02h
    out 0A1h , al

    ; send ICW4
    mov al, 003h
    out 021h, al

    ; send ICW4
    out 0A1h, al

    ; for keyboard
.enable_8259a_main:
    mov al, 11111100b
    out 21h, al

    ; disable other interrupt
.disable_8259a_slave:
    mov al, 11111111b
    out 0A1h, al

    ; call main
.enter_c_word:
    call kernel_main

    jmp $
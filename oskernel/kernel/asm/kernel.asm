[SECTION .data]
r3_code_selector equ 4 << 3 | 0b011
r3_data_selector equ 5 << 3 | 0b011

msg: db "0x%08x", 10, 13, 0

[SECTION .text]
[bits 32]

extern printk
extern get_free_page
extern user_entry

; stack:
;   eip
;   cs
;   eflags
;   esp
;   ss
global move_to_user
move_to_user:
    call get_free_page

    push r3_data_selector
    push eax
    pushf
    push r3_code_selector
    push user_entry

    iret
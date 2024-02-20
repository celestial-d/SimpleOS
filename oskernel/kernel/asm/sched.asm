[SECTION .data]
msg: db "hello", 10, 13, 0

[SECTION .text]
[bits 32]

extern printk
extern get_sched_times
extern current_task_exit

extern current

; stack struct:
; return address
; param
global switch_task
switch_task:
    mov ecx, [esp + 8]      ; task

    push ecx
    call get_sched_times
    add esp, 4

    cmp eax, 0
    je .call

.restore_env:
    mov ecx, [current]

    ;mov eax, [ecx + 0x0c + 10 * 4]
    mov edx, [ecx + 0x0c + 12 * 4]
    mov ebx, [ecx + 0x0c + 13 * 4]
    mov esp, [ecx + 0x0c + 14 * 4]
    mov ebp, [ecx + 0x0c + 15 * 4]
    mov esi, [ecx + 0x0c + 16 * 4]
    mov edi, [ecx + 0x0c + 17 * 4]

    mov eax, ecx
    mov ecx, [eax + 0x0c + 11 * 4]

    mov eax, [eax + 0x0c + 8 * 4]
    jmp eax

.call:
    mov ecx, [current]
    mov eax, [ecx + 4]      ; task.funciton

    call eax

.exit:
    call current_task_exit

    ret


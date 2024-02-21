[BITS 32]
[SECTION .text]

extern task_exit
extern sched
extern inc_scheduling_times
extern get_task_ppid

extern current


global switch_task
switch_task:
    ; recover context
    mov eax, [current]

    ; recover ebp0 esp0
    mov esp, [eax + 4]
    mov ebp, [eax + 15 * 4]

    push eax                        ; get ppid
    call get_task_ppid
    add esp, 4
    cmp eax, 0
    jne .recover_env

    mov eax, [current]
    push eax
    call inc_scheduling_times
    add esp, 4

    cmp eax, 0
    jne .recover_env                ; not the first scheduling

    ; if is first time scheduling
    mov eax, task_exit_handler
    push eax

.recover_env:
    mov eax, [current]

    ; recover
    mov ecx, [eax + 11 * 4]
    mov edx, [eax + 12 * 4]
    mov ebx, [eax + 13 * 4]
    mov esi, [eax + 16 * 4]
    mov edi, [eax + 17 * 4]

    mov eax, [eax + 8 * 4]      ; eip

    sti

    jmp eax

task_exit_handler:
    mov eax, [current]
    push eax
    push 0                      ; exit code = 0 exit normally
    call task_exit
    add esp, 8

    call sched

    ; just keep save
    sti
    hlt

; stack:
;   sched_task return address
;   ...
global sched_task
sched_task:
    xchg bx, bx
    xchg bx, bx

    push ecx

    mov ecx, [current]
    cmp ecx, 0
    je .return

    mov [ecx + 10 * 4], eax
    mov [ecx + 12 * 4], edx
    mov [ecx + 13 * 4], ebx
    mov [ecx + 15 * 4], ebp
    mov [ecx + 16 * 4], esi
    mov [ecx + 17 * 4], edi

    mov eax, [esp + 4]          ; eip
    mov [ecx + 8 * 4], eax      ; tss.eip

    mov eax, esp
    add eax, 8
    mov [ecx + 4], eax          ; tss.esp0

    pop ecx
    mov [ecx + 11 * 4], ecx

.return:
    call sched

    ret
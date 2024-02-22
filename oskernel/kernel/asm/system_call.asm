[BITS 32]
[SECTION .text]

extern create_child
extern get_task_pid
extern get_task_ppid

; stack：
;   return address
;   edx
;   ecx
;   ebx
;   eip
;   cs
;   eflags
;   esp
;   ss
;==========
; parent process，return pid
; child processL return 0
global sys_fork
sys_fork:
    cli                     ; disable interrupt

    push 10                 ; create child process
    push .child_return
    push task_name
    call create_child
    add esp, 12

    ; goto child process's stack
.parent_run:
    mov edx, esp            ; save parent pid

    ; parent(ss、esp、eflags) to child stack
    mov ebx, [esp + 4 * 8]  ; ss
    mov esi, [eax + 4 * 14] ; esp
    mov edi, [esp + 4 * 6]  ; eflags

    mov esp, [eax + 4]      ; goto child stack
    push ebx
    push esi
    push edi

    mov [eax + 4], esp      ; update child process esp

    ; parent(cs、eip) to child stack
    mov esp, edx
    mov ecx, [esp]          ; get return address
    mov esi, [esp + 4 * 5]  ; cs
    mov edi, [esp + 4 * 4]  ; eip

    mov esp, [eax + 4]      ; goto child stack
    push esi
    push edi

    push 0                  ; system_call_entry: add esp, 12
    push 0
    push 0

    push ecx

    mov [eax + 4], esp

    ; go to parent process
    mov esp, edx

    push eax                ; get child pid and return
    call get_task_pid
    add esp, 4

    sti

    ret

.child_return:
    mov eax, 0

    sti

    ret

task_name db "t1", 0
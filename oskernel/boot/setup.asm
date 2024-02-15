[ORG  0x500]

[SECTION .data]
KERNEL_ADDR equ 0x1200

[SECTION .gdt]
SEG_BASE equ 0
SEG_LIMIT equ 0xfffff

B8000_SEG_BASE equ 0xb8000
B8000_SEG_LIMIT equ 0x7fff

CODE_SELECTOR equ (1 << 3)
DATA_SELECTOR equ (2 << 3)
B8000_SELECTOR equ (3 << 3)

gdt_base:
    dd 0, 0
gdt_code:
    dw SEG_LIMIT & 0xffff
    dw SEG_BASE & 0xffff
    db SEG_BASE >> 16 & 0xff
    ;    P_DPL_S_TYPE
    db 0b1_00_1_1000
    ;    G_DB_AVL_LIMIT
    db 0b1_1_00_0000 | (SEG_LIMIT >> 16 & 0xf)
    db SEG_BASE >> 24 & 0xf
gdt_data:
    dw SEG_LIMIT & 0xffff
    dw SEG_BASE & 0xffff
    db SEG_BASE >> 16 & 0xff
    ;    P_DPL_S_TYPE
    db 0b1_00_1_0010
    ;    G_DB_AVL_LIMIT
    db 0b1_1_00_0000 | (SEG_LIMIT >> 16 & 0xf)
    db SEG_BASE >> 24 & 0xf
gdt_b8000:
    dw B8000_SEG_LIMIT & 0xffff
    dw B8000_SEG_BASE & 0xffff
    db B8000_SEG_BASE >> 16 & 0xff
    ;    P_DPL_S_TYPE
    db 0b1_00_1_0010
    ;    G_DB_AVL_LIMIT
    db 0b0_1_00_0000 | (B8000_SEG_LIMIT >> 16 & 0xf)
    db B8000_SEG_BASE >> 24 & 0xf
gdt_ptr:
    dw $ - gdt_base
    dd gdt_base

[SECTION .text]
[BITS 16]
global setup_start
setup_start:
    mov     ax, 0
    mov     ss, ax
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     si, ax

    mov     si, prepare_enter_protected_mode_msg
    call    print

enter_protected_mode:
    ; close bios interrupt
    cli

    ; gdt table
    lgdt  [gdt_ptr]

    ; use A20
    in    al,  92h
    or    al,  00000010b
    out   92h, al

    ; set protected mode
    mov   eax, cr0
    or    eax , 1
    mov   cr0, eax

    jmp CODE_SELECTOR:protected_mode

; hwo to call
; mov     si, msg   ; 1 input string
; call    print     ; 2 call
print:
    mov ah, 0x0e
    mov bh, 0
    mov bl, 0x01
.loop:
    mov al, [si]
    cmp al, 0
    jz .done
    int 0x10

    inc si
    jmp .loop
.done:
    ret

[BITS 32]
protected_mode:
    mov ax, DATA_SELECTOR
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov esp, 0x9fbff

    ; read kernel to memory
    mov edi, KERNEL_ADDR
    mov ecx, 3
    mov bl, 60
    call read_hd

    jmp CODE_SELECTOR:KERNEL_ADDR

read_hd:
    ; 0x1f2 8bit set read/write # of sectors
    mov dx, 0x1f2
    mov al, bl
    out dx, al

    ; 0x1f3 8bit iba 0-7
    inc dx
    mov al, cl
    out dx, al

    ; 0x1f4 8bit iba 8-15
    inc dx
    mov al, ch
    out dx, al

    ; 0x1f5 8bit iba 16-23
    inc dx
    shr ecx, 16
    mov al, cl
    out dx, al

    ; 0x1f6 8bit
    ; 0-3 iba24-27
    ; 4 0 main disk 1 other disk
    ; 5、7 set to 1
    ; 6 0: CHS mode ，1 LAB mode
    inc dx
    shr ecx, 8
    and cl, 0b1111
    mov al, 0b1110_0000     ; LBA mode
    or al, cl
    out dx, al

    ; 0x1f7 8bit  port
    inc dx
    mov al, 0x20
    out dx, al

    ; set # of loops
    mov cl, bl
.start_read:
    push cx     ; save # of loops

    call .wait_hd_prepare
    call read_hd_data

    pop cx      ; retrive

    loop .start_read

.return:
    ret

; wait until all data prepared

.wait_hd_prepare:
    mov dx, 0x1f7

.check:
    in al, dx
    and al, 0b1000_1000
    cmp al, 0b0000_1000
    jnz .check

    ret

; read 256*2bytes=512bytes
read_hd_data:
    mov dx, 0x1f0
    mov cx, 256

.read_word:
    in ax, dx
    mov [edi], ax
    add edi, 2
    loop .read_word

    ret

prepare_enter_protected_mode_msg:
    db "Prepare to go into protected mode...", 10, 13, 0
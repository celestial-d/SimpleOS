[ORG  0x7c00]

[SECTION .data]
BOOT_MAIN_ADDR equ 0x500

[SECTION .text]
[BITS 16]
global boot_start
boot_start:
    ; clean
    mov ax, 3
    int 0x10

    mov edi, BOOT_MAIN_ADDR ; to where
    mov ecx, 1      ; starting sector
    mov bl, 2       ; how many sectors
    call read_hd

    mov     si, jmp_to_setup
    call    print

    jmp     BOOT_MAIN_ADDR

read_hd:
    ; 0x1f2 8bit # of sectors
    mov dx, 0x1f2
    mov al, bl
    out dx, al

    ; 0x1f3 8bit iba low 8 bytes
    inc dx
    mov al, cl
    out dx, al

    ; 0x1f4 8bit iba mid 8 bytes
    inc dx
    mov al, ch
    out dx, al

    ; 0x1f5 8bit iba high 8 bytes
    inc dx
    shr ecx, 16
    mov al, cl
    out dx, al

    ; 0x1f6 8bit
    ; 0-3 iba 24-27bytes
    ; 4 0: main hd 1: other hd
    ; 5、7: must be1
    ; 6 0: CHS mode，1:LAB mode
    inc dx
    shr ecx, 8
    and cl, 0b1111
    mov al, 0b1110_0000     ; LBAmode
    or al, cl
    out dx, al

    ; 0x1f7 8bit  command port
    inc dx
    mov al, 0x20
    out dx, al

    ; set # of loops depends on # of sectors
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

; read hd 256 * 2bytes = 512 bytes = 1 sector
read_hd_data:
    mov dx, 0x1f0
    mov cx, 256

.read_word:
    in ax, dx
    mov [edi], ax
    add edi, 2
    loop .read_word

    ret

; how to use
; mov     si, msg   ; 1 get the string
; call    print     ; 2 call the function
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

; 13: enter 10: change to next line
jmp_to_setup:
    db "jump to setup...", 10, 13, 0

times 510 - ($ - $$) db 0
db 0x55, 0xaa
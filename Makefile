BUILD:=./build

CFLAGS:= -m32 # 32bit
CFLAGS+= -masm=intel
CFLAGS+= -fno-builtin	# no gcc inline func
CFLAGS+= -nostdinc		# no std
CFLAGS+= -fno-pic		# no  position independent code
CFLAGS+= -fno-pie		# no position independent executable
CFLAGS+= -nostdlib		# no std
CFLAGS+= -fno-stack-protector	# no stack protection
CFLAGS:=$(strip ${CFLAGS})

DEBUG:= -g

HD_IMG_NAME:= "hd.img"

all: ${BUILD}/boot/boot.o ${BUILD}/boot/setup.o ${BUILD}/system.bin
	$(shell rm -rf $(BUILD)/$(HD_IMG_NAME))
	bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $(BUILD)/$(HD_IMG_NAME)
	dd if=${BUILD}/boot/boot.o of=$(BUILD)/$(HD_IMG_NAME) bs=512 seek=0 count=1 conv=notrunc
	dd if=${BUILD}/boot/setup.o of=$(BUILD)/$(HD_IMG_NAME) bs=512 seek=1 count=2 conv=notrunc
	dd if=${BUILD}/system.bin of=$(BUILD)/$(HD_IMG_NAME) bs=512 seek=3 count=60 conv=notrunc

${BUILD}/system.bin: ${BUILD}/kernel.bin
	objcopy -O binary ${BUILD}/kernel.bin ${BUILD}/system.bin
	nm ${BUILD}/kernel.bin | sort > ${BUILD}/system.map

${BUILD}/kernel.bin: ${BUILD}/boot/head.o ${BUILD}/init/main.o ${BUILD}/kernel/asm/io.o ${BUILD}/kernel/chr_drv/console.o \
    ${BUILD}/lib/string.o ${BUILD}/kernel/vsprintf.o ${BUILD}/kernel/printk.o ${BUILD}/kernel/gdt.o ${BUILD}/kernel/idt.o \
    ${BUILD}/kernel/asm/interrupt_handler.o ${BUILD}/kernel/traps.o ${BUILD}/kernel/chr_drv/keyboard.o ${BUILD}/kernel/exception.o \
    ${BUILD}/kernel/asm/clock_handler.o ${BUILD}/kernel/chr_drv/clock.o ${BUILD}/mm/memory.o ${BUILD}/kernel/kernel.o \
    ${BUILD}/mm/mm_101012.o
	ld -m elf_i386 $^ -o $@ -Ttext 0x1200

${BUILD}/mm/%.o: oskernel/mm/%.c
	$(shell mkdir -p ${BUILD}/mm)
	gcc ${CFLAGS} ${DEBUG} -c $< -o $@

${BUILD}/kernel/%.o: oskernel/kernel/%.c
	$(shell mkdir -p ${BUILD}/kernel)
	gcc ${CFLAGS} ${DEBUG} -c $< -o $@

${BUILD}/lib/%.o: oskernel/lib/%.c
	$(shell mkdir -p ${BUILD}/lib)
	gcc ${CFLAGS} ${DEBUG} -c $< -o $@

${BUILD}/kernel/chr_drv/%.o: oskernel/kernel/chr_drv/%.c
	$(shell mkdir -p ${BUILD}/kernel/chr_drv)
	gcc ${CFLAGS} ${DEBUG} -c $< -o $@

${BUILD}/kernel/asm/%.o: oskernel/kernel/asm/%.asm
	$(shell mkdir -p ${BUILD}/kernel/asm)
	nasm -f elf32 -g $< -o $@

${BUILD}/init/main.o: oskernel/init/main.c
	$(shell mkdir -p ${BUILD}/init)
	gcc ${CFLAGS} ${DEBUG} -c $< -o $@

${BUILD}/boot/head.o: oskernel/boot/head.asm
	nasm -f elf32 -g $< -o $@

${BUILD}/boot/%.o: oskernel/boot/%.asm
	$(shell mkdir -p ${BUILD}/boot)
	nasm $< -o $@

clean:
	$(shell rm -rf ${BUILD})

bochs: all
	bochs -q -f bochsrc

qemug: all
	qemu-system-x86_64 -m 32M -hda ./build/hd.img -S -s

qemu: all
	qemu-system-i386 \
	-m 32M \
	-boot c \
	-hda ./build/hd.img

# for vmware
vmdk: $(BUILD)/master.vmdk

$(BUILD)/master.vmdk: ./build/hd.img
	qemu-img convert -O vmdk $< $@
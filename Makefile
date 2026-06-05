# =============================================
# Compact Makefile for i686 OS
# =============================================

# Tools
AS      = i686-elf-as
CC      = i686-elf-gcc
LD      = i686-elf-gcc
GRUB    = grub-mkrescue
QEMU    = qemu-system-i386

# Flags
ASFLAGS =
CFLAGS  = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib -lgcc

# Files
OBJ     = boot.o kernel.o gdt.o gdt_flush.o idt.o isr.o pic.o keyboard.o terminal.o paging.o paging_asm.o pmm.o heap.o task.o switch.o semaphore.o tss.o usermode.o shell.o elf.o test_module.o module_blob.o process.o syscall.o ramfs.o hello_blob.o
KERNEL  = myos
ISO     = myos.iso
ISODIR  = isodir

# =============================================
.PHONY: all clean iso run run-iso

all: $(KERNEL)

# Assembly + Compilation + Linking
boot.o: boot.s
	$(AS) $(ASFLAGS) $< -o $@

gdt_flush.o: gdt_flush.s
	$(AS) $(ASFLAGS) $< -o $@

kernel.o: kernel.c
	$(CC) -c $< -o $@ $(CFLAGS)

gdt.o: gdt.c gdt.h
	$(CC) -c $< -o $@ $(CFLAGS)

isr.o: isr.s
	$(AS) $(ASFLAGS) $< -o $@

idt.o: idt.c idt.h
	$(CC) -c $< -o $@ $(CFLAGS)

pic.o: pic.c pic.h
	$(CC) -c $< -o $@ $(CFLAGS)

keyboard.o: keyboard.c keyboard.h
	$(CC) -c $< -o $@ $(CFLAGS)

terminal.o: terminal.c terminal.h
	$(CC) -c $< -o $@ $(CFLAGS)

paging.o: paging.c paging.h
	$(CC) -c $< -o $@ $(CFLAGS)

paging_asm.o: paging.s
	$(AS) $(ASFLAGS) $< -o $@

pmm.o: pmm.c pmm.h
	$(CC) -c $< -o $@ $(CFLAGS)

heap.o: heap.c heap.h
	$(CC) -c $< -o $@ $(CFLAGS)

task.o: task.c task.h
	$(CC) -c $< -o $@ $(CFLAGS)

switch.o: switch.s
	$(AS) $(ASFLAGS) $< -o $@

semaphore.o: semaphore.c semaphore.h
	$(CC) -c $< -o $@ $(CFLAGS)

tss.o: tss.c tss.h
	$(CC) -c $< -o $@ $(CFLAGS)

usermode.o: usermode.s
	$(AS) $(ASFLAGS) $< -o $@

shell.o: shell.c shell.h
	$(CC) -c $< -o $@ $(CFLAGS)

elf.o: elf.c elf.h
	$(CC) -c $< -o $@ $(CFLAGS)

test_module.o: test_module.c
	$(CC) -c $< -o $@ -ffreestanding -O2 -nostdlib

module_blob.o: test_module.o
	i686-elf-ld -r -b binary test_module.o -o module_blob.o

process.o: process.c process.h
	$(CC) -c $< -o $@ $(CFLAGS)

syscall.o: syscall.c syscall.h
	$(CC) -c $< -o $@ $(CFLAGS)

ramfs.o: ramfs.c ramfs.h
	$(CC) -c $< -o $@ $(CFLAGS)

hello_user.elf: hello_user.c user.ld
	i686-elf-gcc -ffreestanding -nostdlib -O2 -e _start \
	    -Wl,-T,user.ld \
	    -o hello_user.elf hello_user.c

hello_blob.o: hello_user.elf
	i686-elf-ld -r -b binary hello_user.elf -o hello_blob.o

$(KERNEL): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# ISO creation
iso: $(KERNEL)
	mkdir -p $(ISODIR)/boot/grub
	cp $(KERNEL) $(ISODIR)/boot/
	cp grub.cfg $(ISODIR)/boot/grub/
	$(GRUB) -o $(ISO) $(ISODIR)

# Running
run: $(KERNEL)
	$(QEMU) -kernel $(KERNEL) -no-reboot -no-shutdown

run-iso: iso
	$(QEMU) -cdrom $(ISO) -boot d -no-reboot -no-shutdown

# Cleanup
clean:
	rm -f *.o $(KERNEL) $(ISO)
	rm -rf $(ISODIR)

# Extra useful targets
rebuild: clean all

# Default
.DEFAULT_GOAL := all
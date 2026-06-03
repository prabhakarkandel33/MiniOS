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
OBJ     = boot.o kernel.o gdt.o gdt_flush.o idt.o isr.o pic.o keyboard.o terminal.o paging.o paging_asm.o pmm.o heap.o task.o switch.o semaphore.o tss.o usermode.o
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
	$(QEMU) -kernel $(KERNEL)

run-iso: iso
	$(QEMU) -cdrom $(ISO) -boot d

# Cleanup
clean:
	rm -f *.o $(KERNEL) $(ISO)
	rm -rf $(ISODIR)

# Extra useful targets
rebuild: clean all

# Default
.DEFAULT_GOAL := all
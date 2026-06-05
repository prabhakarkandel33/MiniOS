# MyOS

MyOS is a hobby 32-bit operating system for the i686 architecture. It is built with a freestanding cross-compiler and runs in QEMU for development and testing.

## Features

- Interrupt handling.
- GDT, IDT, ISR, PIC, and TSS support.
- Virtual memory management with paging.
- Physical memory management.
- Kernel heap allocator.
- Multitasking and task switching.
- User mode support.
- System calls.
- ELF loading for user programs.
- RAM filesystem.
- Keyboard driver.
- Terminal output.
- Basic shell.
- Semaphore support.

## Requirements

MyOS uses an `i686-elf` cross-toolchain for building the kernel.

### Windows

If you are on Windows, install WSL first using Microsoft’s official instructions. After WSL is installed, open Ubuntu from the Start Menu or run `wsl` in a terminal.

### Linux or WSL

Install the required build tools:

```bash
sudo apt update
sudo apt install -y build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo grub-pc-bin xorriso qemu-system-x86
```

## Cross-Compiler

You need a freestanding `i686-elf` GCC toolchain, which is the standard setup for OS development.

The OSDev Wiki has a dedicated GCC cross-compiler guide that explains how to build one for your target system.

Make sure `i686-elf-gcc`, `i686-elf-as`, and `i686-elf-ld` are in your `PATH` before building.

## Build

```bash
make
```

## Run

```bash
make run
```

## ISO

Create an ISO image:

```bash
make iso
```

Run the ISO in QEMU:

```bash
make run-iso
```

## Clean

```bash
make clean
```

## Project Structure

- `boot.s` - boot entry code.
- `kernel.c` - kernel core.
- `gdt.c`, `idt.c`, `tss.c` - CPU descriptor setup.
- `paging.c`, `pmm.c`, `heap.c` - memory management.
- `task.c`, `switch.s`, `semaphore.c` - multitasking and synchronization.
- `usermode.s`, `syscall.c`, `process.c`, `elf.c` - user mode and program loading.
- `ramfs.c` - RAM filesystem.
- `keyboard.c`, `terminal.c`, `shell.c` - input and console interface.

## Credits

This project was built using ideas, documentation, and tutorials from the OSDev Wiki: https://wiki.osdev.org/

## Notes

This is a learning project and is not a production-ready operating system.

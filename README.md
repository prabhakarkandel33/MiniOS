# MiniOS

A hobby 32-bit kernel for the i686 architecture. It includes interrupts, virtual memory, basic user mode support, a RAM filesystem, and several core OS subsystems.

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

This project uses a freestanding `i686-elf` cross-toolchain [web:9].

### On Windows

Install WSL first by running `wsl --install` in an elevated terminal [web:7].  
After installation, start Ubuntu with `wsl -d Ubuntu` or open it from the Start Menu [web:5].  
If you want a specific Ubuntu release, Microsoft’s Ubuntu WSL docs also support installing from a downloaded `.wsl` file or by selecting Ubuntu from the Microsoft Store [web:5].

### On Linux or inside WSL

Install the build dependencies:

```bash
sudo apt update
sudo apt install -y build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo grub-pc-bin xorriso qemu-system-x86
```

## Install the cross-compiler

The kernel expects `i686-elf-gcc`, `i686-elf-as`, and `i686-elf-ld` to be available in your `PATH` [web:9].

A common setup is to build a freestanding cross-compiler for the `i686-elf` target [web:9].  
After installation, make sure the toolchain bin directory is added to `PATH` so your Makefile can find it [web:9].

## Build

```bash
make
```

## Run in QEMU

```bash
make run
```

## Build an ISO

```bash
make iso
```

## Run the ISO

```bash
make run-iso
```

## Clean

```bash
make clean
```

## Project Layout

- `boot.s` - boot entry code.
- `kernel.c` - kernel entry and core logic.
- `gdt.c`, `idt.c`, `tss.c` - CPU descriptor setup.
- `paging.c`, `pmm.c`, `heap.c` - memory management.
- `task.c`, `switch.s`, `semaphore.c` - multitasking and synchronization.
- `usermode.s`, `syscall.c`, `process.c`, `elf.c` - user mode and program loading.
- `ramfs.c` - RAM filesystem.
- `keyboard.c`, `terminal.c`, `shell.c` - input and console interface.

## Notes

This is a learning project and is not a production OS.  
It is built with a freestanding toolchain and booted in an emulator for development.

## Credits

This project was built with help from the [OSDev Wiki](https://wiki.osdev.org/), an excellent community resource for operating system development.

Special thanks to the OSDev community for documentation, tutorials, and low-level systems guidance.

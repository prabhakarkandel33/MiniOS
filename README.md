# MiniOS

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

## Windows Setup

If you are on Windows, install WSL first by running the following command in PowerShell as Administrator:

```powershell
wsl --install
```

This installs WSL and the default Ubuntu distribution on supported systems. After installation, restart if prompted, then open Ubuntu from the Start Menu or run:

```powershell
wsl
```

## Linux or WSL Setup

Once you are inside Linux or Ubuntu on WSL, install the required build tools:

```bash
sudo apt update
sudo apt install -y build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo grub-pc-bin xorriso qemu-system-x86
```

## Cross-Compiler Installation

This project requires a freestanding `i686-elf` GCC toolchain, which is the standard setup for hobby operating system development.

The OSDev Wiki provides a full GCC cross-compiler guide here:

https://wiki.osdev.org/GCC_Cross-Compiler

To install the toolchain, build `binutils` and `gcc` for the `i686-elf` target, then install them into a prefix such as `/usr/local/i686-elf`.

A typical setup uses the following environment variables:

```bash
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"
```

After that, build and install `binutils`:

```bash
mkdir -p src
cd src
wget https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.gz
tar -xvf binutils-2.42.tar.gz
mkdir build-binutils
cd build-binutils
../binutils-2.42/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install
```

Then build and install `gcc`:

```bash
cd ..
wget https://ftp.gnu.org/gnu/gcc/gcc-14.1.0/gcc-14.1.0.tar.gz
tar -xvf gcc-14.1.0.tar.gz
mkdir build-gcc
cd build-gcc
../gcc-14.1.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
make install-gcc
make install-target-libgcc
```

Add the toolchain to your shell configuration so it is available every time you open a terminal:

```bash
echo 'export PREFIX="$HOME/opt/cross"' >> ~/.bashrc
echo 'export TARGET=i686-elf' >> ~/.bashrc
echo 'export PATH="$PREFIX/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

To verify the installation, run:

```bash
i686-elf-gcc --version
i686-elf-as --version
i686-elf-ld --version
```

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

This project was built using ideas, documentation, and tutorials from the [OSDev Wiki](https://wiki.osdev.org/).

## Notes

This is a learning project and is not a production-ready operating system.

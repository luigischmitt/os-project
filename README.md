# os-project

A minimal x86 operating system kernel built from scratch, following [The Little OS Book](https://littleosbook.github.io/) (chapters 2 and 3).

The kernel boots via GRUB using the Multiboot specification, transitions from assembly to C, and writes directly to VGA memory.

## What it does

- Loads a Multiboot-compliant ELF kernel through GRUB
- Enters 32-bit protected mode via the bootloader
- Calls `kernel_main` from assembly
- Prints "Hello from kernel_main" to the VGA text buffer at `0xB8000`

## Project structure

```
src/boot.asm        Multiboot header and entry point (_start)
src/kernel.c        C kernel with VGA text output
linker.ld           Linker script (loads kernel at 1 MB)
iso/boot/grub/      GRUB configuration for bootable ISO
Makefile            Build and run targets
Dockerfile          Cross-compilation environment (x86 toolchain)
```

## Prerequisites
make -j$(nproc)
sudo make install
- sudo apt update
- sudo apt install build-essential libsdl1.2-dev wget tar
- sudo apt install bochsbios vgabios
- BOCHS 2.7 : wget https://downloads.sourceforge.net/project/bochs/bochs/2.7/bochs-2.7.tar.gz
- tar xvf bochs-2.7.tar.gz
- cd bochs-2.7
- ./configure --enable-x86-64 --with-sdl --enable-gdb-stub --enable-all-optimizations
- make -j$(nproc)
- sudo make install
## Run

- make run

## Clean

- make clean

## References

- [The Little OS Book](https://littleosbook.github.io/)
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
- [OSDev Wiki](https://wiki.osdev.org/)

## License

[MIT](LICENSE)

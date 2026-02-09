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

- [Docker](https://www.docker.com/)
- [QEMU](https://www.qemu.org/) (for running the ISO)

On macOS:

```sh
brew install qemu
```

## Build

Build the Docker image (first time only):

```sh
docker build -t os-project .
```

Compile the kernel and generate the bootable ISO:

```sh
docker run --rm -it -v "$PWD":/work os-project make
```

## Run

```sh
qemu-system-i386 -cdrom dist/os.iso
```

## Clean

```sh
docker run --rm -it -v "$PWD":/work os-project make clean
```

## References

- [The Little OS Book](https://littleosbook.github.io/)
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
- [OSDev Wiki](https://wiki.osdev.org/)

## License

[MIT](LICENSE)

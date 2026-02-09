# Makefile
CC=gcc
LD=ld
NASM=nasm

CFLAGS=-m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector
LDFLAGS=-m elf_i386 -T linker.ld -nostdlib

BUILD=build
ISO=dist/os.iso

all: $(ISO)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.o: src/boot.asm | $(BUILD)
	$(NASM) -f elf32 $< -o $@

$(BUILD)/kernel.o: src/kernel.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/kernel.elf: $(BUILD)/boot.o $(BUILD)/kernel.o linker.ld
	$(LD) $(LDFLAGS) $(BUILD)/boot.o $(BUILD)/kernel.o -o $@

$(ISO): $(BUILD)/kernel.elf
	rm -rf dist iso/boot/kernel.elf
	mkdir -p dist
	cp $(BUILD)/kernel.elf iso/boot/kernel.elf
	grub-mkrescue -o $(ISO) iso >/dev/null 2>&1 || grub-mkrescue -o $(ISO) iso

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

clean:
	rm -rf $(BUILD) dist iso/boot/kernel.elf

.PHONY: all run clean

# Makefile
CC=gcc
LD=ld
NASM=nasm

CFLAGS= -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror
LDFLAGS=-m elf_i386 -T linker.ld -nostdlib

BUILD=build
ISO=dist/os.iso

all: $(ISO)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/loader.o: src/loader.s | $(BUILD)
	$(NASM) -f elf32 $< -o $@

$(BUILD)/kmain.o: src/kmain.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/kernel.elf: $(BUILD)/loader.o $(BUILD)/kmain.o linker.ld
	$(LD) $(LDFLAGS) $(BUILD)/loader.o $(BUILD)/kmain.o -o $@

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

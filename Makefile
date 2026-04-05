#Searching for the loader
LOADER_SRC = source/boot/loader.s
LOADER_OBJ = $(LOADER_SRC:.s=.o)

# 2. searching for .c and .s files, ignoring the loader.s
C_SOURCES = $(wildcard source/*/*.c)
S_SOURCES = $(filter-out $(LOADER_SRC), $(wildcard source/*/*.s))

# 3. Objects list
OBJECTS = $(LOADER_OBJ) $(C_SOURCES:.c=.o) $(S_SOURCES:.s=.o)

CC = gcc
# Iinclude sets the include file as our main file for includes
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
    -nostartfiles -nodefaultlibs -Wall -Wextra -c -Iinclude

# Linker
LDFLAGS = -T linker/link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf

# Kernel location
KERNEL_BIN = iso/boot/kernel.elf

all: $(KERNEL_BIN)

$(KERNEL_BIN): $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o $@

os.iso: $(KERNEL_BIN) iso/modules/program
	@mkdir -p out
	genisoimage -R                              \
		-b boot/grub/stage2_eltorito    \
		-no-emul-boot                   \
		-boot-load-size 4               \
		-A os                           \
		-input-charset utf8             \
		-quiet                          \
		-boot-info-table                \
		-o out/os.iso                       \
		iso

iso/modules/program: iso/modules/program.s
	nasm -f bin $< -o $@

run: os.iso
# Bochs config
	bochs -f bconfig/bochsrc.txt -q

# Generic rules to compile .c and .s to .o
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS) *.o *.out out/* iso/modules/program $(KERNEL_BIN)
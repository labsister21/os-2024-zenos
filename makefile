# Compiler & linker
ASM           = nasm
LIN           = ld
CC            = gcc

# Directory
SOURCE_FOLDER = src
OUTPUT_FOLDER = bin
ISO_NAME      = OS2024

# Flags
WARNING_CFLAG = -Wall -Wextra -Werror
DEBUG_CFLAG   = -fshort-wchar -g
STRIP_CFLAG   = -nostdlib -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding
CFLAGS        = $(DEBUG_CFLAG) $(WARNING_CFLAG) $(STRIP_CFLAG) -m32 -c -I$(SOURCE_FOLDER)
AFLAGS        = -f elf32 -g -F dwarf
LFLAGS        = -T $(SOURCE_FOLDER)/linker.ld -melf_i386
BOOT_FLAG	  = -b boot/grub/grub1 -no-emul-boot -boot-load-size 4
IO_FLAG		  = -A os -input-charset utf8 -quiet -boot-info-table -o $(OUTPUT_FOLDER)/$(ISO_NAME).iso $(OUTPUT_FOLDER)/iso

DISK_NAME      = storage

disk:
	@qemu-img create -f raw $(OUTPUT_FOLDER)/$(DISK_NAME).bin 4M

run: all
		@qemu-system-i386 -s -S -drive file=${OUTPUT_FOLDER}/${DISK_NAME}.bin,format=raw,if=ide,index=0,media=disk -cdrom $(OUTPUT_FOLDER)/$(ISO_NAME).iso
all: disk insert-shell insert-clock insert-clockbuddy
build: iso
clean:
	rm -rf *.o *.iso $(OUTPUT_FOLDER)/kernel



kernel:
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/kernel-entrypoint.s -o $(OUTPUT_FOLDER)/kernel-entrypoint.o
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/interrupt/interrupt.s -o $(OUTPUT_FOLDER)/Interrupt.o
# TODO: Compile C file with CFLAGS
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/cpu/portio.c -o $(OUTPUT_FOLDER)/portio.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/framebuffer.c -o $(OUTPUT_FOLDER)/framebuffer.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/cpu/gdt.c -o $(OUTPUT_FOLDER)/gdt.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/interrupt/interrupt.c -o $(OUTPUT_FOLDER)/interruptC.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/filesystem/disk.c -o $(OUTPUT_FOLDER)/diskC.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/filesystem/fat32.c -o $(OUTPUT_FOLDER)/fat32C.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/memory/paging.c -o $(OUTPUT_FOLDER)/paging.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/interrupt/idt.c -o $(OUTPUT_FOLDER)/idt.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/keyboard.c -o $(OUTPUT_FOLDER)/keyboard.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/keyboard.c -o $(OUTPUT_FOLDER)/keyboard.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/stdlib/string.c -o $(OUTPUT_FOLDER)/string.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/process.c -o $(OUTPUT_FOLDER)/process.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/scheduler.c -o $(OUTPUT_FOLDER)/scheduler.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/cmos.c -o $(OUTPUT_FOLDER)/cmos.o
	@$(LIN) $(LFLAGS) bin/*.o -o $(OUTPUT_FOLDER)/kernel
	@echo Linking object files and generate elf32...
	@rm -f *.o

iso: kernel
	@mkdir -p $(OUTPUT_FOLDER)/iso/boot/grub
	@cp $(OUTPUT_FOLDER)/kernel     $(OUTPUT_FOLDER)/iso/boot/
	@cp other/grub1                 $(OUTPUT_FOLDER)/iso/boot/grub/
	@cp $(SOURCE_FOLDER)/menu.lst   $(OUTPUT_FOLDER)/iso/boot/grub/
# TODO: Create ISO image
	@genisoimage -R $(BOOT_FLAG) $(IO_FLAG)
	@rm -r $(OUTPUT_FOLDER)/iso/

inserter:
	@$(CC) -Wno-builtin-declaration-mismatch -g -I$(SOURCE_FOLDER) \
		$(SOURCE_FOLDER)/stdlib/string.c \
		$(SOURCE_FOLDER)/filesystem/fat32.c \
		$(SOURCE_FOLDER)/external/external-inserter.c \
		-o $(OUTPUT_FOLDER)/inserter

user-shell:
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/crt0.s -o crt0.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user-shell.c -o user-shell.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/stdlib/string.c -o string.o
	@$(LIN) -T $(SOURCE_FOLDER)/user-linker.ld -melf_i386 --oformat=binary \
		crt0.o user-shell.o string.o -o $(OUTPUT_FOLDER)/shell
	@echo Linking object shell object files and generate flat binary...
	@$(LIN) -T $(SOURCE_FOLDER)/user-linker.ld -melf_i386 --oformat=elf32-i386 \
		crt0.o user-shell.o string.o -o $(OUTPUT_FOLDER)/shell_elf
	@echo Linking object shell object files and generate ELF32 for debugging...
	@size --target=binary $(OUTPUT_FOLDER)/shell
	@rm -f *.o


user-clock:
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/crt0.s -o crt0.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/clock.c -o clock.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/stdlib/string.c -o string.o
	@$(LIN) -T $(SOURCE_FOLDER)/user-linker.ld -melf_i386 --oformat=binary \
		crt0.o clock.o string.o -o $(OUTPUT_FOLDER)/clock
	@echo Linking object clock object files and generate flat binary...
	@$(LIN) -T $(SOURCE_FOLDER)/user-linker.ld -melf_i386 --oformat=elf32-i386 \
		crt0.o clock.o string.o -o $(OUTPUT_FOLDER)/clock_elf
	@echo Linking object clock object files and generate ELF32 for debugging...
	@size --target=binary $(OUTPUT_FOLDER)/clock
	@rm -f *.o

user-clockbuddy:
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/crt0.s -o crt0.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/buddy.c -o buddy.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/stdlib/string.c -o string.o
	@$(LIN) -T $(SOURCE_FOLDER)/user-linker.ld -melf_i386 --oformat=binary \
		crt0.o buddy.o string.o -o $(OUTPUT_FOLDER)/buddy
	@echo Linking object clock object files and generate flat binary...
	@$(LIN) -T $(SOURCE_FOLDER)/user-linker.ld -melf_i386 --oformat=elf32-i386 \
		crt0.o buddy.o string.o -o $(OUTPUT_FOLDER)/buddy_elf
	@echo Linking object clock object files and generate ELF32 for debugging...
	@size --target=binary $(OUTPUT_FOLDER)/buddy
	@rm -f *.o


insert-shell: inserter user-shell
	@echo Inserting shell into root directory...
	@cd $(OUTPUT_FOLDER); ./inserter shell 2 $(DISK_NAME).bin


insert-clock: inserter user-clock
	@echo Inserting clock into root directory...
	@cd $(OUTPUT_FOLDER); ./inserter clock 2 $(DISK_NAME).bin

insert-clockbuddy: inserter user-clockbuddy
	@echo Inserting clock into root directory...
	@cd $(OUTPUT_FOLDER); ./inserter buddy 2 $(DISK_NAME).bin
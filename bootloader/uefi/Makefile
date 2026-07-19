.PHONY: all run clean

COMMON_DIR  := ../../common
EFI_INCLUDE := /usr/include/efi

# Optional OS-specific kernel loader.
#   make WITH_LOADKERNEL=1    -> shell + `load` (hobbyOS)
#   make 						   				-> pure UEFI shell, no kernel/BootInfo coupling
WITH_LOADKERNEL ?= 0

# All C sources, discovered recursively so new files need no Makefile edits.
SRCS := $(shell find . -name '*.c')

ifeq ($(WITH_LOADKERNEL),1)
    DEFINES := -DWITH_LOADKERNEL
else
    SRCS := $(filter-out ./shell/commands/cmd_loadkernel.c,$(SRCS))
    DEFINES :=
endif

CFLAGS := \
		-target x86_64-windows-gnu \
		-ffreestanding \
		-fshort-wchar \
		-fno-stack-protector \
		-mno-red-zone \
		-nostdlib \
		-fuse-ld=lld \
		-I$(EFI_INCLUDE) \
		-I$(COMMON_DIR) \
		$(DEFINES)

LDFLAGS := \
		-Wl,/subsystem:efi_application \
		-Wl,/entry:efi_main \
		-Wl,/align:4096

all:
	clang $(CFLAGS) $(LDFLAGS) $(SRCS) -o BOOTX64.EFI
	mkdir -p diskimg/EFI/BOOT/
	cp BOOTX64.EFI diskimg/EFI/BOOT/BOOTX64.EFI

run:
	qemu-system-x86_64 \
		-drive if=pflash,format=raw,file=OVMF/OVMF_CODE.4m.fd,readonly=on \
		-drive if=pflash,format=raw,file=OVMF/OVMF_VARS.4m.fd \
		-drive format=raw,file=fat:rw:diskimg \
		-machine q35 \
		-net none

clean:
	rm -f *.EFI

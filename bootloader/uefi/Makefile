.PHONY: all run clean

COMMON_DIR := ../../common
EFI_INCLUDE := /usr/include/efi

all:
	clang \
		-target x86_64-windows-gnu \
		-ffreestanding \
		-fshort-wchar \
		-fno-stack-protector \
		-mno-red-zone \
		-nostdlib \
		-fuse-ld=lld \
		-I$(EFI_INCLUDE) \
		-I$(COMMON_DIR) \
		-Wl,/subsystem:efi_application \
		-Wl,/entry:efi_main \
		-Wl,/align:4096 \
		*.c \
		-o BOOTX64.EFI
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
.PHONY: all kernel uefiboot uefirun clean
all: kernel uefiboot
kernel:
	$(MAKE) -C kernel
uefiboot:
	$(MAKE) -C bootloader/uefi WITH_LOADKERNEL=1
uefirun:
	$(MAKE) -C bootloader/uefi run
clean:
	find . -name "*.o" -delete
	find . -name "*.elf" -delete
	find . -name "*.EFI" -delete

TARGET = kernel.elf
LINKER = kernel.ld

CC = clang
LD = ld.lld

COMMON_DIR := ../common
DISKIMG_DIR := ../bootloader/uefi/diskimg

CFLAGS = \
	-target x86_64-unknown-none \
	-I$(COMMON_DIR) \
	-ffreestanding \
	-fno-stack-protector \
	-fno-builtin \
	-mno-red-zone \
	-fPIE \
	-nostdlib \
	-Wall -Wextra \
	-g

LDFLAGS = -nostdlib -pie -T $(LINKER)

SRCS := kernel.c basic_font_bitmap.c
OBJS := $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS) $(LINKER)
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET)
	mkdir -p $(DISKIMG_DIR)
	cp $(TARGET) $(DISKIMG_DIR)/

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.d $(TARGET)

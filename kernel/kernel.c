#include <stdint.h>
#include <stddef.h>
#include "basic_font_bitmap.h"
#include "../common/bootinfo.h"

static BootInfo* bootinfo = NULL;

/* ================= Framebuffer ================= */

static inline void put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!bootinfo) return;
    if (x >= bootinfo->width || y >= bootinfo->height) return;

    uint32_t* fb = bootinfo->framebuffer;
    fb[y * bootinfo->pitch + x] = color;
}

static void clear_screen(uint32_t color) {
    for (uint32_t y = 0; y < bootinfo->height; y++) {
        for (uint32_t x = 0; x < bootinfo->width; x++) {
            put_pixel(x, y, color);
        }
    }
}

/* ================= Text ================= */

void draw_char(uint32_t x, uint32_t y, unsigned char c, uint32_t color) {
    if (c < 0x20 || c > 0x7E) return;

    uint8_t* glyph = font8x8_basic[c - 0x20];

    for (uint32_t row = 0; row < 8; row++) {
        uint8_t bits = glyph[row];

        for (uint32_t col = 0; col < 8; col++) {
            if (bits & (1 << (7 - col))) {
                put_pixel(x + col, y + row, color);
            }
        }
    }
}

static uint32_t cursor_x = 10;
static uint32_t cursor_y = 10;

void print(const char* str, uint32_t color) {
    while (*str) {
        if (*str == '\n' || cursor_x + 8 > bootinfo->width) {
            cursor_x = 10;
            cursor_y += 10;
        }

        if (*str != '\n') {
            draw_char(cursor_x, cursor_y, *str, color);
            cursor_x += 8;
        }

        str++;
    }
}

/* ================= Kernel Entry ================= */

__attribute__((noreturn))
void _start(BootInfo* info) {
  
    bootinfo = info;
    
    if (!bootinfo || !bootinfo->framebuffer) {
        for (;;) __asm__ volatile ("hlt");
    }

    clear_screen(0x00202020); // dark gray

    print("KERNEL STARTED\n", 0x00FF00);

    if (!bootinfo->mem_map || bootinfo->mem_map_entries == 0) {
        print("NO MEMORY MAP\n", 0xFF0000);
        for (;;) __asm__ volatile ("hlt");
    }

    print("MEMORY MAP OK\n", 0x00FF00);

    print(" _           _     _            ___  ____         \n", 0x00FFFF);
    print("| |__   ___ | |__ | |__  _   _ / _ \\/ ___|       \n", 0x00FFFF);
    print("|  _ \\ / _ \\| '_ \\| '_ \\| | | | | | \\___ \\  \n", 0x00FFFF);
    print("| | | | (_) | |_) | |_) | |_| | |_| |___) |       \n", 0x00FFFF);
    print("|_| |_|\\___/|_.__/|_.__/ \\__, |\\___/|____/     \n", 0x00FFFF);
    print("                          |___|                   \n\n", 0x00FFFF);

    print("Welcome to HobbyOS kernel.\n", 0x00FFFF);

    while (1)
        __asm__ volatile ("hlt");
}
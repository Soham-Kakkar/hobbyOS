#pragma once
#include <stdint.h>

typedef enum {
    MEM_RESERVED = 0,
    MEM_USABLE   = 1,
    MEM_ACPI     = 2,
    MEM_MMIO     = 3
} MemoryType;

typedef struct {
    uint64_t base;   // physical start
    uint64_t length; // bytes
    uint64_t attr; // optional attributes (cache, write-back, etc)
    MemoryType type; // usable, reserved, etc.
} KernelMemoryRegion;


typedef struct {
    uint32_t* framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    KernelMemoryRegion* mem_map;
    uint64_t mem_map_entries;
} BootInfo;

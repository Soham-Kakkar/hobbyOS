#pragma once
#include <stdint.h>

#define ELF_MAGIC 0x464C457F

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint8_t  bits;
    uint8_t  endian;
    uint8_t  ident_version;
    uint8_t  abi;
    uint8_t  abi_version;
    uint8_t  pad[7];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint64_t entry;
    uint64_t phoff;
    uint64_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} Elf64_Ehdr;

typedef struct __attribute__((packed)) {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
} Elf64_Phdr;

/* =========================
   Constants
   ========================= */

#define PT_LOAD        1
#define PT_DYNAMIC     2

#define DT_NULL        0
#define DT_RELA        7
#define DT_RELASZ      8

#define R_X86_64_RELATIVE 8

#define ELF64_R_TYPE(i) ((uint32_t)(i))

#define PF_X 1   // Execute
#define PF_W 2   // Write
#define PF_R 4   // Read
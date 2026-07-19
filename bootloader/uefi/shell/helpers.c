#include "helpers.h"

int str_eq(const CHAR16* a, const CHAR16* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return (*a == 0 && *b == 0);
}

int split_cmd(CHAR16* line, CHAR16** cmd, CHAR16** args) {
    *cmd = line;
    *args = 0;

    while (*line) {
        if (*line == L' ') {
            *line = 0;
            *args = line + 1;
            return 1;
        }
        line++;
    }

    return 0;
}

EFI_FILE* open_path(EFI_FILE* root, CHAR16* path) {
    EFI_FILE* current = root;
    EFI_FILE* next;

    CHAR16* p = path;
    int first = 1;

    while (*p) {

        /* find next separator */
        CHAR16* start = p;
        while (*p && *p != L'/' && *p != L'\\') p++;

        CHAR16 saved = *p;
        *p = 0;

        EFI_STATUS status = current->Open(current, &next, start, EFI_FILE_MODE_READ, 0);

        *p = saved;

        if (status != EFI_SUCCESS) {
            if (!first) current->Close(current);
            return NULL;
        }

        // Close previous handle (except root)
        if (!first) {
            current->Close(current);
        }

        current = next;
        first = 0;

        if (*p) p++;
    }

    return current;
}
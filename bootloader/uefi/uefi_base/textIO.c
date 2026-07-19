#include "uefi_base.h"

/* ===============================
   Input
   =============================== */
static void flush_input(void) {
    EFI_INPUT_KEY key;

    while (gIn->ReadKeyStroke(gIn, &key) == EFI_SUCCESS)
        ;
}

int readline(CHAR16* buffer, int max) {
    flush_input();
    int pos = 0;

    while (1) {
        EFI_INPUT_KEY key;
        UINTN idx;

        gBS->WaitForEvent(1, &gIn->WaitForKey, &idx);
        gIn->ReadKeyStroke(gIn, &key);

        if (key.UnicodeChar == L'\r') {
            buffer[pos] = 0;
            Print(L"\r\n");
            return pos;
        }

        if (key.UnicodeChar == L'\b') {
            if (pos > 0) {
                pos--;
                Print(L"\b \b");
            }
            continue;
        }

        if (pos < max - 1) {
            buffer[pos++] = key.UnicodeChar;

            CHAR16 out[2];
            out[0] = key.UnicodeChar;
            out[1] = 0;

            Print(out);
        }
    }
}

/* ===============================
   Output
   =============================== */

INTN VSPrint(CHAR16 *str, UINTN size, CHAR16 *fmt, va_list args);

void Print(CHAR16 *str, ...) {
    CHAR16 buf[256];
    va_list args;

    va_start(args, str);
    VSPrint(buf, sizeof(buf), str, args);
    va_end(args);

    gOut->OutputString(gOut, buf);
}

#define putc(c) \
    if (count < size - 1) { \
        *p++ = (c); \
        count++; \
    }

INTN VSPrint(CHAR16 *str, UINTN size, CHAR16 *fmt, va_list args) {
    CHAR16 *p = str;
    UINTN count = 0;

    while (*fmt && count < size - 1) {
        if (*fmt == L'%') {
            fmt++;
            int long_flag = 0;

            if (*fmt == L'l') {
                long_flag = 1;
                fmt++;
            }

            switch (*fmt) {

            case L'd': {
                INT64 val = long_flag ? va_arg(args, INT64) : va_arg(args, INTN);
                CHAR16 tmp[32];
                INTN i = 0;
                int neg = 0;

                if (val < 0) {
                    neg = 1;
                    val = -val;
                }

                if (val == 0)
                    tmp[i++] = L'0';
                else
                    while (val > 0) {
                        tmp[i++] = L'0' + (val % 10);
                        val /= 10;
                    }

                if (neg)
                    putc(L'-');

                while (i > 0)
                    putc(tmp[--i]);

                break;
            }

            case L'u': {
                UINT64 val = long_flag ? va_arg(args, UINT64) : va_arg(args, UINTN);
                CHAR16 tmp[32];
                INTN i = 0;

                if (val == 0)
                    tmp[i++] = L'0';
                else
                    while (val > 0) {
                        tmp[i++] = L'0' + (val % 10);
                        val /= 10;
                    }

                while (i > 0)
                    putc(tmp[--i]);

                break;
            }

            case L'x':
            case L'X': {
                UINT64 val = long_flag ? va_arg(args, UINT64) : va_arg(args, UINTN);
                CHAR16 tmp[32];
                INTN i = 0;

                if (val == 0)
                    tmp[i++] = L'0';
                else
                    while (val > 0) {
                        UINTN digit = val % 16;

                        if (digit < 10)
                            tmp[i++] = L'0' + digit;
                        else
                            tmp[i++] = L'A' + digit - 10;

                        val /= 16;
                    }

                putc(L'0');
                putc(L'x');

                while (i > 0)
                    putc(tmp[--i]);

                break;
            }

            case L's': {
                CHAR16 *s = va_arg(args, CHAR16 *);
                if (!s) s = L"(null)";
                while (*s)
                    putc(*s++);
                break;
            }

            case L'c':
                putc((CHAR16)va_arg(args, INTN));
                break;

            case L'%':
                putc(L'%');
                break;

            default:
                putc(L'?');
                break;
            }

            fmt++;
        } else {
            putc(*fmt++);
        }
    }

    *p = L'\0';
    return count;
}

void clear(void) {
    gOut->ClearScreen(gOut);
}
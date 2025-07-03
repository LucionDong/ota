#include <stddef.h>
#include <stdio.h>

static char const hexdigits_lower[16] = "0123456789abcdef";

void uuid_to_string(const unsigned char *uuid, char *buf, char const *restrict fmt) {
    char *p = buf;

    for (int i = 0; i < 16; i++) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            *p++ = '-';
        }
        size_t tmp = uuid[i];
        *p++ = fmt[tmp >> 4];
        *p++ = fmt[tmp & 15];
    }
    *p = '\0';
}

void uuid_unparser(const unsigned char *uuid, char *buf) {
    uuid_to_string(uuid, buf, hexdigits_lower);
}

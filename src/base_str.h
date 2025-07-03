#ifndef BASE_TO_STR_H
#define BASE_TO_STR_H
#include <stddef.h>

int base64_decode(const char *input, unsigned char **output, size_t *out_len);
int base64_encode(const unsigned char *input, char **output, size_t in_len);
#endif

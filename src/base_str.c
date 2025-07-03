#include "base_str.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

int base64_decode(const char *input, unsigned char **output, size_t *out_len) {
    const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t input_len = strlen(input);
    int i, j;
    int padding = 0;

    // 计算输出缓冲区大小
    if (input_len % 4 != 0)
        return -1;
    *out_len = (input_len / 4) * 3;
    if (input[input_len - 1] == '=')
        padding++;
    if (input[input_len - 2] == '=')
        padding++;
    *out_len -= padding;

    // 分配内存
    *output = (unsigned char *) malloc(*out_len + 1);
    if (!*output)
        return -1;

    // 解码过程
    for (i = 0, j = 0; i < input_len;) {
        uint32_t sextet_a = input[i] == '=' ? 0 & i++ : strchr(base64_chars, input[i++]) - base64_chars;
        uint32_t sextet_b = input[i] == '=' ? 0 & i++ : strchr(base64_chars, input[i++]) - base64_chars;
        uint32_t sextet_c = input[i] == '=' ? 0 & i++ : strchr(base64_chars, input[i++]) - base64_chars;
        uint32_t sextet_d = input[i] == '=' ? 0 & i++ : strchr(base64_chars, input[i++]) - base64_chars;

        uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + sextet_d;

        if (j < *out_len)
            (*output)[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *out_len)
            (*output)[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *out_len)
            (*output)[j++] = triple & 0xFF;
    }
    (*output)[*out_len] = '\0';
    return 0;
}

int base64_encode(const unsigned char *input, char **output, size_t in_len) {
    if (input == NULL || output == NULL || in_len == 0) {
        LOG_ERROR("input is NULL");
        return -1;
    }
    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    size_t out_len = 4 * ((in_len + 2) / 3);
    *output = malloc(out_len + 1);
    if (!*output)
        return -1;

    int i = 0, j = 0;
    for (; i < in_len - 2; i += 3) {
        uint32_t triple = (input[i] << 16) | (input[i + 1] << 8) | input[i + 2];

        (*output)[j++] = base64_chars[(triple >> 18) & 0x3F];
        (*output)[j++] = base64_chars[(triple >> 12) & 0x3F];
        (*output)[j++] = base64_chars[(triple >> 6) & 0x3F];
        (*output)[j++] = base64_chars[triple & 0x3F];
    }

    // 处理剩余字节
    if (i < in_len) {
        uint32_t triple = input[i] << 16;
        if (i + 1 < in_len)
            triple |= input[i + 1] << 8;

        (*output)[j++] = base64_chars[(triple >> 18) & 0x3F];
        (*output)[j++] = base64_chars[(triple >> 12) & 0x3F];
        (*output)[j++] = (i + 1 < in_len) ? base64_chars[(triple >> 6) & 0x3F] : '=';
        (*output)[j++] = '=';
    }

    (*output)[out_len] = '\0';
    return 0;
}

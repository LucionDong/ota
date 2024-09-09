/*
 *
 * Copyright (C) 2023-12-08 11:21 dongbin <dongbin0625@163.com>
 *
 */
#include <stdio.h>

#include "utarray.h"

typedef struct char_c {
    char *buf;
} char_t;

UT_icd char_icd = {sizeof(char_t), NULL, NULL, NULL};

void test1() {
    UT_array *chars;
    utarray_new(chars, &char_icd);

    char_t p;
    p.buf = "hello";
    utarray_push_back(chars, &p);
    p.buf = "push";
    utarray_push_back(chars, &p);

    char_t *tmp = utarray_front(chars);

    printf("%s\n", tmp->buf);
}

UT_icd char_point_icd = {sizeof(char *), NULL, NULL, NULL};

void test() {
    UT_array *arr;
    utarray_new(arr, &char_point_icd);

    char *p = NULL;
    p = "what";
    utarray_push_back(arr, &p);
    p = "bug";
    utarray_push_back(arr, &p);

    char **t = utarray_front(arr);
    printf("%s\n", *t);
    // printf("%s\n", (char **) utarray_front(arr));
}

int main() {
    // test1();
    test();

    //     UT_array *arr;
    // utarray_new(arr, &ut_int_icd);
    //
    // int p = 1;
    // utarray_push_back(arr, &p);
    // p = 2;
    // utarray_push_back(arr, &p);
    //
    //     printf("%d\n", *(int *) utarray_front(arr));
}

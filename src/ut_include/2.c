/*
 *
 * Copyright (C) 2023-12-08 10:13 dongbin <dongbin0625@163.com>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utarray.h"

void test(UT_array **strs) {
    char *buf = "what";
    utarray_push_back(*strs, &buf);
    buf = "but";
    utarray_push_back(*strs, &buf);
}

void test1() {
    char *msg = "12315-";

    char *tmp = strstr(msg, "5");
    int a = tmp - msg;
    printf("%s\n", tmp);
    printf("%d\n", a);
    strncpy(tmp, msg, tmp - msg);
    tmp[a] = '\0';
    printf("%d\n", atoi(&(msg[0])));
    printf("%s\n", tmp);
}

int main(int argc, char *argv[]) {
    // test1();

    UT_array *strs;
    utarray_new(strs, &ut_str_icd);

    test(&strs);

    //     char *buf = "what";
    //
    // utarray_push_back(strs, &buf);
    // // test0(&strs, &buf);
    // buf = "but";
    //     utarray_push_back(strs, &buf);
    // test0(&strs, &buf);

    //     char **p;
    // p = NULL;
    // while ((p = (char **) utarray_next(strs, p))) {
    //     printf("%s\n", *p);
    //     }

    // printf("%s\n", (char *) utarray_front(strs));
    // utarray_front(strs);

    // utarray_free(strs);
    utarray_clear(strs);

    test(&strs);
    char **p;
    p = NULL;
    while ((p = (char **) utarray_next(strs, p))) {
        printf("%s\n", *p);
    }

    return 0;
}

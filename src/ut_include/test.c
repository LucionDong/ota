/*
 *
 * Copyright (C) 2023-11-28 14:07 dongbin <dongbin0625@163.com>
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include "uthash.h"

typedef enum test_type { WHAT, SHIOT } test_type_e;

typedef struct test_hash_struct {
    test_type_e type;
    // int name;
    char *name;
    UT_hash_handle hh;
} test_hash_struct_t;

void add_hsahtable(test_hash_struct_t **test_hash_table, test_type_e test_type, char *name) {
    test_hash_struct_t *hash_member;
    HASH_FIND_INT(*test_hash_table, &test_type, hash_member);

    if (!hash_member) {
        // hash_member = malloc(sizeof *hash_member);
        hash_member = calloc(1, sizeof(test_hash_struct_t));
        hash_member->type = test_type;
        HASH_ADD_INT(*test_hash_table, type, hash_member);
    }
    if (hash_member->name)
        free(hash_member->name);
    // strcpy(hash_member->name, name);
    hash_member->name = strdup(name);
}

test_hash_struct_t *find_hash_table(test_hash_struct_t **test_hash_table, test_type_e test_type) {
    test_hash_struct_t *target_hash_member;
    target_hash_member = malloc(sizeof *target_hash_member);
    // target_hash_member = calloc(1, sizeof(test_hash_struct_t));
    HASH_FIND_INT(*test_hash_table, &test_type, target_hash_member);
    printf("----------\n");
    // printf("test_hash_table name is %s\n", target_hash_member->name);
    return target_hash_member;
}

void print_test_hash_table(test_hash_struct_t *test_hash_table) {
    test_hash_struct_t *tmp;
    for (tmp = test_hash_table; tmp != NULL; tmp = tmp->hh.next) {
        printf("%d %s\n", tmp->type, tmp->name);
    }
}

int main(int argc, char *argv[]) {
    test_hash_struct_t *test_hash_table = NULL;
    add_hsahtable(&test_hash_table, WHAT, "what");
    add_hsahtable(&test_hash_table, SHIOT, "shit");
    add_hsahtable(&test_hash_table, WHAT, "buyt");

    char id[10] = "1";

    print_test_hash_table(test_hash_table);

    test_hash_struct_t *target_hash_member = find_hash_table(&test_hash_table, WHAT);
    if (target_hash_member == NULL) {
        printf("NULL\n");
        return 0;
    }
    printf("target_hash_member is %s\n", target_hash_member->name);
    return 0;
}

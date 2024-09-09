/*
 *
 * Copyright (C) 2024-04-25 11:29 dongbin <dongbin0625@163.com>
 *
 */
#include <archive.h>
#include <archive_entry.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void archive_test(const char *path) {
    struct archive *a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    int ret = archive_read_open_filename(a, path, 10240);
    if (ret != ARCHIVE_OK) {
        perror("error opening archive");
        return;
    }

    struct archive_entry *entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char *path_name = archive_entry_pathname(entry);
        printf("path_name: %s\n", path_name);

        ret = archive_read_extract(a, entry, ARCHIVE_EXTRACT_TIME);
        if (ret != ARCHIVE_OK) {
            perror("error extract");
        }
    }

    archive_read_close(a);
    archive_read_free(a);
}

void test0() {
    FILE *fp = NULL;
    char str[1024] = {0};

    fp = popen("./test.sh", "r");
    if (fp == NULL) {
        perror("脚本错误");
        return;
    }

    while (fgets(str, sizeof(str) - 1, fp) != NULL) {
        printf("%s", str);
    }

    pclose(fp);
}

void test1() {
    int ret_value = system("./test.sh");

    if (WIFEXITED(ret_value)) {
        int exit_status = WEXITSTATUS(ret_value);
        printf("status: %d\n", exit_status);
    } else {
        printf("no\n");
    }
}

void test2() {
    while (1) {
    }
}

void test3() {
    char address_tmp[] = "http://speedtest.tele2.net/100MB.zip";
    // char *address = strdup(address_tmp);
    char *name = "MY_VAR";
    setenv(name, address_tmp, 1);

    system("wget_install.sh");
    // free(address);
}

void test4() {
    json_error_t error;
    json_t *root = json_object();
}

int main() {
    archive_test("./data.tar.gz");
    return 0;
}

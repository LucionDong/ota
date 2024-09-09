#include <curl/curl.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "openssl/md5.h"
#include "openssl/sha.h"

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

void compute_md5(FILE *file, unsigned char *result) {
    MD5_CTX md5_ctx;
    unsigned char buffer[1024];
    int bytes_read;

    // 初始化 MD5 上下文
    MD5_Init(&md5_ctx);

    // 读取文件并更新 MD5
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) != 0) {
        MD5_Update(&md5_ctx, buffer, bytes_read);
    }

    // 完成 MD5 计算
    MD5_Final(result, &md5_ctx);

    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        LOG_INFO("%02x", result[i]);
    }
}

void md5_to_string(const unsigned char *md5_result, char *md5_string) {
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(md5_string + (i * 2), "%02x", md5_result[i]);
    }
    md5_string[MD5_DIGEST_LENGTH * 2] = '\0';  // 添加字符串结束符
}

int wget_install(const char *url, const char *recv_md5_string) {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    char *outfilename = "ota.tar.gz", md5_string[MD5_DIGEST_LENGTH * 2 + 1];
    unsigned char md5_res[MD5_DIGEST_LENGTH];
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(outfilename, "rb");
        if (fp == NULL) {
            perror("fopen");
            return -1;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            perror("curl_easy_perform");
        }
        curl_easy_cleanup(curl);
        compute_md5(fp, md5_res);
        md5_to_string(md5_res, md5_string);
        fclose(fp);
    }
    // printf("download over\n");
    LOG_INFO("md5_string: %s", md5_string);

    if (strcmp(recv_md5_string, md5_string)) {
        LOG_WARN("check md5 error");
        return -1;
    }

    return 0;
}

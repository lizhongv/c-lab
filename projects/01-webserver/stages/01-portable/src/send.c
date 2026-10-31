/** @file send.c @brief 200 响应与文件内容发送。 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "webserver.h"

int send_file(const char *uri, clab_socket_t sock) {
    const char *type = content_type_for(uri);
    if (type == NULL) {
        printf("无法识别的文件类型：%s\n", uri);
        return CLAB_ERR;
    }

    FILE *file = fopen(uri, "rb");
    if (file == NULL) {
        printf("打开文件失败：%s\n", uri);
        return CLAB_ERR;
    }

    struct stat info;
    if (stat(uri, &info) != 0) {
        (void)fclose(file);
        return CLAB_ERR;
    }

    char length[32];
    (void)snprintf(length, sizeof length, "%lld", (long long)info.st_size);

    const char *status = "HTTP/1.1 200 OK\r\n";
    const char *end = "\r\n";

    if (send_all(sock, status, strlen(status)) != CLAB_OK ||
        send_all(sock, SERVER_NAME, strlen(SERVER_NAME)) != CLAB_OK) {
        (void)fclose(file);
        return CLAB_ERR;
    }

    /* 第 02 章把 Data 改为 Date，并把 ctime 换成 IMF-fixdate。 */
    if (send_all(sock, "Data: ", 6) != CLAB_OK) {
        (void)fclose(file);
        return CLAB_ERR;
    }
    const char *date = http_date();
    if (send_all(sock, date, strlen(date)) != CLAB_OK ||
        send_all(sock, type, strlen(type)) != CLAB_OK ||
        send_all(sock, "Content-Length: ", 16) != CLAB_OK ||
        send_all(sock, length, strlen(length)) != CLAB_OK ||
        /* 保留第 00 章的换行缺陷：这里发的是 LF，不是 CRLF（第 02 章修）。 */
        send_all(sock, "\n", 1) != CLAB_OK ||
        send_all(sock, end, strlen(end)) != CLAB_OK) {
        (void)fclose(file);
        return CLAB_ERR;
    }

    char buffer[BUF_SIZE];
    size_t remaining = (size_t)info.st_size;
    while (remaining > 0) {
        size_t chunk = remaining < sizeof buffer ? remaining : sizeof buffer;
        size_t got = fread(buffer, 1, chunk, file);
        if (got == 0) {
            break;
        }
        if (send_all(sock, buffer, got) != CLAB_OK) {
            printf("发送文件内容失败\n");
            /* 保留第 00 章的缺陷：失败后不递减 remaining，继续空转到文件末尾（第 05 章修）。 */
            continue;
        }
        remaining -= got;
    }

    (void)fclose(file);
    return CLAB_OK;
}

/** @file other.c @brief 发送封装、时间、URI 检查等公共工具。 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "webserver.h"

int send_all(clab_socket_t sock, const void *buf, size_t len) {
    const char *data = (const char *)buf;
    size_t total = 0;

    while (total < len) {
        long sent = platform_socket_send(sock, data + total, len - total);
        if (sent < 0) {
            return CLAB_ERR;
        }
        /* 保留第 00 章的缺陷：send 返回 0 时 total 不前进，会死循环（第 05 章修）。 */
        total += (size_t)sent;
    }
    return CLAB_OK;
}

const char *http_date(void) {
    time_t now;
    time(&now);
    const char *text = ctime(&now);
    return text != NULL ? text : "";
}

void post_value(const char *message) {
    const char *suffix = strrchr(message, '\n');
    if (suffix != NULL) {
        suffix++;
    } else {
        suffix = message;
    }
    printf("\nPost Value: %s\n", suffix);
}

int judge_uri(const char *uri, clab_socket_t sock) {
    struct stat info;
    if (stat(uri, &info) == -1) {
        (void)file_not_found(sock);
        return CLAB_ERR;
    }
    return CLAB_OK;
}

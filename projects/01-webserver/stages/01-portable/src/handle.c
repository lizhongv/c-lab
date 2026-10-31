/** @file handle.c @brief 请求行解析与分发。 */

#include <stdio.h>
#include <string.h>

#include "webserver.h"

static http_method_t parse_method(const char *method) {
    if (strcmp(method, "GET") == 0) {
        return HTTP_GET;
    }
    if (strcmp(method, "POST") == 0) {
        return HTTP_POST;
    }
    return HTTP_UNSUPPORTED;
}

int handle_request(char *message, clab_socket_t sock) {
    char method[16];
    char uri[BUF_SIZE];
    char version[16];

    /* 宽度限定为缓冲区大小减 1：第 00 章的 %s 没有上限。 */
    if (sscanf(message, "%15s %1023s %15s", method, uri, version) != 3) {
        printf("请求行格式错误\n");
        return CLAB_ERR;
    }

    http_method_t parsed = parse_method(method);
    if (parsed == HTTP_UNSUPPORTED) {
        /* 只有这里发 501；第 00 章的 Judge_Method 既判定又发响应，还被调用了两次。 */
        (void)send_method_not_implemented(sock);
        return CLAB_ERR;
    }

    if (parsed == HTTP_POST) {
        post_value(message);
    }

    /* 请求目标是 origin-form（以 / 开头），文件路径是相对工作目录的；
       去掉一个前导斜杠，否则 fopen("/hello.txt") 会去找当前盘符的根目录。
       完整的 URL 解码、查询串剥离与根目录约束在第 02、03 章处理。 */
    const char *path = uri;
    if (path[0] == '/') {
        path++;
    }

    if (judge_uri(path, sock) != CLAB_OK) {
        return CLAB_ERR;
    }
    return send_file(path, sock);
}

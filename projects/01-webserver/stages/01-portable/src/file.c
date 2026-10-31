/** @file file.c @brief 404 响应与扩展名到 Content-Type 的映射。 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "webserver.h"

int file_not_found(clab_socket_t sock) {
    const char *status = "HTTP/1.1 404 Not Found\r\n";
    const char *type = "Content-type: text/plain\r\n";
    const char *length = "Content-Length: 42\r\n";
    const char *end = "\r\n";
    const char *body = "The file which is requested is not found!\n";

    printf("请求的文件不存在\n");

    if (send_all(sock, status, strlen(status)) != CLAB_OK ||
        send_all(sock, SERVER_NAME, strlen(SERVER_NAME)) != CLAB_OK) {
        return CLAB_ERR;
    }

    if (send_all(sock, "Data: ", 6) != CLAB_OK) {
        return CLAB_ERR;
    }
    const char *date = http_date();
    if (send_all(sock, date, strlen(date)) != CLAB_OK ||
        send_all(sock, type, strlen(type)) != CLAB_OK ||
        send_all(sock, length, strlen(length)) != CLAB_OK ||
        send_all(sock, end, strlen(end)) != CLAB_OK ||
        send_all(sock, body, strlen(body)) != CLAB_OK) {
        return CLAB_ERR;
    }
    return CLAB_OK;
}

const char *content_type_for(const char *uri) {
    const char *suffix = strrchr(uri, '.');
    if (suffix != NULL) {
        suffix++;
    }

    /* 保留第 00 章的缺陷：无扩展名时 suffix 为 NULL，strcmp 崩溃（第 02 章修）。 */
    if (strcmp(suffix, "html") == 0) {
        return "Content-type: text/html\r\n";
    }
    if (strcmp(suffix, "jpg") == 0) {
        return "Content-type: image/jpg\r\n";
    }
    if (strcmp(suffix, "png") == 0) {
        return "Content-type: image/png\r\n";
    }
    if (strcmp(suffix, "gif") == 0) {
        return "Content-type: image/gif\r\n";
    }
    if (strcmp(suffix, "txt") == 0) {
        return "Content-type: text/plain\r\n";
    }
    if (strcmp(suffix, "xml") == 0) {
        return "Content-type: text/xml\r\n";
    }
    if (strcmp(suffix, "rtf") == 0) {
        return "Content-type: text/rtf\r\n";
    }
    return NULL;
}

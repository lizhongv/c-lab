/** @file method.c @brief 501 响应。 */

#include <stdio.h>
#include <string.h>

#include "webserver.h"

int send_method_not_implemented(clab_socket_t sock) {
    const char *status = "HTTP/1.1 501 Not Implemented\r\n";
    const char *type = "Content-type: text/plain\r\n";
    const char *length = "Content-Length: 41\r\n";
    const char *end = "\r\n";
    const char *body = "The request method is not yet completed!\n";

    printf("请求方法未实现\n");

    if (send_all(sock, status, strlen(status)) != CLAB_OK ||
        send_all(sock, SERVER_NAME, strlen(SERVER_NAME)) != CLAB_OK) {
        return CLAB_ERR;
    }

    /* 第 02 章把 Data 改为 Date，并把 ctime 换成 IMF-fixdate。 */
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

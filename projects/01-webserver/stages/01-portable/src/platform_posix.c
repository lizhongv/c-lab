/** @file platform_posix.c @brief POSIX（Linux / macOS）实现。 */

#define _POSIX_C_SOURCE 200809L

#include "platform.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

int platform_init(void) {
    /* 对端关闭后继续 send 会触发 SIGPIPE，默认动作是终止进程；服务器应自己处理错误码。 */
    (void)signal(SIGPIPE, SIG_IGN);
    return 0;
}

void platform_cleanup(void) { }

clab_socket_t platform_socket_create(void) {
    return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

int platform_socket_bind(clab_socket_t sock, const char *ip, unsigned short port) {
    struct sockaddr_in addr;
    (void)memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {
        return -1;
    }
    {
        int reuse = 1;
        (void)setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, (socklen_t)sizeof reuse);
    }
    return bind(sock, (const struct sockaddr *)&addr, (socklen_t)sizeof addr) == 0 ? 0 : -1;
}

int platform_socket_listen(clab_socket_t sock, int backlog) {
    return listen(sock, backlog) == 0 ? 0 : -1;
}

clab_socket_t platform_socket_accept(clab_socket_t sock, char *peer, size_t peer_size, unsigned short *peer_port) {
    struct sockaddr_in addr;
    socklen_t len = (socklen_t)sizeof addr;
    clab_socket_t client = accept(sock, (struct sockaddr *)&addr, &len);
    if (client == CLAB_INVALID_SOCKET) {
        return CLAB_INVALID_SOCKET;
    }
    if (peer != NULL && peer_size > 0) {
        (void)inet_ntop(AF_INET, &addr.sin_addr, peer, (socklen_t)peer_size);
    }
    if (peer_port != NULL) {
        *peer_port = ntohs(addr.sin_port);
    }
    return client;
}

long platform_socket_recv(clab_socket_t sock, void *buf, size_t len) {
    ssize_t got;
    do {
        got = recv(sock, buf, len, 0);
    } while (got < 0 && errno == EINTR);
    return (long)got;
}

long platform_socket_send(clab_socket_t sock, const void *buf, size_t len) {
    ssize_t sent;
    do {
        sent = send(sock, buf, len, MSG_NOSIGNAL);
    } while (sent < 0 && errno == EINTR);
    return (long)sent;
}

void platform_socket_close(clab_socket_t sock) { (void)close(sock); }

int platform_socket_local_port(clab_socket_t sock, unsigned short *port) {
    struct sockaddr_in addr;
    socklen_t len = (socklen_t)sizeof addr;
    if (getsockname(sock, (struct sockaddr *)&addr, &len) != 0) {
        return -1;
    }
    *port = ntohs(addr.sin_port);
    return 0;
}

const char *platform_last_error(void) { return strerror(errno); }

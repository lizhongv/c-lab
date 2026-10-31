/** @file platform_win32.c @brief WinSock 实现。 */

#include "platform.h"

#include <stdio.h>
#include <string.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0 /* Winsock 没有 SIGPIPE，占位以保持调用点一致 */
#endif

int platform_init(void) {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0 ? 0 : -1;
}

void platform_cleanup(void) { (void)WSACleanup(); }

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
        (void)setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, (int)sizeof reuse);
    }
    return bind(sock, (const struct sockaddr *)&addr, (int)sizeof addr) == 0 ? 0 : -1;
}

int platform_socket_listen(clab_socket_t sock, int backlog) {
    return listen(sock, backlog) == 0 ? 0 : -1;
}

clab_socket_t platform_socket_accept(clab_socket_t sock, char *peer, size_t peer_size, unsigned short *peer_port) {
    struct sockaddr_in addr;
    int len = (int)sizeof addr;
    clab_socket_t client = accept(sock, (struct sockaddr *)&addr, &len);
    if (client == CLAB_INVALID_SOCKET) {
        return CLAB_INVALID_SOCKET;
    }
    if (peer != NULL && peer_size > 0) {
        (void)inet_ntop(AF_INET, &addr.sin_addr, peer, peer_size);
    }
    if (peer_port != NULL) {
        *peer_port = ntohs(addr.sin_port);
    }
    return client;
}

long platform_socket_recv(clab_socket_t sock, void *buf, size_t len) {
    int got = recv(sock, (char *)buf, (int)len, 0);
    return (long)got;
}

long platform_socket_send(clab_socket_t sock, const void *buf, size_t len) {
    int sent = send(sock, (const char *)buf, (int)len, MSG_NOSIGNAL);
    return (long)sent;
}

void platform_socket_close(clab_socket_t sock) { (void)closesocket(sock); }

int platform_socket_local_port(clab_socket_t sock, unsigned short *port) {
    struct sockaddr_in addr;
    int len = (int)sizeof addr;
    if (getsockname(sock, (struct sockaddr *)&addr, &len) != 0) {
        return -1;
    }
    *port = ntohs(addr.sin_port);
    return 0;
}

const char *platform_last_error(void) {
    static char message[64];
    (void)snprintf(message, sizeof message, "WSA error %d", WSAGetLastError());
    return message;
}

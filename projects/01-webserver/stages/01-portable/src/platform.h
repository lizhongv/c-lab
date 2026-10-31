/**
 * @file platform.h
 * @brief 平台抽象层：把 WinSock 与 POSIX socket 的差异收在这一个头文件后面。
 *
 * 约定：
 *  - 所有函数返回 0 / CLAB_OK 表示成功，负数或 CLAB_INVALID_SOCKET 表示失败。
 *  - platform_last_error() 返回最近一次失败的静态描述，不保证线程安全。
 *  - 接收与发送返回「本次实际字节数」，失败返回负数；调用方自行处理部分收发。
 */

#ifndef CLAB_PLATFORM_H
#define CLAB_PLATFORM_H

#include <stddef.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET clab_socket_t;
#define CLAB_INVALID_SOCKET INVALID_SOCKET
#else
typedef int clab_socket_t;
#define CLAB_INVALID_SOCKET (-1)
#endif

/** 启动网络库（Windows 的 WSAStartup；POSIX 忽略 SIGPIPE）。成功返回 0。 */
int platform_init(void);

/** 释放网络库资源。 */
void platform_cleanup(void);

/** 创建 TCP 套接字，失败返回 CLAB_INVALID_SOCKET。 */
clab_socket_t platform_socket_create(void);

/** 绑定地址与端口；端口为 0 时由系统分配。成功返回 0。 */
int platform_socket_bind(clab_socket_t sock, const char *ip, unsigned short port);

/** 开始监听。成功返回 0。 */
int platform_socket_listen(clab_socket_t sock, int backlog);

/**
 * 接受一个连接。
 * @param peer      输出参数：对端 IP 文本，至少 46 字节。
 * @param peer_size peer 缓冲区大小。
 * @param peer_port 输出参数：对端端口，可为 NULL。
 */
clab_socket_t platform_socket_accept(clab_socket_t sock, char *peer, size_t peer_size, unsigned short *peer_port);

/** 接收数据，返回字节数；连接关闭返回 0，失败返回负数。 */
long platform_socket_recv(clab_socket_t sock, void *buf, size_t len);

/** 发送数据，返回字节数；失败返回负数。 */
long platform_socket_send(clab_socket_t sock, const void *buf, size_t len);

/** 关闭套接字。 */
void platform_socket_close(clab_socket_t sock);

/** 查询套接字本地端口（用于端口传 0 时打印实际端口）。 */
int platform_socket_local_port(clab_socket_t sock, unsigned short *port);

/** 最近一次错误的文本描述。 */
const char *platform_last_error(void);

#endif /* CLAB_PLATFORM_H */

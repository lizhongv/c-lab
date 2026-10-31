/**
 * @file webserver.h
 * @brief 第 01 章的服务器常量与模块接口。
 *
 * 本章只解决「跨平台可构建」：行为与第 00 章保持一致的地方刻意保留，
 * 路径穿越、无扩展名崩溃、单线程阻塞等问题留到后续章节。
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <stddef.h>

#include "platform.h"

#define SERVER_IP_ADDR "127.0.0.1"
#define SERVER_PORT 8080
#define BACKLOG 10
#define BUF_SIZE 1024
#define CLAB_OK 0
#define CLAB_ERR (-1)

#define SERVER_NAME "Server: c-lab webserver/0.1\r\n"

typedef enum http_method {
    HTTP_GET,
    HTTP_POST,
    HTTP_UNSUPPORTED
} http_method_t;

/** 解析请求行并处理一次请求。 */
int handle_request(char *message, clab_socket_t sock);

/** 发完 len 字节才算成功；部分发送由内部循环处理。 */
int send_all(clab_socket_t sock, const void *buf, size_t len);

/** 501：方法不支持。 */
int send_method_not_implemented(clab_socket_t sock);

/** 文件存在返回 CLAB_OK，否则发送 404 并返回 CLAB_ERR。 */
int judge_uri(const char *uri, clab_socket_t sock);

/** 404：文件不存在。 */
int file_not_found(clab_socket_t sock);

/** 200：发送文件；类型未知或打开失败返回 CLAB_ERR。 */
int send_file(const char *uri, clab_socket_t sock);

/** 扩展名到 Content-Type；未知或无扩展名返回 NULL（本章保留该缺陷）。 */
const char *content_type_for(const char *uri);

/** ctime 风格的当前时间，末尾带换行（第 02 章改为 IMF-fixdate）。 */
const char *http_date(void);

/** 打印 POST 请求体最后一行（本章不解析表单）。 */
void post_value(const char *message);

#endif /* WEBSERVER_H */

/** @file main.c @brief 程序入口：解析参数、初始化平台层、单线程 accept 循环。 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clab/log.h"
#include "webserver.h"

static int parse_port(const char *text, unsigned short *port) {
    char *end = NULL;
    long value = strtol(text, &end, 10);
    if (end == text || *end != '\0' || value < 0 || value > 65535) {
        return CLAB_ERR;
    }
    *port = (unsigned short)value;
    return CLAB_OK;
}

static void usage(const char *program) {
    printf("用法：%s [--port N]\n", program);
    printf("  --port N  监听端口，0 表示由系统分配（默认 %d）\n", SERVER_PORT);
}

int main(int argc, char **argv) {
    unsigned short port = SERVER_PORT;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            if (parse_port(argv[++i], &port) != CLAB_OK) {
                fprintf(stderr, "--port 需要 0～65535 之间的整数\n");
                return 2;
            }
        } else if (strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "未知参数：%s\n", argv[i]);
            usage(argv[0]);
            return 2;
        }
    }

    if (platform_init() != 0) {
        fprintf(stderr, "网络库初始化失败：%s\n", platform_last_error());
        return 1;
    }

    clab_socket_t server = platform_socket_create();
    if (server == CLAB_INVALID_SOCKET) {
        fprintf(stderr, "创建套接字失败：%s\n", platform_last_error());
        platform_cleanup();
        return 1;
    }

    if (platform_socket_bind(server, SERVER_IP_ADDR, port) != 0) {
        fprintf(stderr, "绑定 %s:%u 失败：%s\n", SERVER_IP_ADDR, (unsigned)port, platform_last_error());
        platform_socket_close(server);
        platform_cleanup();
        return 1;
    }

    if (platform_socket_listen(server, BACKLOG) != 0) {
        fprintf(stderr, "监听失败：%s\n", platform_last_error());
        platform_socket_close(server);
        platform_cleanup();
        return 1;
    }

    unsigned short actual = 0;
    (void)platform_socket_local_port(server, &actual);
    /* 测试脚本依赖这一行拿到实际端口，必须刷新并保持格式稳定。 */
    printf("listening on %s:%u\n", SERVER_IP_ADDR, (unsigned)actual);
    fflush(stdout);

    clab_log_set_level(CLAB_LOG_INFO);
    CLAB_INFO("webserver 第 01 章启动，工作目录即文件根目录");

    for (;;) {
        char peer[64];
        unsigned short peer_port = 0;
        clab_socket_t client = platform_socket_accept(server, peer, sizeof peer, &peer_port);
        if (client == CLAB_INVALID_SOCKET) {
            CLAB_WARN("accept 失败：%s", platform_last_error());
            continue;
        }

        CLAB_INFO("来自 %s:%u", peer, (unsigned)peer_port);

        char request[BUF_SIZE];
        long got = platform_socket_recv(client, request, sizeof request - 1);
        if (got <= 0) {
            CLAB_WARN("接收失败或连接直接关闭");
            platform_socket_close(client);
            continue;
        }
        /* 第 00 章的越界写在这里修正：先判返回值，再留出 1 字节给终止符。 */
        request[got] = '\0';
        CLAB_INFO("请求：%s", request);

        (void)handle_request(request, client);
        platform_socket_close(client);
    }
}

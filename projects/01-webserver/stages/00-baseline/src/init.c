#include "webserver.h"

int Server_Socket_Init()
{
    //初始化和构造套接字
    WORD wVersionrequested;
    WSADATA wsaData;
    SOCKET ServerSock;
    struct sockaddr_in ServerAddr;
    int rval;

    /* 加载Winsock */
    wVersionrequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionrequested, &wsaData) != 0)
    {
        printf("Failed to load Winsock!\n");
        system("pause");
        return -1;
    }
    printf("Succeed to load Winsock!\n");

    /* 创建套接字 */
    ServerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ServerSock == INVALID_SOCKET)
    {
        printf("Failed to create socket!\n");
        system("pause");
        exit(1);
    }
    printf("Succeed to create socket!\n");

    /* 配置服务器IP、端口信息 */
    memset(&ServerAddr, 0, sizeof(struct sockaddr));	//每一个字节都用0来填充
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(SERVER_PORT);
    ServerAddr.sin_addr.s_addr = inet_addr(SERVER_IP_ADDR);

    /* 绑定 */
    rval = bind(ServerSock, (SOCKADDR*)&ServerAddr, sizeof(struct sockaddr));
    if (rval == SOCKET_ERROR)
    {
        printf("Failed to bind stream socket!\n");
        system("pause");
        exit(1);
    }
    printf("Succeed to bind stream socket!\n");

    return ServerSock;
}

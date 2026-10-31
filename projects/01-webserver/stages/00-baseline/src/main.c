#include "webserver.h"

const char* Server_name = "Server: Web Server 1.0 - BooLo\r\n";
//Web服务器信息


int Logo()
{
    //Web服务器标志信息
    printf("___________________________________________________________\n");
    printf("  __          ________ _______\n");
    printf("  \\ \\        / /  ____|  _____\\\n");
    printf("   \\ \\  /\\  / /| |____  |____) )\n");
    printf("    \\ \\/  \\/ / |  ____|  ____(   __  __     __ ___\n");
    printf("     \\  /\\  /  | |____  |____) )(__ |_ \\  /|_ |___)\n");
    printf("      \\/  \\/   |______|_______/  __)|__ \\/ |__|   \\\n");
    printf("\n");
    printf("            Welcome to use the Web Server!\n");
    printf("                     Version 1.0\n\n");
    printf("                        lizhong\n");
    printf("___________________________________________________________\n\n");

    return OK;
}

int main()
{
    SOCKET ServerSock, MessageSock;
    struct sockaddr_in ClientAddr;
    int rval, Length;
    char revbuf[BUF_SIZE];

    // TODO (Administrator#1#): later modify
    Logo();
    printf("Web Server 1.0 is starting......\n\n");
    ServerSock = Server_Socket_Init();
    printf("\n-----------------------------------------------------------\n");

    while (OK)
    {
        /* 启动监听 */
        rval = listen(ServerSock, BACKLOG);
        if (rval == SOCKET_ERROR)
        {
            printf("Failed to listen socket!\n");
            system("pause");
            exit(1);
        }
        printf("Listening the socket ......\n");

        /* 接受客户端请求建立连接 */
        Length = sizeof(struct sockaddr);
        MessageSock = accept(ServerSock, (SOCKADDR*)&ClientAddr, &Length);
        if (MessageSock == INVALID_SOCKET)
        {
            printf("Failed to accept connection from client!\n");
            system("pause");
            exit(1);
        }
        printf("Succeed to accept connection from [%s:%d] !\n\n", inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port));

        /* 接收客户端请求数据 */
        memset(revbuf, 0, BUF_SIZE);	//每一个字节都用0来填充
        rval = recv(MessageSock, revbuf, BUF_SIZE, 0);
        revbuf[rval] = 0x00;
        if (rval <= 0)
            printf("Failed to receive request message from client!\n");
        else
        {
            printf("%s\n", revbuf);	//输出请求数据内容
            rval = Handle_Request_Message(revbuf, MessageSock);
        }

        closesocket(MessageSock);
        printf("\n-----------------------------------------------------------\n");
    }

    closesocket(ServerSock);	//关闭套接字
    WSACleanup();	//停止Winsock

    return OK;
}

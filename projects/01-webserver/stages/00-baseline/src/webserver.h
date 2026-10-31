#ifndef WEBSERVER_H_INCLUDED
#define WEBSERVER_H_INCLUDED

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNNINGS

#define SERVER_IP_ADDR "127.0.0.1"	//服务器IP地址
#define SERVER_PORT 80				//服务器端口号
#define BACKLOG 10
#define BUF_SIZE 1024
#define OK 1
#define ERROR 0

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")

// 注意  命令行添加参数 -lwsock32

int Server_Socket_Init();
int Handle_Request_Message(char* message, int Socket);
int Judge_URI(char* URI, int Socket);
int Send_Ifon(int Socket, const char* sendbuf, int Length);
int Error_Request_Method(int Socket);
int Inquire_File(char* URI);
int File_not_Inquire(int Socket);
int Send_File(char* URI, int Socket);
int Logo();
const char* Judge_Method(char* method, int Socket);
const char* Judge_File_Type(char* URI, const char* content_type);
const char* Get_Data(const char* cur_time);
const char* Post_Value(char* message);

#endif // WEBSERVER_H_INCLUDED

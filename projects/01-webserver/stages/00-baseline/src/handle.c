#include "webserver.h"

int Handle_Request_Message(char* message, int Socket)
{
    //处理HTTP请求报文信息
    int rval = 0;
    char Method[BUF_SIZE];
    char URI[BUF_SIZE];
    char Version[BUF_SIZE];

    if (sscanf(message, "%s %s %s", Method, URI, Version) != 3)
    {
        printf("Request line error!\n");
        return ERROR;
    }	//提取"请求方法"、"URL"、"HTTP版本"三个关键要素

    if (Judge_Method(Method, Socket) == ERROR)
    {
        return ERROR;
    }
    else if(Judge_Method(Method, Socket) == "POST")
    {
        Post_Value(message);
    }	//判断处理"请求方法"

    if (Judge_URI(URI, Socket) == ERROR)
    {
        return ERROR;
    }	//判断处理"URI"
    else
        rval = Send_File(URI, Socket);

    if (rval == OK)
    {
        printf("The process is successfully finished!\n");
    }

    return OK;
}

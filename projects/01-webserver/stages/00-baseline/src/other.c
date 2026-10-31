#include "webserver.h"


int Send_Ifon(int Socket, const char* sendbuf, int Length)
{
    //发送信息到客户端
    int sendtotal = 0, bufleft, rval = 0;

    bufleft = Length;
    while (sendtotal < Length)
    {
        rval = send(Socket, sendbuf + sendtotal, bufleft, 0);
        if (rval < 0)
        {
            break;
        }
        sendtotal += rval;
        bufleft -= rval;
    }

    Length = sendtotal;

    return rval < 0 ? ERROR : OK;
}


const char* Get_Data(const char* cur_time)
{
    //获取Web服务器的当前时间作为响应时间
    time_t curtime;
    time(&curtime);
    cur_time = ctime(&curtime);

    return cur_time;
}

const char* Post_Value(char* message)
{
    //获取客户端POST请求方式的值
    const char* suffix;

    if ((suffix = strrchr(message, '\n')) != NULL)
        suffix = suffix + 1;
    printf("\n\nPost Value: %s\n\n", suffix);

    return suffix;
}

int Judge_URI(char* URI, int Socket)
{
    //判断请求URI
    if (Inquire_File(URI) == ERROR)
    {
        File_not_Inquire(Socket);
        return ERROR;
    }
    else
        return OK;
}

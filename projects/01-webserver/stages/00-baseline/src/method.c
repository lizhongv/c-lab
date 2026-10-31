#include "webserver.h"

extern const char *Server_name;

int Error_Request_Method(int Socket)
{
    //501 Not Implemented响应
    const char* Method_err_line = "HTTP/1.1 501 Not Implemented\r\n";
    const char* cur_time = "";
    const char* Method_err_type = "Content-type: text/plain\r\n";
    const char* File_err_length = "Content-Length: 41\r\n";
    const char* Method_err_end = "\r\n";
    const char* Method_err_info = "The request method is not yet completed!\n";

    printf("The request method from client's request message is not yet completed!\n");

    if (Send_Ifon(Socket, Method_err_line, strlen(Method_err_line)) == ERROR)
    {
        printf("Sending method_error_line failed!\n");
        return ERROR;
    }

    if (Send_Ifon(Socket, Server_name, strlen(Server_name)) == ERROR)
    {
        printf("Sending Server_name failed!\n");
        return ERROR;
    }

    cur_time = Get_Data(cur_time);
    Send_Ifon(Socket, "Data: ", 6);
    if (Send_Ifon(Socket, cur_time, strlen(cur_time)) == ERROR)
    {
        printf("Sending cur_time error!\n");
        return ERROR;
    }

    if (Send_Ifon(Socket, Method_err_type, strlen(Method_err_type)) == ERROR)
    {
        printf("Sending method_error_type failed!\n");
        return ERROR;
    }

    if (Send_Ifon(Socket, Method_err_end, strlen(Method_err_end)) == ERROR)
    {
        printf("Sending method_error_end failed!\n");
        return ERROR;
    }

    if (Send_Ifon(Socket, Method_err_info, strlen(Method_err_info)) == ERROR)
    {
        printf("Sending method_error_info failed!\n");
        return ERROR;
    }

    return OK;
}


const char* Judge_Method(char* method, int Socket)
{
    //判断请求方式
    if (strcmp(method, "GET") == 0)
    {
        return "GET";
    }
    else if (strcmp(method, "POST") == 0)
    {
        return "POST";
    }
    else
    {
        Error_Request_Method(Socket);
        return ERROR;
    }
}



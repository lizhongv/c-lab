# include "webserver.h"
extern const char *Server_name;

int File_not_Inquire(int Socket)
{
    //404 Not Found响应
    const char* File_err_line = "HTTP/1.1 404 Not Found\r\n";
    const char* cur_time = "";
    const char* File_err_type = "Content-type: text/plain\r\n";
    const char* File_err_length = "Content-Length: 42\r\n";
    const char* File_err_end = "\r\n";
    const char* File_err_info = "The file which is requested is not found!\n";

    printf("The request file from client's request message is not found!\n");

    if (Send_Ifon(Socket, File_err_line, strlen(File_err_line)) == ERROR)
    {
        printf("Sending file_error_line error!\n");
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

    if (Send_Ifon(Socket, File_err_type, strlen(File_err_type)) == ERROR)
    {
        printf("Sending file_error_type error!\n");
        return ERROR;
    }

    if (Send_Ifon(Socket, File_err_length, strlen(File_err_length)) == ERROR)
    {
        printf("Sending file_error_length error!\n");
        return ERROR;
    }

    if (Send_Ifon(Socket, File_err_end, strlen(File_err_end)) == ERROR)
    {
        printf("Sending file_error_end error!\n");
        return ERROR;
    }

    if (Send_Ifon(Socket, File_err_info, strlen(File_err_info)) == ERROR)
    {
        printf("Sending file_error_info failed!\n");
        return ERROR;
    }

    return OK;
}


const char* Judge_File_Type(char* URI, const char* content_type)
{
    //文件类型判断
    const char* suffix;

    if ((suffix = strrchr(URI, '.')) != NULL)
        suffix = suffix + 1;

    if (strcmp(suffix, "html") == 0)
    {
        return content_type = "Content-type: text/html\r\n";
    }
    else if (strcmp(suffix, "jpg") == 0)
    {
        return content_type = "Content-type: image/jpg\r\n";
    }
    else if (strcmp(suffix, "png") == 0)
    {
        return content_type = "Content-type: image/png\r\n";
    }
    else if (strcmp(suffix, "gif") == 0)
    {
        return content_type = "Content-type: image/gif\r\n";
    }
    else if (strcmp(suffix, "txt") == 0)
    {
        return content_type = "Content-type: text/plain\r\n";
    }
    else if (strcmp(suffix, "xml") == 0)
    {
        return content_type = "Content-type: text/xml\r\n";
    }
    else if (strcmp(suffix, "rtf") == 0)
    {
        return content_type = "Content-type: text/rtf\r\n";
    }
    else
        return ERROR;
}


int Inquire_File(char* URI)
{
    //查找文件
    struct stat File_info;

    if (stat(URI, &File_info) == -1)
        return ERROR;
    else
        return File_info.st_size;
}


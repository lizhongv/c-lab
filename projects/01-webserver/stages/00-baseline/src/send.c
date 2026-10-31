# include "webserver.h"

extern const char* Server_name;

int Send_File(char* URI, int Socket)
{
    //200 OK响应
    const char* File_ok_line = "HTTP/1.1 200 OK\r\n";
    const char* cur_time = "";
    const char* File_ok_type = "";
    const char* File_ok_length = "Content-Length: ";
    const char* File_ok_end = "\r\n";

    FILE* file;
    struct stat file_stat;
    char Length[BUF_SIZE];
    char sendbuf[BUF_SIZE];
    int send_length;

    if (Judge_File_Type(URI, File_ok_type) == ERROR)
    {
        printf("The request file's type from client's request message is error!\n");
        return ERROR;
    }

    file = fopen(URI, "rb");
    if (file != NULL)
    {
        fstat(fileno(file), &file_stat);
        itoa(file_stat.st_size, Length, 10);

        if (Send_Ifon(Socket, File_ok_line, strlen(File_ok_line)) == ERROR)
        {
            printf("Sending file_ok_line error!\n");
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

        File_ok_type = Judge_File_Type(URI, File_ok_type);
        if (Send_Ifon(Socket, File_ok_type, strlen(File_ok_type)) == ERROR)
        {
            printf("Sending file_ok_type error!\n");
            return ERROR;
        }

        if (Send_Ifon(Socket, File_ok_length, strlen(File_ok_length)) != ERROR)
        {
            if (Send_Ifon(Socket, Length, strlen(Length)) != ERROR)
            {
                if (Send_Ifon(Socket, "\n", 1) == ERROR)
                {
                    printf("Sending file_ok_length error!\n");
                    return ERROR;
                }
            }
        }

        if (Send_Ifon(Socket, File_ok_end, strlen(File_ok_end)) == ERROR)
        {
            printf("Sending file_ok_end error!\n");
            return ERROR;
        }

        while (file_stat.st_size > 0)
        {
            if (file_stat.st_size < 1024)
            {
                send_length = fread(sendbuf, 1, file_stat.st_size, file);
                if (Send_Ifon(Socket, sendbuf, send_length) == ERROR)
                {
                    printf("Sending file information error!\n");
                    continue;
                }
                file_stat.st_size = 0;
            }
            else
            {
                send_length = fread(sendbuf, 1, 1024, file);
                if (Send_Ifon(Socket, sendbuf, send_length) == ERROR)
                {
                    printf("Sending file information error!\n");
                    continue;
                }
                file_stat.st_size -= 1024;
            }
        }
    }
    else
    {
        printf("The file is NULL!\n");
        return ERROR;
    }

    return OK;
}

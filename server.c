#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll

#define BUF_SIZE 1024
typedef struct sockaddr_in sockaddr_in;

void createResponse(char response[], char *status);  //根据命令参数创建响应

int main(){
    //先输入文件名，看文件是否能创建成功
    char buffer[BUF_SIZE] = {0};  //文件缓冲区
    int nCount;
    char filename[100] = {0};  //文件名
    printf("Input filename to save: ");
    gets(filename);
    FILE *fp = fopen(filename, "wb");  //以二进制方式打开（创建）文件
    if(fp == NULL){
        printf("Cannot open file, press any key to exit!\n");
        system("pause");
        exit(0);
    }

    //初始化 DLL
    WSADATA wsaData;
    WSAStartup( MAKEWORD(2, 2), &wsaData);
    SOCKET servSock = socket(AF_INET, SOCK_STREAM, 0);  //创建socket

    //配置服务器信息
    struct sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = PF_INET;  //使用IPv4地址
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  //配置IP地址
    sockAddr.sin_port = htons(1234);  //端口号
    
    bind(servSock, (SOCKADDR *)&sockAddr, sizeof(SOCKADDR));  //绑定套接字
    listen(servSock, 20);  //开始监听




    //接收客户端请求
    SOCKADDR clntAddr;
    int nSize = sizeof(SOCKADDR);
    SOCKET clntSock = accept(servSock, (SOCKADDR*)&clntAddr, &nSize);


    recv(clntSock, buffer, MAXBYTE, 0);  //接收HTTP request
    printf("request: %s\n", buffer);
    //成功创建文件，返回200
    createResponse(buffer, "200 OK");
    printf("response: %s\n", buffer);
    send(clntSock, buffer, BUF_SIZE, 0); //返回response
    printf("waiting for file...\n");
    system("PAUSE");

    //循环接收数据，直到文件传输完毕
    int num = 1;
    while( (nCount = recv(clntSock, buffer, BUF_SIZE, 0)) > 0 ){
        printf("%d: %s\n\n", num++, buffer);
        fwrite(buffer, nCount, 1, fp);
    }
    puts("File transfer success!");

    //文件接收完毕后直接关闭套接字
    fclose(fp);
    closesocket(clntSock);
    WSACleanup();

    system("pause");
    return 0;
}

//根据命令参数创建响应
void createResponse(char response[], char *status){
    response[0] = '\0';
    strcat(response, "HTTP/1.1 "); //添加协议版本
    strcat(response, status);    //添加状态码
    strcat(response, "\r\nHost: localhost:1234\r\n\r\n");    //响应头
}
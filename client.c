#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

#define BUF_SIZE 1024
typedef struct sockaddr_in sockaddr_in;

int main(){
    //先检查文件是否存在
    char *filename = "C:\\SourceCode\\httpNetDisc\\youzi.png";  //文件名
    FILE *fp = fopen(filename, "rb");  //以二进制方式打开文件
    if(fp == NULL){
        printf("Cannot open file, press any key to exit!\n");
        system("pause");
        exit(0);
    }

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = PF_INET;
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sockAddr.sin_port = htons(1234);
    connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
    
    //循环发送数据，直到文件结尾
    char buffer[BUF_SIZE] = {0};  //缓冲区
    int nCount;
    while( (nCount = fread(buffer, 1, BUF_SIZE, fp)) > 0 ){
        send(sock, buffer, nCount, 0);
    }

    shutdown(sock, SD_SEND);  //文件读取完毕，断开输出流，向客户端发送FIN包
    recv(sock, buffer, BUF_SIZE, 0);  //阻塞，等待客户端接收完毕

    fclose(fp); //关闭文件
    closesocket(sock);
    WSACleanup();
    

    system("pause");
    return 0;
}
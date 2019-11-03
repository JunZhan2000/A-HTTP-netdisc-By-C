#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <synchapi.h>
#include <winsock2.h>
#include "testServer.h"

#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll

char baseUrl[100] = "C:\\SourceCode\\httpNetDisc\\";  //根目录

int main(){
    WSADATA wsaData;
    WSAStartup( MAKEWORD(2, 2), &wsaData);  //初始化 DLL
    SOCKET servSock = createSocket();       //创建并配置套接字
    
    while(1){
        recvRequest(servSock);   //接收并处理请求
    }
    HANDLE threads[MAXCLIENTS];    //线程存放 数组
    int CountClient = 0;
    while(TRUE)
    {
        SOCKADDR clientAddr;
        int size = sizeof(SOCKADDR);
        SOCKET clientsocket;
        clientsocket = accept(s,&clientAddr,&size);        //阻塞模式 直到有新的Tcp 接入
        printf("Sys: New client touched ID is %d .\n",CountClient+1);            //
        if(CountClient < MAXCLIENTS)                        //创建新线程
        {
            threads[CountClient ++] = CreateThread(NULL,0,&ProcessClientRequest,&clientsocket,0,NULL);
        }
        else                                                //线程数超了 拒绝服务
        {
                char *msg = " Error Too many client Connecttion  !.\r\n";
                send(clientsocket,msg,strlen(msg)+sizeof(char),NULL);
                printf(" ** SYS **  REFUSED !.\n");
                closesocket(clientsocket);
        }
    }

    closesocket(servSock);  //关闭套接字
    WSACleanup();   //终止 DLL 的使用
    return 0;
}


//创建并配置socket
SOCKET createSocket(){
    //创建套接字
    SOCKET servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    //绑定套接字
    sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));  //每个字节都用0填充
    sockAddr.sin_family = PF_INET;  //使用IPv4地址
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  //绑定IP地址
    sockAddr.sin_port = htons(1234);  //端口
    bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));  //绑定端口
    listen(servSock, 20);  //进入监听状态
    
    return servSock;
}


//创建用来通话的SOCKET
SOCKET Accept(SOCKET servSock){
    SOCKADDR clntAddr;
    int nSize = sizeof(SOCKADDR);
    SOCKET clntSock = accept(servSock, (SOCKADDR*)&clntAddr, &nSize);  //等待客户端连接

    return clntSock;
}


//解析request请求行，返回一个请求行结构体
void parseRequestLine(char request[], RequestLine requstLine){
    char *move = request, *method = request, *url = 0, *version = 0;
    
    while(*move != '\r'){
        //循环条件：未遇到换行符，即请求行尾部
        if(*move == ' '){
            //遇到空格分隔符
            if(url == 0){
                url = move + 1;
            }else{
                version = move + 1;
            }
            *move = '\0';
        }
        move++;
    }
    *move = '\0';

    //将三个指针直接指向request数组中的属性，并将请求行的分隔符置为'\0'
    requstLine->method = method;
    requstLine->url = url;
    requstLine->version = version;
}


//根据命令参数创建响应
void createResponse(char response[], char *status){
    response[0] = '\0';
    strcat(response, "HTTP/1.1 "); //添加协议版本
    strcat(response, status);    //添加状态码
    strcat(response, "\r\nHost: localhost:1234\r\n\r\n");    //响应头
}


//接受并处理request
void recvRequest(SOCKET servSock) {
    RequestLine requestLine = (RequestLine)malloc(sizeof(struct RequestLine));  //request请求行结构体
    char fileUrl[100] = {0}, buffer[BUF_SIZE];  //缓冲区

    printf("waiting for connect...\n");
    SOCKET clntSock = Accept(servSock);  //接收socket连接

    recv(clntSock, buffer, MAXBYTE, 0);  //接收HTTP request
    // printf("\nMessage form client:\n%s\n", buffer);  //输出requst
    parseRequestLine(buffer, requestLine); //解析request，获取请求行参数
    strcat(fileUrl, baseUrl);
    strcat(fileUrl, requestLine->url);  //获取文件的完整地址

    if(strcmp(requestLine->method, "GET") == 0){
        processGet(clntSock, fileUrl);     //处理GET请求
    }
    else if (strcmp(requestLine->method, "POST") == 0) {
        processPost(clntSock, fileUrl);      //处理POST请求
    }
    else if (strcmp(requestLine->method, "DELETE") == 0){
        processDelete(clntSock, fileUrl);  //处理DELETE请求
    } else {
        printf("Invalid request.\n");
    }

    // printf("disconnect.\n");
    closesocket(clntSock);   //关闭套接字，断开连接
}


//处理GET请求
void processGet(SOCKET clntSock, char *fileUrl) {
    char buffer[BUF_SIZE];

    FILE *fp = fopen(fileUrl, "rb"); //以二进制方式打开文件
    if (fp == NULL) {
        //未找到文件，返回404
        createResponse(buffer, "404 NOT FOUND");
        send(clntSock, buffer, BUF_SIZE, 0);  //返回response
        printf("404 NOT FOUND\n");
        return;
    }

    //正常打开文件，创建响应头部并返回
    // printf("We have opened file!\n");
    createResponse(buffer, "200 OK");
    send(clntSock, buffer, BUF_SIZE, 0);  //返回response

    //分多个小包发送文件
    int nCount;
    while ((nCount = fread(buffer, 1, BUF_SIZE, fp)) > 0) {
        send(clntSock, buffer, nCount, 0);
    }
    shutdown(clntSock, SD_SEND);  //文件读取完毕，断开输出流，向客户端发送FIN包
    recv(clntSock, buffer, BUF_SIZE, 0);  //阻塞，等待客户端接收完毕关闭socket，接收对方发过来的FIN包

    fclose(fp); //关闭文件
    printf("We have sent the file: %s\n", fileUrl);
}


//处理POST请求
void processPost(SOCKET clntSock, char *fileUrl) {
    //先输入文件名，看文件是否能创建成功
    char buffer[BUF_SIZE] = {0};  //文件缓冲区

    FILE *fp = fopen(fileUrl, "wb");  //以二进制方式打开（创建）文件
    if(fp == NULL){
        createResponse(buffer, "500 Internal Server Error");
        send(clntSock, buffer, BUF_SIZE, 0);  //返回500 response
        printf("Cannot create the file.\n");
        return;
    }

    //成功创建文件，返回200
    createResponse(buffer, "200 OK");
    send(clntSock, buffer, BUF_SIZE, 0); //返回response

    recv(clntSock, buffer, BUF_SIZE, 0);
    int nCount;
    while( (nCount = recv(clntSock, buffer, BUF_SIZE, 0)) > 0 ){
        fwrite(buffer, nCount, 1, fp);
    }
    fclose(fp);  //关闭文件

    printf("Save file successfully!\n");
}


//处理DELETE请求
void processDelete(SOCKET clntSock, char *fileUrl) {
    char buffer[BUF_SIZE]; //缓冲区
    
    //尝试打开文件，判断文件是否存在
    FILE *fp = fopen(fileUrl, "rb"); //以二进制方式打开文件
    if (fp == NULL) {
        //未找到文件，返回404
        createResponse(buffer, "404 NOT FOUND");
        send(clntSock, buffer, BUF_SIZE, 0);  //返回response
        printf("We not found file\n");
        return;
    }
    fclose(fp);  //关闭文件
    // printf("File %s exist.\n", fileUrl);
    //文件存在，尝试删除文件
    if (remove(fileUrl) == 0) {
        //文件删除成功，返回200
        printf("Delete %s.\n", fileUrl);
        createResponse(buffer, "200 OK");
    } else {
        //文件删除失败，返回500
        perror("Delete");
        createResponse(buffer, "500 Internal Server Error");
    }
    send(clntSock, buffer, BUF_SIZE, 0);  //返回response
}
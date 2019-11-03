#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <string.h>
#include <synchapi.h>
#include "testClient.h"

#pragma comment(lib, "ws2_32.lib")  //加载 ws2_32.dll

int main(){
    //初始化DLL
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    start(); //开始的提示
    
    Command commandStruct = (Command)malloc(sizeof(struct Command));

    while (1) {
        char command[100]; //命令缓冲区
        getCommmand(commandStruct, command);

        Connect(commandStruct);        
    }

    free(commandStruct);
    WSACleanup();       //终止使用 DLL
    return 0;
}


//创建并配置socket
SOCKET createSocket(){
    //创建套接字
    SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    //向服务器发起请求
    sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));  //每个字节都用0填充
    sockAddr.sin_family = PF_INET;
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sockAddr.sin_port = htons(1234);
    connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));  //连接服务器
    // printf("connect to server.\n");

    return sock;
}


//程序开始，读取命令
void start(){
    printf("GET FileURL: download file\n");
    printf("POST FileURL: Upload file\n");
    printf("DELETE FileURL: delete file\n");
}


//解析命令，创建对应的Command结构体
void parseCommand(char command[], Command commandStruct){
    char *move = command;
    
    while(*move != ' ' && *move != '\0'){
        move++;
    }
    *move = '\0';

    //配置command结构体
    commandStruct->method = command;
    commandStruct->fileUrl = move + 1;
}


//根据命令参数创建请求
void createRuquest(Command commandStruct, char request[]) {
    request[0] = '\0';
    strcat(request, commandStruct->method);     //添加请求方法
    strcat(request, " ");        //空格
    strcat(request, commandStruct->fileUrl);    //添加文件地址
    strcat(request, " HTTP/1.1\r\nUser-Agent: myClient\r\n\r\n");    //添加协议版本及请求头
}


//解析response状态行，返回一个状态行结构体指针
void parseResponseStatus(char response[], ResponseStatus responseStatus){
    char *move = response, *version = response, *status = 0;
    
    while(*move != '\r'){
        //循环条件：未遇到换行符，即请求行尾部
        if(*move == ' '){
            //遇到空格分隔符
            if(status == 0){
                *move = '\0';
                status = move + 1;
            }
        }
        move++;
    }
    *move = '\0';

    //将三个指针直接指向request数组中的属性，并将请求行的分隔符置为'\0'
    responseStatus->version = version;
    responseStatus->status = status;
}


//读取命令
void getCommmand(Command commandStruct, char *command) {
    gets(command);
    parseCommand(command, commandStruct);  //解析命令，获取命令参数
    if(strcmp(commandStruct->method, "quit") == 0){
        printf("ByeBye~\n");
        exit(0);
    }
}


//连接服务器
void Connect(Command commandStruct) {
    SOCKET sock = createSocket();   //创建socket，连接服务器
    sendRequest(commandStruct, sock);  //发送请求
    closesocket(sock);  //关闭套接字
}


//发送HTTP请求
void sendRequest(Command commandStruct, SOCKET sock) {
    char buffer[BUF_SIZE] = {0};
    ResponseStatus responseStatus = (ResponseStatus)malloc(sizeof(ResponseStatus));
    
    createRuquest(commandStruct, buffer);  //创建HTTP request

    send(sock, buffer, BUF_SIZE, 0);   //发送request

    recv(sock, buffer, BUF_SIZE, 0);   //接收response
    

    parseResponseStatus(buffer, responseStatus);  //解析response，获取状态行参数
    
    printf("response status: %s\n", responseStatus->status); //输出状态码
    processResponse(commandStruct, responseStatus, sock);   //处理响应

    free(responseStatus);
}


//处理响应
void processResponse(Command commandStruct, ResponseStatus responseStatus, SOCKET sock) {
    // printf("command.method: %s\n", commandStruct->method);
    // printf("command.fileUrl: %s\n", commandStruct->fileUrl);
    if (strcmp(commandStruct->method, "GET") == 0) {
        processGetRes(commandStruct, responseStatus, sock);
    } else if (strcmp(commandStruct->method, "POST") == 0) {
        processPOSTRes(commandStruct, responseStatus, sock);
    } else if (strcmp(commandStruct->method, "DELETE") == 0) {
        processDeleteRes(responseStatus);
    }
}


//处理GET请求的响应
void processGetRes(Command commandStruct, ResponseStatus responseStatus, SOCKET sock) {
    if(strcmp(responseStatus->status, "200 OK") == 0){
        //正常，开始传输文件
        recvFile(sock, commandStruct->fileUrl);
    } else if (strcmp(responseStatus->status, "404 NOT FOUND") == 0) {
        //未找到文件
        printf("File does not exist!\n");
    } else if (strcmp(responseStatus->status, "500 Internal Server Error") == 0) {
        //服务器问题
        printf("File transfer failed.\n");
    }
}


//处理POST请求的响应
void processPOSTRes(Command commandStruct, ResponseStatus responseStatus, SOCKET sock){
    if(strcmp(responseStatus->status, "200 OK") == 0){
        //正常，开始传输文件
        sendFile(sock, commandStruct->fileUrl);
    } else if (strcmp(responseStatus->status, "500 Internal Server Error") == 0) {
        //服务器问题
        printf("File transfer failed.\n");
    }
}


//处理DELETE请求的响应
void processDeleteRes(ResponseStatus responseStatus) {
    if(strcmp(responseStatus->status, "200 OK") == 0){
        //文件删除成功
        printf("File deleted successfully!\n");
    } else if (strcmp(responseStatus->status, "404 NOT FOUND") == 0) {
        //未找到文件
        printf("File does not exist!\n");
    } else if (strcmp(responseStatus->status, "500 Internal Server Error") == 0) {
        //文件删除失败
        printf("File deletion failed.\n");
    }
}


//接收文件
void recvFile(SOCKET sock, char * fileUrl){
    FILE *fp = fopen(fileUrl, "wb");  //以二进制方式打开（创建）文件
    // printf("fileUrl: %s\n", fileUrl);
    if(fp == NULL){
        printf("Cannot create the file.\n");
        return;
    }
    //循环接收数据，直到文件传输完毕
    char buffer[BUF_SIZE] = {0};  //文件缓冲区
    int nCount;
    while( (nCount = recv(sock, buffer, BUF_SIZE, 0)) > 0 ){
        fwrite(buffer, nCount, 1, fp);
    }

    printf("Download file successfully!\n");

    fclose(fp);
}


//上传文件
void sendFile(SOCKET sock, char *fileUrl) {
    //先检查文件是否存在
    char buffer[BUF_SIZE];

    FILE *fp = fopen(fileUrl, "rb"); //以二进制方式打开文件
    if (fp == NULL) {
        //文件打开失败
        printf("File open failed\n");
        return;
    }

    //正常打开文件, 分多个小包发送文件
    int nCount;
    while ((nCount = fread(buffer, 1, BUF_SIZE, fp)) > 0) {
        send(sock, buffer, nCount, 0);
    }
    shutdown(sock, SD_SEND);  //文件读取完毕，断开输出流，向服务端发送FIN包
    recv(sock, buffer, BUF_SIZE, 0);  //阻塞，等待服务端接收完毕关闭socket，接收对方发过来的FIN包

    fclose(fp);  //关闭文件
    printf("File upload succeeded: %s\n", fileUrl);
}
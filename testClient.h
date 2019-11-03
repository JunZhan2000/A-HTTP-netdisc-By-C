#ifndef _TESTCLIENT_H

#include <winsock2.h>

#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll

#define BUF_SIZE 1024

struct Command{
    //命令结构体
    char *method;       //请求方法
    char *fileUrl;      //文件地址
};

struct ResponseStatus{
    //respomse状态行
    char *version;      //协议版本
    char *status;       //状态码
};

typedef struct sockaddr_in sockaddr_in;
typedef struct ResponseStatus *ResponseStatus;
typedef struct Command *Command;

SOCKET createSocket();  //创建并配置socket
void start();           //提示
void getCommmand(Command commandStruct, char *command);     //读取命令
void parseCommand(char command[], Command commandStruct);   //解析命令，创建对应的Command结构体
void createRuquest(Command commandStruct, char request[]);  //根据命令参数创建请求
void parseResponseStatus(char response[], ResponseStatus responseStatus);  //解析response状态行，返回一个状态行结构体指针
void Connect(Command commandStruct);    //连接服务器
void sendRequest(Command commandStruct, SOCKET sock);   //发送HTTP请求
void processResponse(Command commandStruct, ResponseStatus responseStatus, SOCKET sock);    //处理响应
void recvFile(SOCKET sock, char *fileUrl);              //接收文件
void sendFile(SOCKET sock, char *fileUrl);              //上传文件
void processGetRes(Command commandStruct, ResponseStatus responseStatus, SOCKET sock);      //处理GET请求的响应
void processPOSTRes(Command commandStruct, ResponseStatus responseStatus, SOCKET sock);     //处理POST请求的响应
void processDeleteRes(ResponseStatus responseStatus);   //处理DELETE请求的响应

#endif
#ifndef _TESTSERVER_H

#include <winsock2.h>

#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll

#define BUF_SIZE 1024
#define MAX_CLIENTS 20

struct RequestLine{
    //request请求行
    char *method;      //请求方法
    char *url;         //请求url
    char *version;       //HTTP协议版本
};

typedef struct sockaddr_in sockaddr_in;
typedef struct RequestLine *RequestLine;

SOCKET createSocket();  //创建并配置socket
SOCKET Accept(SOCKET servSock);  //接收socket连接
DWORD WINAPI recvRequest(LPVOID lpParam);  //接受并处理request的线程函数
void createResponse(char response[], char *status);  //根据命令参数创建请求
void parseRequestLine(char request[], RequestLine requstLine);     //解析request请求行，返回一个请求行结构体指针
void processGet(SOCKET clntSock, char *fileUrl);  //处理GET请求
void processPost(SOCKET clntSock, char *fileUrl);      //处理POST请求
void processDelete(SOCKET clntSock, char *fileUrl);  //处理DELETE请求     

#endif





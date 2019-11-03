#include "stdio.h"
#include "winsock2.h"
#include "stdlib.h"
#define MAXCLIENTS 2

#pragma comment(lib,"ws2_32")


DWORD WINAPI ProcessClientRequest(LPVOID lpParam)
{
    SOCKET* clientsocket = (SOCKET*)lpParam;
    char* msg = "Hello, Welcome to connect . \r\n";
    send(*clientsocket,msg,strlen(msg)+sizeof(char),NULL);
    printf("---SYS----    Hello @_@\n");
    while(TRUE)
    {
        char buffer[MAXBYTE] = {0};
        recv(*clientsocket,buffer,MAXBYTE,NULL);
        if(strcmp(buffer,"exit")==0)
        {
            char* exit_msg = "Bye \r\n";
            send(*clientsocket,exit_msg,strlen(exit_msg)+sizeof(char),NULL);
            break;
        }
        printf("--- Sys: %s--\n",buffer);
    }
    closesocket(*clientsocket);
    return 0;
}


int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2),&wsaData);
    SOCKET s = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = PF_INET;
    sockaddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    sockaddr.sin_port = htons(9000);
    bind(s,(SOCKADDR*)&sockaddr,sizeof(SOCKADDR));
    listen(s,1);
    printf("listening on port[%d].\n",9000);
    
    char* msg = new char[1000];
    
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

    printf("Maximize clients occurred for d%.\r\n", MAXCLIENTS); 
    WaitForMultipleObjects(MAXCLIENTS,threads,TRUE,INFINITE);
    closesocket(s); //关闭socket

    for(int i=0;i<MAXCLIENTS;i++)
    {
        CloseHandle(threads[i]);
    }

    WSACleanup();
    getchar();
    exit(0);
    return 0;
}
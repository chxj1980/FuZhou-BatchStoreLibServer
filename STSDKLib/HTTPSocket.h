#pragma once
#include "LogRecorder.h"
#include "STSDKInterface.h"
#include <WinSock2.h>
#pragma comment( lib, "Ws2_32.lib" )

#define RESOVERTIME 8   //商汤回应超时时间
typedef struct _HTTPMsg
{
    string sHttpHead;   //http头信息
    int nCurBodyLen;    //当前己收到http Body长度
    int nBodyLen;       //Body总长度
    string sHttpBody;   //Body信息
    time_t tRecvTime;   //收到头时间
    _HTTPMsg()
    {
        nCurBodyLen = 0;
        nBodyLen = 0;
        time(&tRecvTime);
    }
    void Init()
    {
        nCurBodyLen = 0;
        nBodyLen = 0;
        time(&tRecvTime);
        sHttpBody = "";
        sHttpHead = "";
    }
}HTTPMSG, *LPHTTPMSG;

class CHTTPSocket
{
public:
    CHTTPSocket();
    ~CHTTPSocket();
public:
    bool InitConnect(const char * pIP, unsigned int nPort);
    bool SendMsg(const char * pMsg, unsigned int nSize);
    int RecvMsg(char * pRecvMsg, unsigned int * nSize);
private:
    int ParseHTTPMsg(const char * pMsg, unsigned int nSize);
    bool ReConnectSTServer();
private:
    SOCKET m_Socket;
    SOCKADDR_IN m_addrSTServer;
    char  m_pRecvMsg[4096];
    DWORD m_nRecvSize;

    char m_pServerIP[20];
    int m_nServerPort;

    LPHTTPMSG m_pHttpMsg;       //商汤回应Http消息
    static unsigned int m_nNum;

public:
    int m_nCurrentNum;


    
};


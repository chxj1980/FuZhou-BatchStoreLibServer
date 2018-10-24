#pragma once
#include "LogRecorder.h"
#include "STSDKInterface.h"
#include <WinSock2.h>
#pragma comment( lib, "Ws2_32.lib" )

#define RESOVERTIME 8   //������Ӧ��ʱʱ��
typedef struct _HTTPMsg
{
    string sHttpHead;   //httpͷ��Ϣ
    int nCurBodyLen;    //��ǰ���յ�http Body����
    int nBodyLen;       //Body�ܳ���
    string sHttpBody;   //Body��Ϣ
    time_t tRecvTime;   //�յ�ͷʱ��
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

    LPHTTPMSG m_pHttpMsg;       //������ӦHttp��Ϣ
    static unsigned int m_nNum;

public:
    int m_nCurrentNum;


    
};


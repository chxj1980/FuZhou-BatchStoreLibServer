#pragma once
#include "HttpConnection.h"
#include "TCPIOCPSocket.h"
#include "DataDefine.h"
#include "LogRecorder.h"

class CPostLayoutAlarm
{
public:
    CPostLayoutAlarm();
    ~CPostLayoutAlarm();
public:
    bool InitHttpClient(string sAlarmServerIP, int nAlarmServerPort);
    void StopWork();
    
    int SendLayoutAlarm(LPLAYOUTALARM pLayoutAlarm, char * pResponseMsg, int * nMsgLen);
private:
    static void _stdcall MSGCALLBACK(SOCKET RemoteSocket, SOCKADDR_IN RemoteAddr, string sMsg, int nError);
    void ParseMessage(SOCKET ClientSocket, SOCKADDR_IN RemoteAddr, string RecvBuf);
private:
    CTCPIOCPSocket * m_pTCPSocket;
    HANDLE m_hConnectST;
    HANDLE m_hResponseEvent;
    char m_pSTAddr[32];

    char m_pHttpBody[SQLMAXLEN];
    string m_sResponseMsg;
};


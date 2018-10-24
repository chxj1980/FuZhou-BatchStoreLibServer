#pragma once
#include "TCPIOCPSocket.h"
#include "HttpProtocol.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"

#define HTTPMSGOVERTIME 500
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

typedef void(CALLBACK* LPHTTPCLIENTREQUEST)(LPHTTPREQUEST pHttpRequest, void * pUser);
class CHttpServerAction
{
public:
    CHttpServerAction();
    ~CHttpServerAction();
public:
    bool StartHttpListen(char * pHttpIP, int nHttpPort, LPHTTPCLIENTREQUEST pHttpClientRequest, void * pUser);
    void StopHttpServer();
    bool ResponseBody(SOCKET ClientSocket, string sRecvHttpMsg, string sResponseBody);
public:
    static void CALLBACK MSGCALLBACK(SOCKET RemoteSocket, SOCKADDR_IN RemoteAddr, string sMsg, int nEvent);
    void ParseMessage(SOCKET ClientSocket, SOCKADDR_IN RemoteAddr, string RecvBuf);

    //回调客户端请求线程
    static DWORD WINAPI ClientRequestCallbackThread(LPVOID lParam);
    void ClientRequestCallbackAction();
    string ChangeSecondToTime(unsigned long long nSecond);

private:
    CRITICAL_SECTION m_cs;
    CRITICAL_SECTION m_listcs;
    LPHTTPCLIENTREQUEST m_pHttpRequestCallback;
    void * m_pUser;
    HANDLE m_hStopEvent;   //服务停止事件

    CTCPIOCPSocket * m_pTCPSocket;
    std::map<SOCKET, LPHTTPMSG> m_mapClientMsg;
    std::list <LPHTTPREQUEST> m_listHTTPRequest;
};


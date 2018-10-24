#pragma once
#include "TCPIOCPSocket.h"
#include "HttpProtocol.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"

#define HTTPMSGOVERTIME 500
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

    //�ص��ͻ��������߳�
    static DWORD WINAPI ClientRequestCallbackThread(LPVOID lParam);
    void ClientRequestCallbackAction();
    string ChangeSecondToTime(unsigned long long nSecond);

private:
    CRITICAL_SECTION m_cs;
    CRITICAL_SECTION m_listcs;
    LPHTTPCLIENTREQUEST m_pHttpRequestCallback;
    void * m_pUser;
    HANDLE m_hStopEvent;   //����ֹͣ�¼�

    CTCPIOCPSocket * m_pTCPSocket;
    std::map<SOCKET, LPHTTPMSG> m_mapClientMsg;
    std::list <LPHTTPREQUEST> m_listHTTPRequest;
};


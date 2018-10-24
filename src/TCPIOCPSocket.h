#pragma once
#include <string>
#include <map>
#include <WinSock2.h>
#pragma comment( lib, "Ws2_32.lib" )
using namespace std;

/*nEvent: 0:无错误, 正常回调信息, 1: 有客户端连接, 10000: 客户端断开连接, Other: 错误*/
typedef void(CALLBACK* LPMSGCALLBACK)(SOCKET RemoteSocket, SOCKADDR_IN RemoteAddr, string sMsg, int nEvent);
#define DATA_BUFSIZE 4096
typedef struct _HandleData
{
    SOCKET RemoteSocket;  
}HANDLEDATA, *LPHANDLEDATA;

typedef struct _IocpData
{
    OVERLAPPED Overlapped;
    WSABUF DataBuf;
    int nOperatorType;              //1为发送数据, 2为接收数据
    SOCKADDR_IN addrClient;
}IOCPDATA, * LPIOCPDATA;

typedef struct _SocketResource
{
    SOCKET RemoteSocket;
    SOCKADDR_IN RemoteAddr;         //地址
    LPHANDLEDATA pHandleData;
    LPIOCPDATA pRecvIocpData;
    LPIOCPDATA pSendIocpData;

    bool bDestroy;
    time_t tDestroyTime;
    _SocketResource()
    {
        RemoteSocket = INVALID_SOCKET;
        pHandleData = new HANDLEDATA;
        pSendIocpData = new IOCPDATA;
        pSendIocpData->nOperatorType = 1;
        pRecvIocpData = new IOCPDATA;
        pRecvIocpData->DataBuf.buf = new char[DATA_BUFSIZE];
        pRecvIocpData->DataBuf.len = DATA_BUFSIZE;
        pRecvIocpData->nOperatorType = 2;
        bDestroy = false;
        tDestroyTime = 0;
    }
    ~_SocketResource()
    {
        if(RemoteSocket != INVALID_SOCKET)
        {
            closesocket(RemoteSocket);
            RemoteSocket = INVALID_SOCKET;
        }
        delete pHandleData;
        delete []pRecvIocpData->DataBuf.buf;
        delete pRecvIocpData;
        delete pSendIocpData;
    }
}SOCKETRESOURCE, *LPSOCKETRESOURCE;


class CTCPIOCPSocket
{
public:
    CTCPIOCPSocket(void);
public:
    ~CTCPIOCPSocket(void);
public:
    bool InitSocket(string sLocalIP, int nLocalPort, LPMSGCALLBACK pMsgCallback, bool m_bIsServer = false);

    static DWORD WINAPI TCPStreamThread(LPVOID lParam);
    bool RunAccept();        //作为TCP服务端时使用
    bool ConnectServer(string sServerIP, int nServerPort);  //作为TCP客户端时使用
    void StopSocket();

    bool SendData(char * pSendData, int nLen, SOCKET RemoteSocket = INVALID_SOCKET);
    bool RecvData(SOCKET RemoteSocket = INVALID_SOCKET);

    static DWORD WINAPI ConnectServerThread(LPVOID lParam);
    void ConnectServerAction();

    static DWORD WINAPI ServerWorkerThread(LPVOID lParam);
    void ServerWorkerAction();

    static DWORD WINAPI DestroyResourceThread(LPVOID lParam);
    void DestroyResourceAction();



private:
    bool m_bIsServer;               //true: TCP服务端, false: TCP客户端
    HANDLE m_CompletionPort;        //完成端口句柄
    SYSTEM_INFO SystemInfo;         //系统信息, 用来读取Cpu个数
    DWORD m_nByteOfSend;

    SOCKET m_LocalSocket;           //本地Socket
    SOCKADDR_IN m_LocalAddr;        //本地地址
    string m_sLocalIP;              //本地IP
    int m_nLocalPort;               //本地端口

    SOCKADDR_IN m_RemoteAddr;       //作为客户端时使用, 连接服务器地址
    string m_sServerIP;             //服务器IP
    int m_nServerPort;           //服务器Port

    LPMSGCALLBACK m_pMsgCallback;  //信息回调函数
    map<SOCKET, LPSOCKETRESOURCE> m_mapSocketResource; 

    CRITICAL_SECTION m_cs;
    bool m_bStopSocket;             //停止标志
    HANDLE m_hStopEvent;            //停止事件
    int m_nConnectServerFailed;          //作为客户端时, 连接服务失败次数
};

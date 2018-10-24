#pragma once
#include <string>
#include <map>
#include <WinSock2.h>
#pragma comment( lib, "Ws2_32.lib" )
using namespace std;

/*nEvent: 0:�޴���, �����ص���Ϣ, 1: �пͻ�������, 10000: �ͻ��˶Ͽ�����, Other: ����*/
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
    int nOperatorType;              //1Ϊ��������, 2Ϊ��������
    SOCKADDR_IN addrClient;
}IOCPDATA, * LPIOCPDATA;

typedef struct _SocketResource
{
    SOCKET RemoteSocket;
    SOCKADDR_IN RemoteAddr;         //��ַ
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
    bool RunAccept();        //��ΪTCP�����ʱʹ��
    bool ConnectServer(string sServerIP, int nServerPort);  //��ΪTCP�ͻ���ʱʹ��
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
    bool m_bIsServer;               //true: TCP�����, false: TCP�ͻ���
    HANDLE m_CompletionPort;        //��ɶ˿ھ��
    SYSTEM_INFO SystemInfo;         //ϵͳ��Ϣ, ������ȡCpu����
    DWORD m_nByteOfSend;

    SOCKET m_LocalSocket;           //����Socket
    SOCKADDR_IN m_LocalAddr;        //���ص�ַ
    string m_sLocalIP;              //����IP
    int m_nLocalPort;               //���ض˿�

    SOCKADDR_IN m_RemoteAddr;       //��Ϊ�ͻ���ʱʹ��, ���ӷ�������ַ
    string m_sServerIP;             //������IP
    int m_nServerPort;           //������Port

    LPMSGCALLBACK m_pMsgCallback;  //��Ϣ�ص�����
    map<SOCKET, LPSOCKETRESOURCE> m_mapSocketResource; 

    CRITICAL_SECTION m_cs;
    bool m_bStopSocket;             //ֹͣ��־
    HANDLE m_hStopEvent;            //ֹͣ�¼�
    int m_nConnectServerFailed;          //��Ϊ�ͻ���ʱ, ���ӷ���ʧ�ܴ���
};

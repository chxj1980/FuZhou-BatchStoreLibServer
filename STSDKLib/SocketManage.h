#pragma once
#include <list>
#include "LogRecorder.h"
#include "STSDKInterface.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"
#include "HTTPSocket.h"
using namespace std;
using namespace rapidjson;

typedef enum OperatorFun
{
    OPERATORINIT,
    ADDDB           ,
    DELDB           ,
    CLEARDB         ,
    GETALLDBINFO    ,
    FACEQUALITY     ,
    SYNADDIMAGE     ,
    SYNADDMULTIPLEIMAGE, 
    SYNADDFEATURE   ,
    GETIMAGEBYID    ,
    DELIMAGEBYID    ,
    SEARCHIMAGE     ,
    VERIFICATION    ,
    SEARCHIMAGEBYFEATURE ,
    GETATTRIBUTE    ,
    GETFEATURE      ,
    GETDETAIL       
}OPERATORFUN;

#define HTTPSENDMSGSIZE 1024 * 1024 * 5
#define HTTPRECVMSGSIZE 1024 * 1024 * 1
typedef struct _SocketInfo
{
    char pSTServerIP[20];   //ST������IP
    int nSTServerPort;      //ST�������˿�
    int nType;              //ST����������, 1: ץ��, 2: �ص�
    bool bUsed;             //�Ƿ񼺱�ʹ��
    bool bStatus;           //ST�������Ƿ���������
    char * pHTTPMsg;        //����HTTP��Ϣ
    int nHTTPMsgLen;        //����HTTP��Ϣ����
    char * pRecvMsg;        //�յ�ST��Ӧ��Ϣ
    unsigned int nRecvLen;  //��Ӧ��Ϣ����
    bool bDeal;             //�յ�����Ϣ�Ƿ񼺴���
    HANDLE m_hSTEvent;      //�����ҽ���ST��������Ϣ�����Ƿ����֪ͨ�¼�
    int nError;             //������Ϣ�쳣������
    OPERATORFUN nFunType;           //��������, ��ͬ���͵ķ���Json���ϲ�����һ��
    _SocketInfo()
    {
        bUsed = false;
        pHTTPMsg = NULL;
        nHTTPMsgLen = 0;
        pRecvMsg = new char[HTTPRECVMSGSIZE];
        nRecvLen = HTTPRECVMSGSIZE;
        m_hSTEvent = CreateEvent(NULL, true, false, NULL);
        nRecvLen = 0;
        bDeal = true;
        bStatus = false;
        nError = 0;
    }
    void Init()
    {
        pHTTPMsg = NULL;
        nHTTPMsgLen = 0;
        nRecvLen = 0;
        bDeal = true;
        nError = 0;
    }
    ~_SocketInfo()
    {
        delete []pRecvMsg;
        pRecvMsg = NULL;
        CloseHandle(m_hSTEvent);
    }
}SOCKETINFO, *LPSOCKETINFO;

typedef struct _STServerCount
{
    int nCurCount;  //ST�⵱ǰ�����
    int nMaxCount;  //ST���������
    _STServerCount()
    {
        nCurCount = 0;
        nMaxCount = 0;
    }
}STSERVERCOUNT, *LPSTSERVERCOUNT;

class CSocketManage
{
public:
    CSocketManage();
    ~CSocketManage();
public:
    bool Init(STSERVERINFO STServerInfo[], int nQuantity);
    bool Uninit();
    //nSTType: ѡ����������������, 1:ץ��(·��)��, 2: �ص��; nFunType: ��������; nSynNum: ���ʱ�����ͼƬ������ֵ����
    bool SendMsg(char * pHttpMsg, int nHttpMsgLen, int nSTType, OPERATORFUN nFunType, int nSynNum = 1);
    int RecvMsg(char * pRecvMsg, unsigned int * nSize);
private:
    static DWORD WINAPI PostSTMsgThread(LPVOID lParam);
    void PostSTMsgAction();

    //ͼƬ����ֵ���, ���ؾ���ѡ��ǰ�����������С��ST������
    string GetAddImageSTServer(int nSynNum);
    //�ϲ�Json������
    int DealwithRepMsg(char * pMsg, int nLen, char * pSTServerIP, int nFunType);
    void ParseErrorJson(int nError);
public:
    static map<string, LPSTSERVERCOUNT> m_mapDBCountInfo;  //����ST���Ӧ�ĵ�ǰ���������, �������ʱ���ؾ���
    static int m_nNum;
private:
    CRITICAL_SECTION m_cs;
    map<CHTTPSocket*, LPSOCKETINFO> m_mapHTTPSocket;
    HANDLE m_hStopEvent;           //����ֹͣ�¼�

    char * m_pRecvMsg;
    int m_nRecvLen;

};


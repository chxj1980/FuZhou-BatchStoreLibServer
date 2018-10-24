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
    char pSTServerIP[20];   //ST服务器IP
    int nSTServerPort;      //ST服务器端口
    int nType;              //ST服务器类型, 1: 抓拍, 2: 重点
    bool bUsed;             //是否己被使用
    bool bStatus;           //ST服务器是否连接正常
    char * pHTTPMsg;        //发送HTTP消息
    int nHTTPMsgLen;        //发送HTTP消息长度
    char * pRecvMsg;        //收到ST回应消息
    unsigned int nRecvLen;  //回应消息长度
    bool bDeal;             //收到的消息是否己处理
    HANDLE m_hSTEvent;      //发送且接收ST服务器信息流程是否结束通知事件
    int nError;             //处理消息异常错误码
    OPERATORFUN nFunType;           //方法类型, 不同类型的方法Json串合并处理不一样
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
    int nCurCount;  //ST库当前己入库
    int nMaxCount;  //ST库最大容量
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
    //nSTType: 选择商汤服务器类型, 1:抓拍(路人)库, 2: 重点库; nFunType: 商汤操作; nSynNum: 入库时的入库图片或特征值数量
    bool SendMsg(char * pHttpMsg, int nHttpMsgLen, int nSTType, OPERATORFUN nFunType, int nSynNum = 1);
    int RecvMsg(char * pRecvMsg, unsigned int * nSize);
private:
    static DWORD WINAPI PostSTMsgThread(LPVOID lParam);
    void PostSTMsgAction();

    //图片特征值入库, 负载均衡选择当前己入库容量最小的ST服务器
    string GetAddImageSTServer(int nSynNum);
    //合并Json串处理
    int DealwithRepMsg(char * pMsg, int nLen, char * pSTServerIP, int nFunType);
    void ParseErrorJson(int nError);
public:
    static map<string, LPSTSERVERCOUNT> m_mapDBCountInfo;  //保存ST库对应的当前己入库容量, 用于入库时负载均衡
    static int m_nNum;
private:
    CRITICAL_SECTION m_cs;
    map<CHTTPSocket*, LPSOCKETINFO> m_mapHTTPSocket;
    HANDLE m_hStopEvent;           //服务停止事件

    char * m_pRecvMsg;
    int m_nRecvLen;

};


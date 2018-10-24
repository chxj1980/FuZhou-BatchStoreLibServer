#pragma once

#include "ConfigRead.h"
#include "mysql_acl.h"
#include "HttpServerAction.h"   //接收客户端推送布控信息
#include "ZeromqManage.h"
#include "CheckpointSubManage.h"
#include <set>
using namespace std;


typedef struct _AnalyseFeatureLibInfo
{
    set<int> setFeatureInfo;
}ANALYSEFEATURELIBINFO, *LPANALYSEFEATURELIBINFO;
typedef map<string, LPANALYSEFEATURELIBINFO> MAPANALYSEFEATURELIBINFO;

class CLayoutServer
{
public:
    CLayoutServer(void);
public:
    ~CLayoutServer(void);
public:
    bool Init();
    bool StartLayoutServer();
    bool StopLayoutServer();

private:
    bool InitDB();
    //连接数据库查询布控卡口信息
    bool GetLayoutCameraInfo();
    //获取接口服务布控任务
    bool GetLayoutTaskInfo();
    //初始化Http服务
    bool InitHttpServer(string sLocalIP, int nLocalPort);
    //初始化Redis连接
    bool InitRedisManage(string sRedisIP, int nRedisPort);

    //推送布控图片信息并接收布控结果
    bool InitAnalyseSearch();
    //分析服务布控结果回调 
    static void CALLBACK AnalyseLayoutResultCallback(LPSUBMESSAGE pSubMessage, void * pUser);

    //初始化订阅卡口图片
    bool InitCheckpointSubManage();
    //Zeromq订阅人脸图片信息回调
    static void CALLBACK ImageSubCallback(LPSUBMESSAGE pSubMessage, void * pUser);

    //订阅分析布控服务特征值结点指针信息
    bool InitZeroMq();
    //Zeromq回调特征值结点信息
    static void CALLBACK AnalyseLibInfoCallback(LPSUBMESSAGE pSubMessage, void * pUser);

    //发面卡口订阅请求线程
    static DWORD WINAPI SubCheckpointThread(LPVOID lParam);
    void SubCheckpointAction();

    //增加布控卡口(初始化时使用) nTaskType: 0: 系统布控, 1: 接口服务布控任务新增, 2: 接口服务布控任务更新
    bool AddSubCheckpointInfo(int nLibID, char * pBeginTime, char * pEndtime, char * pCheckpointID, int nScore, int nTaskType = 0);
    //删除布控卡口
    bool DelLayoutCheckpoint(int nLibID, char * pCheckpointID);
    //增加布控人脸图片
    bool AddLayoutImage(int nLibID, char * pBeginTime, char * pEndtime, char * pFaceUUID);
    //删除布控人脸图片
    bool DelLayoutImage(int nLibID, char * pFaceUUID);
    //停止布控库
    bool StopLayoutLib(int nLibID);
    //增加|修改布控任务
    int AddLayoutTaskInfo(LPLAYOUTTASKINFO pTaskInfo);
    //删除布控任务
    int StopLayoutTask(string sTaskID);

    //HTTP客户端请求回调
    static void CALLBACK HttpClientRequestCallback(LPHTTPREQUEST pHttpRequest, void * pUser);
    //处理Http请求线程
    static DWORD WINAPI HTTPRequestParseThread(LPVOID lParam);
    void HTTPRequestParseAction();
    //回应客户端
    void SendResponseMsg(SOCKET ClientSocket, int nEvent, string sTaskID = "");
    int GetErrorMsg(int nError, char * pMsg);

    unsigned long long ChangeTimeToSecond(string sTime);
    string ChangeSecondToTime(unsigned long long nSecond);
    string GetUUID();

    bool UpdateTaskInfoToDB(string sTaskID, bool bAdd = true, string sTaskInfo = "", string sPublishURL = "");
private:
    CRITICAL_SECTION m_cs;
    CRITICAL_SECTION m_csHttp;
    CRITICAL_SECTION m_csFeature;
    CRITICAL_SECTION m_csTask;
    HANDLE m_hStopEvent;                //服务停止事件
    CConfigRead * m_pConfigRead;        //配置文件读取

    char m_pServerIP[20];               //本地服务IP
    int m_nServerPort;                  //本地服务Port

    char m_pProxyServerIP[20];          //分析代理服务IP
    int m_nProxyPort;                   //分析代理服务Port
    CZeromqManage * m_pZeromqManage;    //订阅分析服务库特征值结点信息
    CCheckpointSubManage * m_pCheckSubManage;   //订阅卡口抓拍图片管理

    CMysql_acl m_mysqltool;             //MySQL数据库操作

    char m_pRedisIP[20];                //Redis IP
    int m_nRedisPort;                   //Redis Port
    CRedisManage * m_pRedisManage;      //Redis操作管理

    CAnalyseSearch * m_pAnalyseSearch;   //分析布控服务操作对象

    CHttpServerAction * m_pHttpServer;  //Web HTTP请求
    LISTHTTPREQUEST m_listHttpRequest;  //HTTP请求消息链表

    int m_nImageNum;                    //己抓拍回调的图片数量
    int m_nAnalyseSearchNum;            //匹配中的图片数量
    SYSTEMTIME m_sysTime;               //时间

    MAPANALYSEFEATURELIBINFO m_mapAnalyseFeatureLibInfo;    //分析服务特征值结点信息
    MAPLAYOUTCHECKPOINTINFO m_mapLayoutCheckpointInfo;      //布控卡口及关联布控库信息

    MAPLAYOUTTASKINFO m_mapLayoutTaskInfo;                  //布控任务信息
    map<string, int> m_mapLayoutFaceUUIDInfo;           //布控库对应FaceUUID

    bool m_bPrintSnap;      //是否打印抓拍图片信息
};

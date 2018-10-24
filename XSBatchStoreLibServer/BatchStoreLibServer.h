#pragma once

#include "ConfigRead.h"
#include "mysql_acl.h"
#include "HttpServerAction.h"   //接收客户端推送重点信息
#include "ZBase64.h"
#include "AnalyseStore.h"
#include "GetFeature.h"
#include <iostream>
using namespace std;


class CBatchStoreLibServer
{
public:
    CBatchStoreLibServer(void);
public:
    ~CBatchStoreLibServer(void);
public:
    bool StartBatchStoreLibServer();
    bool StopBatchStoreLibServer();

private:
    bool Init();

    //初始化连接DB
    bool InitDB();
    //连接数据库查询重点库人脸信息
    bool GetStoreLibInfo();
    //初始化HTTP服务
    bool InitHttpServer(string sRedisIP, int nRedisPort);
    //初始化ST连接
    bool InitGetFeature();
    //初始化分析服务连接
    bool InitAnalyse();

    //HTTP客户端请求回调
    static void CALLBACK HttpClientRequestCallback(LPHTTPREQUEST pHttpRequest, void * pUser);
    //处理Http请求线程
    static DWORD WINAPI HTTPRequestParseThread(LPVOID lParam);
    void HTTPRequestParseAction();

    //HTTP请求增加人脸入ST库
    bool AddStoreLibFace(LPHTTPREQUEST pHttpRequest);
    //HTTP请求从ST库删除人脸
    bool DelStoreLibFace(LPHTTPREQUEST pHttpRequest);
    //HTTP请求从ST库删除重点库
    bool DelStoreLib(LPHTTPREQUEST pHttpRequest);
    //保存图片到本地磁盘
    bool SaveFileToDisk(unsigned int nStoreLibID, char * pImageBuf, int nLen, char * pFilePath);
    //保存增加的人脸信息到本地map
    bool AddStoreFaceToMap(std::string sFaceUUID, std::string sSTImageID, unsigned int nLayoutLibID);
    //从本地map删除人脸信息
    bool DelStoreFaceToMap(std::string sFaceUUID, unsigned int nLayoutLibID);
    //从本地map删除重点库, bDelLib: true: 删除重点库, false: 不删除
    bool DelStoreLibToMap(unsigned int nLayoutLibID, bool bDelLib = true);
    //从本地磁盘删除整个重点库图片, bDelDir: true: 删除文件夹, false: 不删除
    bool DelLibFaceFromDisk(unsigned int nStoreLibID, bool bDelDir = true);
    //获取错误码
    int GetErrorMsg(int nError, char * pMsg);
    //回应客户端
    void SendResponseMsg(SOCKET ClientSocket, int nEvent);
    LPSTOREFACEINFO GetFreeFaceInfo();
    void RecoverFaceInfo(LPSTOREFACEINFO pStoreImageInfo);

    //ST入库后回调入库结果
    static void CALLBACK ImageInfoCallback(LPSTOREFACEINFO pStoreImageInfo, void * pUser);

private:
     CRITICAL_SECTION m_Httpcs;
     CRITICAL_SECTION m_cs;
     HANDLE m_hStopEvent;           //服务停止事件
     CConfigRead * m_pConfigRead;   //配置文件读取

     char m_pServerIP[20];          //本地服务IP
     int m_nServerPort;             //本地服务Port

     CMysql_acl m_mysqltool;        //MySQL数据库操作

     MAPSTSERVERINFO m_mapSTServerInfo; //获取特征值服务器信息
     CGetFeature * m_pGetFeature;   //获取特征值操作对象
     CAnalyseStore * m_pAnalyseStore;//推送特征值到分析服务

     CHttpServerAction * m_pHttpServer; //HTTP服务操作对象
     LISTHTTPREQUEST m_listHttpRequest; //HTTP请求消息链表

     MAPSTORELIBINFO m_mapStoreLibInfo;//重点库及卡口与人脸图片信息

     LISTSTOREFACEINFO m_listStoreImageInfo; //保存收到的HTTP消息资源池

     char m_pProxyServerIP[20];
     int m_nProxyPort;
     LISTANALYSESERVERINFO m_listAnalyseServerInfo;

     SYSTEMTIME m_sysTime;
     char m_pSavePath[128];         //图片保存路径(由配置文件读取)
};

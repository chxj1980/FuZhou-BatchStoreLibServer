#pragma once

#include "DataDefine.h"
#include "ZeromqManage.h"
#include "LogRecorder.h"
class CAnalyseStore
{
public:
    CAnalyseStore(char pDisk);
    ~CAnalyseStore();
public:
    bool Init(char * pProxyServer, int nProxyPort, char * pServerID, char * pServerIP, LISTANALYSESERVERINFO listAnalyseServerInfo, int nQualityThreshold = 50);
    bool UnInit();
    static void ZeromqSubMsg(LPSUBMESSAGE pSubMessage, void * pUser);
    //增加库
    int AddKeyLib(int nLibID, int nLibType = 3);
    // 删除库
    int DelLib(int nLibID, int nLibType = 3);
    //增加特征值
    int AddFeature(LPSTOREFACEINFO pStoreImageInfo, char * pSavePath);
    //从重点库删除特征值
    int DelFeature(int nLibID,  const char * pFaceUUID);
    
    //清空库
    int ClearLib(int nLibID);

    //心跳线程, 保持zmq长时间不断
    static DWORD WINAPI HeartBeatThread(LPVOID lParam);
    void HeartBeatAction();
private:
    HANDLE m_hStopEvent;
    CZeromqManage * m_pZeromqManage;    //Zeromq管理
    char m_pSaveDisk[2];
    char m_pProxyServerIP[20];
    int m_nProxyPort;

    char m_pServerID[MAXLEN];
    char m_pServerIP[20];

    int m_nQualityThreshold;

    LISTANALYSESERVERINFO m_listAnalyseServerInfo;
};


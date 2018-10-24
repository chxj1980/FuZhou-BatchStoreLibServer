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
    //���ӿ�
    int AddKeyLib(int nLibID, int nLibType = 3);
    // ɾ����
    int DelLib(int nLibID, int nLibType = 3);
    //��������ֵ
    int AddFeature(LPSTOREFACEINFO pStoreImageInfo, char * pSavePath);
    //���ص��ɾ������ֵ
    int DelFeature(int nLibID,  const char * pFaceUUID);
    
    //��տ�
    int ClearLib(int nLibID);

    //�����߳�, ����zmq��ʱ�䲻��
    static DWORD WINAPI HeartBeatThread(LPVOID lParam);
    void HeartBeatAction();
private:
    HANDLE m_hStopEvent;
    CZeromqManage * m_pZeromqManage;    //Zeromq����
    char m_pSaveDisk[2];
    char m_pProxyServerIP[20];
    int m_nProxyPort;

    char m_pServerID[MAXLEN];
    char m_pServerIP[20];

    int m_nQualityThreshold;

    LISTANALYSESERVERINFO m_listAnalyseServerInfo;
};


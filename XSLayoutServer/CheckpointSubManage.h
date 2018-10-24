#pragma once

#include "ZeromqManage.h"
#include "DataDefine.h"
class CCheckpointSubManage
{
public:
    CCheckpointSubManage();
    ~CCheckpointSubManage();
public:
    bool Init(char * pProxyServerIP, int nProxyServerPort, LPSubMessageCallback pCallback, void * pUser);
    void UnInit();
    
    bool AddSubCheckpoint(char * pCheckpointID);
    bool DelSubCheckpoint(char * pCheckpointID);
private:
    bool InitZeromq();
    //Zeromq消息回调, 接收订阅卡口特征值信息
    static void CALLBACK ZeromqSubMsg(LPSUBMESSAGE pSubMessage, void * pUser);
    bool ParseZeromqJson(LPSUBMESSAGE pSubMessage);
private:
    char m_pProxyServerIP[20];          //代理服务IP
    int m_nProxyPort;                   //代理服务Port
    CZeromqManage * m_pZeromqManage;    //Zeromq订阅管理

    LPSubMessageCallback m_pCallback;
    void * m_pUser;

};


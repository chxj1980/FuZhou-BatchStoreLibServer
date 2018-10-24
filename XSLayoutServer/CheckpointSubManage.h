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
    //Zeromq��Ϣ�ص�, ���ն��Ŀ�������ֵ��Ϣ
    static void CALLBACK ZeromqSubMsg(LPSUBMESSAGE pSubMessage, void * pUser);
    bool ParseZeromqJson(LPSUBMESSAGE pSubMessage);
private:
    char m_pProxyServerIP[20];          //�������IP
    int m_nProxyPort;                   //�������Port
    CZeromqManage * m_pZeromqManage;    //Zeromq���Ĺ���

    LPSubMessageCallback m_pCallback;
    void * m_pUser;

};


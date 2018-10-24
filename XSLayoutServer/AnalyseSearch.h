#pragma once

#include "stdafx.h"
#include "ZeromqManage.h"

#define STRESPONSELEN 1024 * 1024

class CAnalyseSearch
{
public:
    CAnalyseSearch();
    ~CAnalyseSearch();
public:
    //��ʼ��, ����: ����IP:����Push�˿�:�������IP:�������Port:���Ĵ�����Ϣ:�ص�����:�ص�����
    bool Init(char * pLocalIP, int nLocalPushPort, int nLocalPullPort, LPSubMessageCallback pCallback, void * pUser);
    void UnInit();
    //Push����������Ϣ
    bool PushLayoutSearchMessage(LPSUBMESSAGE pSubMessage);
private:
    //��ʼ��Zeromq Push, Sub
    bool InitZeromq();
    //Zeromq��Ϣ�ص�, ���ն��Ŀ�������ֵ��Ϣ
    static void CALLBACK ZeromqSubMsg(LPSUBMESSAGE pSubMessage, void * pUser);
public:
    CRITICAL_SECTION m_cs;
    HANDLE m_hStopEvent;

    char m_pLocalIP[20];        //����IP
    int m_nLocalPushPort;       //�������Ͳ�������˿�
    int m_nLocalPullPort;       //���ؽ��ղ��ط�����Ϣ�˿�
    CZeromqManage * m_pZeromqManage;

    LPSubMessageCallback m_pImageCallback;
    void * m_pUser;
};


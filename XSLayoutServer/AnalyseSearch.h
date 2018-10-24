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
    //初始化, 参数: 本地IP:本地Push端口:代理服务IP:代理服务Port:订阅代理消息:回调函数:回调参数
    bool Init(char * pLocalIP, int nLocalPushPort, int nLocalPullPort, LPSubMessageCallback pCallback, void * pUser);
    void UnInit();
    //Push布控搜索消息
    bool PushLayoutSearchMessage(LPSUBMESSAGE pSubMessage);
private:
    //初始化Zeromq Push, Sub
    bool InitZeromq();
    //Zeromq消息回调, 接收订阅卡口特征值信息
    static void CALLBACK ZeromqSubMsg(LPSUBMESSAGE pSubMessage, void * pUser);
public:
    CRITICAL_SECTION m_cs;
    HANDLE m_hStopEvent;

    char m_pLocalIP[20];        //本地IP
    int m_nLocalPushPort;       //本地推送布控任务端口
    int m_nLocalPullPort;       //本地接收布控返回消息端口
    CZeromqManage * m_pZeromqManage;

    LPSubMessageCallback m_pImageCallback;
    void * m_pUser;
};


#pragma once
#include "stdafx.h"

class CRedisManage
{
public:
    CRedisManage();
    ~CRedisManage();
public:
    //��ʼ������Redis
    bool InitConnect(std::string sRedisIP, int nRedisPort);
    bool DisConnect();
    //��������֪ͨ
    bool PublishAlarmInfo(char * pKey, char * pPubMessage);
private:
    char m_pRedisAddr[32];                  //Redis��ַ
    acl::redis_client * m_pClient;
    acl::redis *        m_pRedisCmd;       //Redis��������
};


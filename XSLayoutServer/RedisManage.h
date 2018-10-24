#pragma once
#include "stdafx.h"

class CRedisManage
{
public:
    CRedisManage();
    ~CRedisManage();
public:
    //初始化连接Redis
    bool InitConnect(std::string sRedisIP, int nRedisPort);
    bool DisConnect();
    //发布报警通知
    bool PublishAlarmInfo(char * pKey, char * pPubMessage);
private:
    char m_pRedisAddr[32];                  //Redis地址
    acl::redis_client * m_pClient;
    acl::redis *        m_pRedisCmd;       //Redis命令处理对象
};


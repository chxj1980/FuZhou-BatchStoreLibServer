#include "stdafx.h"
#include "RedisManage.h"

extern CLogRecorder g_LogRecorder;
CRedisManage::CRedisManage()
{
    m_pClient = NULL;
    m_pRedisCmd = NULL;
}


CRedisManage::~CRedisManage()
{
}
bool CRedisManage::InitConnect(string sRedisIP, int nRedisPort)
{
    sprintf_s(m_pRedisAddr, "%s:%d", sRedisIP.c_str(), nRedisPort);
    acl::acl_cpp_init();
	acl::log::stdout_open(true);
    m_pClient = new acl::redis_client(m_pRedisAddr, 20, 10);
    m_pRedisCmd = new acl::redis(m_pClient);

    return true;
}
bool CRedisManage::DisConnect()
{
    if (NULL != m_pClient)
    {
        delete m_pClient;
        m_pClient = NULL;
    }
    if (NULL != m_pRedisCmd)
    {
        delete m_pRedisCmd;
        m_pRedisCmd = NULL;
    }
    return true;
}
bool CRedisManage::PublishAlarmInfo(char * pKey, char * pPubMessage)
{
    bool bRet = false;
    if (m_pRedisCmd->publish(pKey, pPubMessage, strlen(pPubMessage)) < 0)
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__,
            "****Error: publish Layout Alarm Failed,  LayoutFaceUUID[%s], Message [%s] .",
            pKey, pPubMessage);
        bRet = false;
    }
    else
    {
        g_LogRecorder.WriteInfoLogEx(__FUNCTION__,
            "publish Layout Alarm success,  LayoutFaceUUID[%s], Message [%s] .",
            pKey, pPubMessage);
        bRet = true;
    }
    return bRet;

    /*bool bRet = false;
    char pPublish[64] = { 0 };
    char pPublishMessage[128] = { 0 };
    sprintf_s(pPublish, sizeof(pPublish), "Layout.%s", sLayoutFaceUUID.c_str());
    sprintf_s(pPublishMessage, sizeof(pPublishMessage), "%s#%s#%s",
        pFaceImageInfo->pFaceUUID, pFaceImageInfo->pDeviceID, pFaceImageInfo->pImageTime);
    if (m_pRedisCmd->publish(pPublish, pPublishMessage, strlen(pPublishMessage)) < 0)
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__,
            "****Error: publish Layout Alarm Failed,  LayoutFaceUUID[%s], Message [%s] .",
            pPublish, pPublishMessage);
        bRet = false;
    }
    else
    {
        g_LogRecorder.WriteInfoLogEx(__FUNCTION__,
            "publish Layout Alarm success,  LayoutFaceUUID[%s], Message [%s] .",
            pPublish, pPublishMessage);
        bRet = true;
    }
*/
}

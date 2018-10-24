#include "StdAfx.h"
#include "STSDKManage.h"

CLogRecorder g_LogRecorder;
CSTSDKManage::CSTSDKManage(void)
{
    InitializeCriticalSection(&m_cs);
}
CSTSDKManage::~CSTSDKManage(void)
{
    DeleteCriticalSection(&m_cs);
}

int CSTSDKManage::Init(STSERVERINFO STServerInfo[], int nQuantity, int nThread)
{
    DWORD nBufferLenth = MAX_PATH;
    char szBuffer[MAX_PATH] = {0};
    DWORD dwRet = GetModuleFileNameA(NULL, szBuffer, nBufferLenth);
    if (dwRet == 0)
    {
        return false;
    }
    char *sPath = strrchr(szBuffer, '\\');
    memset(sPath, 0, strlen(sPath));
    string sFilePath = string(szBuffer) + "/Config/STSDKLib_config.properties";
#ifdef _DEBUG
    sFilePath = "./Config/STSDKLib_config.properties";
#endif
    g_LogRecorder.InitLogger(sFilePath.c_str(), "STSDKLibLogger", "STSDKLib");

    //生成资源
    for(int i = 0; i < nThread; i ++)
    {
        CSTSDKClient * pSTSDKClient = new CSTSDKClient;
        m_listSTClient.push_back(pSTSDKClient);

        pSTSDKClient->Init(STServerInfo, nQuantity);
    }
    return 0;
}
int CSTSDKManage::UnInit()
{
    list<CSTSDKClient *>::iterator it = m_listSTClient.begin();
    while(it != m_listSTClient.end())
    {
        delete *it;
        it = m_listSTClient.erase(it);
    }
    return 0;
}
int CSTSDKManage::OperationDB(const char * pDBName, int nOperationType, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    CSTSDKClient * pSTSDKClient = GetSTClient();
    if(NULL != pSTSDKClient)
    {
        nRet = pSTSDKClient->OperationDB(pDBName, nOperationType, pResponseMsg, nMsgLen, nType);
        PushClient(pSTSDKClient);
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 获取STSDKClient失败, 未找到可用资源.");

    }
    return nRet;
}
int CSTSDKManage::GetAllDBInfo(char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    CSTSDKClient * pSTSDKClient = GetSTClient();
    if(NULL != pSTSDKClient)
    {
        nRet = pSTSDKClient->GetAllDBInfo(pResponseMsg, nMsgLen, nType);
        PushClient(pSTSDKClient);
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 获取STSDKClient失败, 未找到可用资源.");

    }
    return nRet;
}
int CSTSDKManage::FaceQuality(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    CSTSDKClient * pSTSDKClient = GetSTClient();
    if(NULL != pSTSDKClient)
    {
        nRet = pSTSDKClient->FaceQuality(pImage, nLen, pResponseMsg, nMsgLen, nType);
        PushClient(pSTSDKClient);
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 获取STSDKClient失败, 未找到可用资源.");

    }
    return nRet;
}
int CSTSDKManage::SynAddImage(const char * pDBName, const char * pImage, int nLen, int nGetFeature,
    int nQualityThreshold, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    CSTSDKClient * pSTSDKClient = GetSTClient();
    if(NULL != pSTSDKClient)
    {
        nRet = pSTSDKClient->SynAddImage(pDBName, pImage, nLen, nGetFeature, nQualityThreshold, pResponseMsg, nMsgLen, nType);
        PushClient(pSTSDKClient);
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 获取STSDKClient失败, 未找到可用资源.");

    }
    return nRet;
}
int CSTSDKManage::SynAddMultipleImage(const char * pDBName, STIMAGEINFO STImageInfo[], int nNum, int nGetFeature,
    int nQualityThreshold, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    CSTSDKClient * pSTSDKClient = GetSTClient();
    if(NULL != pSTSDKClient)
    {
        nRet = pSTSDKClient->SynAddMultipleImage(pDBName, STImageInfo, nNum, nGetFeature, nQualityThreshold, pResponseMsg, nMsgLen, nType);
        PushClient(pSTSDKClient);
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 获取STSDKClient失败, 未找到可用资源.");

    }
    return nRet;
}
int CSTSDKManage::BatchGetFeature(STIMAGEINFO STImageInfo[], int nNum, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    CSTSDKClient * pSTSDKClient = GetSTClient();
    if (NULL != pSTSDKClient)
    {
        nRet = pSTSDKClient->BatchGetFeature(STImageInfo, nNum, pResponseMsg, nMsgLen, nType);
        PushClient(pSTSDKClient);
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 获取STSDKClient失败, 未找到可用资源.");

    }
    return nRet;
}
int CSTSDKManage::SynAddFeature(const char * pDBName, const char * pFeature, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    CSTSDKClient * pSTSDKClient = GetSTClient();
    if(NULL != pSTSDKClient)
    {
        nRet = pSTSDKClient->SynAddFeature(pDBName, pFeature, pResponseMsg, nMsgLen, nType);
        PushClient(pSTSDKClient);
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 获取STSDKClient失败, 未找到可用资源.");

    }
    return nRet;
}
int CSTSDKManage::GetImageByID(const char * pImageID, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    CSTSDKClient * pSTSDKClient = GetSTClient();
    if(NULL != pSTSDKClient)
    {
        nRet = pSTSDKClient->GetImageByID(pImageID, pResponseMsg, nMsgLen, nType);
        PushClient(pSTSDKClient);
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 获取STSDKClient失败, 未找到可用资源.");

    }
    return nRet;
}
int CSTSDKManage::DelImageByID(const char * pDBName, const char * pImageID, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    CSTSDKClient * pSTSDKClient = GetSTClient();
    if(NULL != pSTSDKClient)
    {
        nRet = pSTSDKClient->DelImageByID(pDBName, pImageID, pResponseMsg, nMsgLen, nType);
        PushClient(pSTSDKClient);
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 获取STSDKClient失败, 未找到可用资源.");

    }
    return nRet;
}
int CSTSDKManage::SearchImage(const char * pDBName, const char * pImage, int nLen, int nTopNum, double dbScore, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    CSTSDKClient * pSTSDKClient = GetSTClient();
    if(NULL != pSTSDKClient)
    {
        nRet = pSTSDKClient->SearchImage(pDBName, pImage, nLen, nTopNum, dbScore, pResponseMsg, nMsgLen, nType);
        PushClient(pSTSDKClient);
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 获取STSDKClient失败, 未找到可用资源.");

    }
    return nRet;
}
int CSTSDKManage::Verification(const char * pImage1, int nLen1, const char * pImage2, int nLen2, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    CSTSDKClient * pSTSDKClient = GetSTClient();
    if(NULL != pSTSDKClient)
    {
        nRet = pSTSDKClient->Verification(pImage1, nLen1, pImage2, nLen2, pResponseMsg, nMsgLen, nType);
        PushClient(pSTSDKClient);
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 获取STSDKClient失败, 未找到可用资源.");

    }
    return nRet;
}
int CSTSDKManage::SearchImageByFeature(const char * pDBName, const char * pFeature, int nLen, int nTopNum,
    double dbScore, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    CSTSDKClient * pSTSDKClient = GetSTClient();
    if(NULL != pSTSDKClient)
    {
        nRet = pSTSDKClient->SearchImageByFeature(pDBName, pFeature, nLen, nTopNum, dbScore, pResponseMsg, nMsgLen, nType);
        PushClient(pSTSDKClient);
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 获取STSDKClient失败, 未找到可用资源.");

    }
    return nRet;
}
int CSTSDKManage::GetAttribute(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    CSTSDKClient * pSTSDKClient = GetSTClient();
    if(NULL != pSTSDKClient)
    {
        nRet = pSTSDKClient->GetAttribute(pImage, nLen, pResponseMsg, nMsgLen, nType);
        PushClient(pSTSDKClient);
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 获取STSDKClient失败, 未找到可用资源.");

    }
    return nRet;
}
int CSTSDKManage::GetFeature(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    CSTSDKClient * pSTSDKClient = GetSTClient();
    if(NULL != pSTSDKClient)
    {
        nRet = pSTSDKClient->GetFeature(pImage, nLen, pResponseMsg, nMsgLen, nType);
        PushClient(pSTSDKClient);
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 获取STSDKClient失败, 未找到可用资源.");

    }
    return nRet;
}
int CSTSDKManage::GetDetail(char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    CSTSDKClient * pSTSDKClient = GetSTClient();
    if(NULL != pSTSDKClient)
    {
        nRet = pSTSDKClient->GetDetail(pResponseMsg, nMsgLen, nType);
        PushClient(pSTSDKClient);
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 获取STSDKClient失败, 未找到可用资源.");

    }
    return nRet;
}
CSTSDKClient * CSTSDKManage::GetSTClient()
{
    CSTSDKClient * pSTClient = NULL;
    int nNum = 0;
    do 
    {
        EnterCriticalSection(&m_cs);
        if(m_listSTClient.size() > 0)
        {
            pSTClient = m_listSTClient.front();
            m_listSTClient.pop_front();
        }
        LeaveCriticalSection(&m_cs);
        if(NULL == pSTClient)
        {
            Sleep(200);
            nNum ++;
        }
        else
        {
            break;
        }
    }while(nNum < 10);

    return pSTClient;
}
void CSTSDKManage::PushClient(CSTSDKClient * pSTClient)
{
    EnterCriticalSection(&m_cs);
    m_listSTClient.push_back(pSTClient);
    LeaveCriticalSection(&m_cs);
}
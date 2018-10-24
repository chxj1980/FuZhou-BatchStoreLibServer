#include "StdAfx.h"
#include "AnalyseStore.h"


extern CLogRecorder g_LogRecorder;
CAnalyseStore::CAnalyseStore(char pDisk)
{
    ZeroMemory(m_pProxyServerIP, sizeof(m_pProxyServerIP));
    m_nProxyPort = 0;
    m_pZeromqManage = NULL;
    m_pSaveDisk[0] = pDisk;
    m_pSaveDisk[1] = '\0';
    m_hStopEvent = CreateEvent(NULL, true, false, NULL);
}
CAnalyseStore::~CAnalyseStore()
{
    Sleep(100);
    CloseHandle(m_hStopEvent);
}
bool CAnalyseStore::Init(char * pProxyServer, int nProxyPort, char * pServerID, char * pServerIP, LISTANALYSESERVERINFO listAnalyseServerInfo, int nQualityThreshold)
{
    m_listAnalyseServerInfo = listAnalyseServerInfo;
    strcpy_s(m_pServerID, sizeof(m_pServerID), pServerID);
    strcpy_s(m_pServerIP, sizeof(m_pServerIP), pServerIP);
    strcpy_s(m_pProxyServerIP, sizeof(m_pProxyServerIP), pProxyServer);
    m_nProxyPort = nProxyPort;
    m_nQualityThreshold = nQualityThreshold;

    if (NULL == m_pZeromqManage)
    {
        m_pZeromqManage = new CZeromqManage;
    }
    if (!m_pZeromqManage->InitSub(NULL, 0, m_pProxyServerIP, m_nProxyPort, ZeromqSubMsg, this))
    {
        printf("****Error: 订阅[%s:%d:%s]失败!", m_pProxyServerIP, m_nProxyPort, pServerID);
        return false;
    }
    m_pZeromqManage->AddSubMessage(pServerID);

    if (!m_pZeromqManage->InitPub(NULL, 0, m_pProxyServerIP, m_nProxyPort + 1))
    {
        printf("****Error: 初始化发布失败!");
        return false;
    }
    CreateThread(NULL, 0, HeartBeatThread, this, NULL, 0);   //zmq心跳

    return true;
}
bool CAnalyseStore::UnInit()
{
    if (NULL != m_pZeromqManage)
    {
        m_pZeromqManage->UnInit();
        delete m_pZeromqManage;
        m_pZeromqManage = NULL;
    }

    return true;
}
DWORD WINAPI CAnalyseStore::HeartBeatThread(LPVOID lParam)
{
    CAnalyseStore * pThis = (CAnalyseStore*)lParam;
    pThis->HeartBeatAction();
    return 0;
}
//http请求信息处理线程
void CAnalyseStore::HeartBeatAction()
{
    LPSUBMESSAGE pSubMsg = new SUBMESSAGE;
    strcpy(pSubMsg->pHead, HEARTBEATMSG);
    strcpy(pSubMsg->pOperationType, HEARTBEATMSG);
    strcpy(pSubMsg->pSource, m_pServerID);
    strcpy(pSubMsg->pSubJsonValue, HEARTBEATMSG);
    while (WAIT_TIMEOUT == WaitForSingleObject(m_hStopEvent, 1000 * 600))
    {
        m_pZeromqManage->PubMessage(pSubMsg);
    }

    return;
}
//Zeromq消息回调
void CAnalyseStore::ZeromqSubMsg(LPSUBMESSAGE pSubMessage, void * pUser)
{
    CAnalyseStore * pThis = (CAnalyseStore *)pUser;
    if (string(pSubMessage->pOperationType) == HEARTBEATMSG)
    {
        printf("Recv Heartbeat From %s.\n", pSubMessage->pSource);
    }
    //printf("%s\n", pSubMessage->pSubJsonValue);
}
int CAnalyseStore::AddKeyLib(int nLibID, int nLibType)
{
    char pLibID[MAXLEN] = { 0 };
    sprintf_s(pLibID, sizeof(pLibID), "%dkeylib", nLibID);

    LPSUBMESSAGE pPubMessage = new SUBMESSAGE;
    strcpy(pPubMessage->pOperationType, COMMANDADDLIB);
    strcpy(pPubMessage->pSource, m_pServerID);

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType&allocator = document.GetAllocator();
    rapidjson::Value array(rapidjson::kArrayType);
    document.AddMember(JSONLIBID, rapidjson::StringRef(pLibID), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    pPubMessage->sPubJsonValue = buffer.GetString();

    for (LISTANALYSESERVERINFO::iterator it = m_listAnalyseServerInfo.begin();
        it != m_listAnalyseServerInfo.end(); it++)
    {
        if (2 == nLibType && (*it)->nServerType != 2) //如新建核查库, 则只往核查分析服务发布; 如新建布控库, 则向所有核查与布控分析服务发布
        {
            continue;
        }
        strcpy(pPubMessage->pHead, (*it)->pServerID);
        
        m_pZeromqManage->PubMessage(pPubMessage);
    }
    
    delete pPubMessage;
    pPubMessage = NULL;

    return 0;
}
int CAnalyseStore::DelLib(int nLibID, int nLibType)
{
    //重点库名
    char pLibID[MAXLEN] = { 0 };
    sprintf_s(pLibID, sizeof(pLibID), "%dkeylib", nLibID);

    LPSUBMESSAGE pPubMessage = new SUBMESSAGE;

    strcpy(pPubMessage->pOperationType, COMMANDDELLIB);
    strcpy(pPubMessage->pSource, m_pServerID);

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType&allocator = document.GetAllocator();
    rapidjson::Value array(rapidjson::kArrayType);
    document.AddMember(JSONLIBID, rapidjson::StringRef(pLibID), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    pPubMessage->sPubJsonValue = buffer.GetString();

    for (LISTANALYSESERVERINFO::iterator it = m_listAnalyseServerInfo.begin();
        it != m_listAnalyseServerInfo.end(); it++)
    {
        if (2 == nLibType && (*it)->nServerType != 2) //如删除核查库, 则只往核查分析服务发布; 如删除布控库, 则向所有核查与布控分析服务发布
        {
            continue;
        }
        strcpy(pPubMessage->pHead, (*it)->pServerID);

        m_pZeromqManage->PubMessage(pPubMessage);
    }

    delete pPubMessage;
    pPubMessage = NULL;

    return 0;
}
//增加特征值
int CAnalyseStore::AddFeature(LPSTOREFACEINFO pStoreImageInfo, char * pSavePath)
{
    //重点库名
    char pLibID[MAXLEN] = { 0 };
    sprintf_s(pLibID, sizeof(pLibID), "%dkeylibedit", pStoreImageInfo->nStoreLibID);

    LPSUBMESSAGE pPubMessage = new SUBMESSAGE;
    strcpy(pPubMessage->pHead, pLibID);
    strcpy(pPubMessage->pOperationType, COMMANDADD);
    strcpy(pPubMessage->pSource, m_pServerID);
    
    LPSUBMESSAGE pKafkaPubMessage = new SUBMESSAGE;
    LISTPUSHSTIMAGEINFO::iterator it = pStoreImageInfo->listPushSTImageInfo.begin();
    for (; it != pStoreImageInfo->listPushSTImageInfo.end(); it++)
    {
        if ((*it)->nFaceQuality <= 100 && (*it)->nFaceQuality >= m_nQualityThreshold)
        {
            rapidjson::Document document;
            document.SetObject();
            rapidjson::Document::AllocatorType&allocator = document.GetAllocator();

            document.AddMember(JSONFACEUUID, rapidjson::StringRef((*it)->pFaceUUID), allocator);
            document.AddMember(JSONFEATURE, rapidjson::StringRef((*it)->pFeature), allocator);
            document.AddMember(JSONDRIVE, rapidjson::StringRef(m_pSaveDisk), allocator);
            document.AddMember(JSONSERVERIP, rapidjson::StringRef(m_pServerIP), allocator);

            string sPath(pSavePath);
            sPath.erase(1, 1);
            sprintf((*it)->pFaceURL, "http://%s:80/%s/%d/%s.jpg", m_pServerIP, sPath.c_str(), pStoreImageInfo->nStoreLibID, (*it)->pFaceUUID);
            document.AddMember(JSONFACEURL, rapidjson::StringRef((*it)->pFaceURL), allocator);

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.Accept(writer);
            pPubMessage->sPubJsonValue = buffer.GetString();

            m_pZeromqManage->PubMessage(pPubMessage);

            //向zmq代理服务额外推送布控图片信息, 提供给Kafka服务, 再推送到公安网
            {
                rapidjson::Document document;
                document.SetObject();
                rapidjson::Document::AllocatorType&allocator = document.GetAllocator();

                char pID[10] = { 0 };
                itoa(pStoreImageInfo->nStoreLibID, pID, 10);
                document.AddMember("layoutlib_id", rapidjson::StringRef(pID), allocator);
                document.AddMember("layoutlib_name", rapidjson::StringRef(pStoreImageInfo->pStoreLibName), allocator);
                document.AddMember("layoutfaceuuid", rapidjson::StringRef((*it)->pFaceUUID), allocator);
                document.AddMember("layoutface_url", rapidjson::StringRef((*it)->pFaceURL), allocator);
                document.AddMember("layoutface_info", rapidjson::StringRef((*it)->pImageName), allocator);

                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                document.Accept(writer);
                pKafkaPubMessage->sPubJsonValue = buffer.GetString();

                strcpy(pKafkaPubMessage->pHead, "KAFKALAYOUT");
                strcpy(pKafkaPubMessage->pOperationType, "LayoutLibImageInfo");
                strcpy(pKafkaPubMessage->pSource, m_pServerID);
                m_pZeromqManage->PubMessage(pKafkaPubMessage);
            }

        }
    }

    delete pPubMessage;
    pPubMessage = NULL;

    delete pKafkaPubMessage;
    pKafkaPubMessage = NULL;

    return 0;
}
//从重点库删除特征值
int CAnalyseStore::DelFeature(int nLibID, const char * pFaceUUID)
{
    //重点库名
    char pLibID[MAXLEN] = { 0 };
    sprintf_s(pLibID, sizeof(pLibID), "%dkeylibedit", nLibID);

    LPSUBMESSAGE pPubMessage = new SUBMESSAGE;
    strcpy(pPubMessage->pHead, pLibID);
    strcpy(pPubMessage->pOperationType, COMMANDDEL);
    strcpy(pPubMessage->pSource, m_pServerID);

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType&allocator = document.GetAllocator();
    rapidjson::Value array(rapidjson::kArrayType);
    document.AddMember(JSONFACEUUID, rapidjson::StringRef(pFaceUUID), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    pPubMessage->sPubJsonValue = buffer.GetString();

    m_pZeromqManage->PubMessage(pPubMessage);

    delete pPubMessage;
    pPubMessage = NULL;

    return 0;
}
int CAnalyseStore::ClearLib(int nLibID)
{
    //重点库名
    char pLibID[MAXLEN] = { 0 };
    sprintf_s(pLibID, sizeof(pLibID), "%dkeylibedit", nLibID);

    LPSUBMESSAGE pPubMessage = new SUBMESSAGE;
    strcpy(pPubMessage->pHead, pLibID);

    strcpy(pPubMessage->pOperationType, COMMANDCLEAR);
    strcpy(pPubMessage->pSource, m_pServerID);

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType&allocator = document.GetAllocator();
    rapidjson::Value array(rapidjson::kArrayType);
    document.AddMember(JSONLIBID, rapidjson::StringRef(pLibID), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    pPubMessage->sPubJsonValue = buffer.GetString();

    m_pZeromqManage->PubMessage(pPubMessage);

    delete pPubMessage;
    pPubMessage = NULL;

    return 0;
}
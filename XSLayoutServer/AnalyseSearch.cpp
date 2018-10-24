#include "stdafx.h"
#include "AnalyseSearch.h"

extern CLogRecorder g_LogRecorder;
CAnalyseSearch::CAnalyseSearch()
{
    m_hStopEvent = CreateEvent(NULL, true, false, NULL);
    InitializeCriticalSection(&m_cs);

    ZeroMemory(m_pLocalIP, sizeof(m_pLocalIP));
    m_nLocalPushPort = 0;
    m_nLocalPullPort = 0;
    m_pZeromqManage = NULL;
}
CAnalyseSearch::~CAnalyseSearch()
{
    CloseHandle(m_hStopEvent);
    DeleteCriticalSection(&m_cs);
}
bool CAnalyseSearch::Init(char * pLocalIP, int nLocalPushPort, int nLocalPullPort, LPSubMessageCallback pCallback, void * pUser)
{
    strcpy_s(m_pLocalIP, sizeof(m_pLocalIP), pLocalIP);
    m_nLocalPushPort = nLocalPushPort;
    m_nLocalPullPort = nLocalPullPort;

    if (!InitZeromq())
    {
        return false;
    }
    
    m_pImageCallback = pCallback;
    m_pUser = pUser;
    return true;
}
void CAnalyseSearch::UnInit()
{
    SetEvent(m_hStopEvent);
    Sleep(100);
    if (NULL != m_pZeromqManage)
    {
        m_pZeromqManage->UnInit();
        delete m_pZeromqManage;
        m_pZeromqManage = NULL;
    }
}
bool CAnalyseSearch::InitZeromq()
{
    if (NULL == m_pZeromqManage)
    {
        m_pZeromqManage = new CZeromqManage;
        if (!m_pZeromqManage->InitPush(m_pLocalIP, m_nLocalPushPort, NULL, 0))
        {
            g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error: InitPush[%s:%d] Failed.", m_pLocalIP, m_nLocalPushPort);
            return false;
        }

        if (!m_pZeromqManage->InitPull(m_pLocalIP, m_nLocalPullPort, NULL, 0, ZeromqSubMsg, this))
        {
            g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error: CAnalyseSearch::InitPull Failed.");
            return false;
        }
    }
    return true;
}
bool CAnalyseSearch::PushLayoutSearchMessage(LPSUBMESSAGE pSubMessage)
{
    if (!m_pZeromqManage->PushMessage(pSubMessage))
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: Push Message Failed!");
        return false;
    }
    else
    {
        //g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "发布搜索任务[%s][%s][%s].",
            //pSubMessage->pHead, pSubMessage->pDeviceID, pSubMessage->pFaceUUID);
    }

    return true;
}
void CAnalyseSearch::ZeromqSubMsg(LPSUBMESSAGE pSubMessage, void * pUser)
{
    CAnalyseSearch * pThis = (CAnalyseSearch *)pUser;
    pThis->m_pImageCallback(pSubMessage, pThis->m_pUser);
}


#include "stdafx.h"
#include "PostLayoutAlarm.h"

CPostLayoutAlarm * g_pThis;
extern CLogRecorder g_LogRecorder;
CPostLayoutAlarm::CPostLayoutAlarm()
{
    m_pTCPSocket = NULL;
    g_pThis = this;

    m_hConnectST = CreateEvent(NULL, true, false, NULL);
    m_hResponseEvent = CreateEvent(NULL, true, false, NULL);
}


CPostLayoutAlarm::~CPostLayoutAlarm()
{
    if (NULL != m_pTCPSocket)
    {
        delete m_pTCPSocket;
        m_pTCPSocket = NULL;
    }
    CloseHandle(m_hResponseEvent);
    CloseHandle(m_hConnectST);
}
bool CPostLayoutAlarm::InitHttpClient(string sAlarmServerIP, int nAlarmServerPort)
{
    m_pTCPSocket = new CTCPIOCPSocket;
    if (!m_pTCPSocket->InitSocket("", 0, MSGCALLBACK))
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error: 本地TCP Socket初始化失败!");
        return false;
    }
    m_pTCPSocket->ConnectServer(sAlarmServerIP, nAlarmServerPort);
    sprintf_s(m_pSTAddr, sizeof(m_pSTAddr), "%s:%d", sAlarmServerIP.c_str(), nAlarmServerPort);
    return true;
}
void CPostLayoutAlarm::StopWork()
{
    SetEvent(m_hConnectST);
    SetEvent(m_hResponseEvent);
    m_pTCPSocket->StopSocket();
    return;
}

int CPostLayoutAlarm::SendLayoutAlarm(LPLAYOUTALARM pLayoutAlarm, char * pResponseMsg, int * nMsgLen)
{
    int nRet = 0;
    char pURL[64] = { 0 };
    sprintf_s(pURL, sizeof(pURL), "/Layout/Alarm");
    HttpConnection HttpInfo;
    HttpInfo.SetAddr(m_pSTAddr);
    HttpInfo.SetURL(pURL);
    HttpInfo.setRequestMethod(HL_HTTP_POST);
    HttpInfo.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
    HttpInfo.setRequestProperty("Cache-Control", "no-cache");
    HttpInfo.setRequestProperty("Pragma", "no-cache");
    HttpInfo.setRequestProperty("User-Agent", "XSLayoutServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("Accept", "text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");

    Json::Value json_Body;
    Json::FastWriter fast_writer;
    json_Body["LayoutFaceUUID"] = Json::Value(pLayoutAlarm->pLayoutFaceUUID);
    json_Body["Checkpoint"] = Json::Value(pLayoutAlarm->pCheckpoint);
    json_Body["FaceUUID"] = Json::Value(pLayoutAlarm->pFaceUUID);
    json_Body["Score"] = Json::Value(pLayoutAlarm->nScore);
    json_Body["Time"] = Json::Value(pLayoutAlarm->pTime);
    string sBody = fast_writer.write(json_Body);

    char pBodyLen[12] = { 0 };
    sprintf_s(pBodyLen, sizeof(pBodyLen), "%d", sBody.size());
    HttpInfo.setRequestProperty("Content-Length", pBodyLen);

    string sHttp = HttpInfo.GetHttpHead();
    if (WAIT_OBJECT_0 == WaitForSingleObject(m_hConnectST, 1000 * 2))
    {
        ResetEvent(m_hResponseEvent);
        m_pTCPSocket->SendData((char*)sHttp.c_str(), sHttp.size());
        m_pTCPSocket->SendData((char*)sBody.c_str(), sBody.size());

        if (WAIT_TIMEOUT == WaitForSingleObject(m_hResponseEvent, 1000 * RESOVERTIME))
        {
            nRet = STRESPOVERTIME;
        }
        else
        {
            if (*nMsgLen <= m_sResponseMsg.size())
            {
                nRet = RESPONSEMSGLENFAILED;
            }
            else
            {
                strcpy_s(pResponseMsg, *nMsgLen, m_sResponseMsg.c_str());
                *nMsgLen = m_sResponseMsg.size();
            }
        }
    }
    else
    {
        nRet = DISCONNECTSERVER;
    }

    return nRet;
}
void CPostLayoutAlarm::MSGCALLBACK(SOCKET RemoteSocket, SOCKADDR_IN RemoteAddr, string sMsg, int nEvent)
{
    string sRemoteIP = inet_ntoa(RemoteAddr.sin_addr);
    int nRemotePort = ntohs(RemoteAddr.sin_port);
    switch (nEvent)
    {
    case 0:
        g_pThis->ParseMessage(RemoteSocket, RemoteAddr, sMsg);
        break;
    case 1:
        //g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "Success: 与服务[%s:%d]连接成功.", sRemoteIP.c_str(), nRemotePort);
        SetEvent(g_pThis->m_hConnectST);
        break;
    case 2:
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "***Warning: 与服务[%s:%d]断开连接...", sRemoteIP.c_str(), nRemotePort);
        ResetEvent(g_pThis->m_hConnectST);
        break;
    default:
        break;
    }
}
void CPostLayoutAlarm::ParseMessage(SOCKET ClientSocket, SOCKADDR_IN RemoteAddr, string RecvBuf)
{
    m_sResponseMsg = RecvBuf;
    SetEvent(m_hResponseEvent);
}
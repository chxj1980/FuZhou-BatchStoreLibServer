#include "stdafx.h"
#include "HTTPSocket.h"


extern CLogRecorder g_LogRecorder;
unsigned int CHTTPSocket::m_nNum = 1;
CHTTPSocket::CHTTPSocket()
{
    m_nRecvSize = 0;
    m_Socket = INVALID_SOCKET;
    m_pHttpMsg = new HTTPMSG;
    m_nCurrentNum = m_nNum++;

    m_nServerPort = 0;
}


CHTTPSocket::~CHTTPSocket()
{
    if (INVALID_SOCKET == m_Socket)
    {
        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
    }
    if (NULL != m_pHttpMsg)
    {
        delete m_pHttpMsg;
        m_pHttpMsg = NULL;
    }
}
bool CHTTPSocket::InitConnect(const char * pIP, unsigned int nPort)
{
    bool bRet = false;
    if(m_nServerPort == 0)
    {
        strcpy_s(m_pServerIP, sizeof(m_pServerIP), pIP);
        m_nServerPort = nPort;
    }

    m_Socket = socket(AF_INET, SOCK_STREAM, 0);
    m_addrSTServer.sin_addr.S_un.S_addr = inet_addr(m_pServerIP);
    m_addrSTServer.sin_family = AF_INET;
    m_addrSTServer.sin_port = htons(m_nServerPort);
    int nRet = connect(m_Socket, (SOCKADDR*)&m_addrSTServer, sizeof(m_addrSTServer));
    if (nRet == SOCKET_ERROR)
    {
        int nError = GetLastError();
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__,
            "****Error: TCP���ӷ���[%s:%d]ʧ��, Error: %d.", pIP, nPort, nError);
    }
    else
    {
        bRet = true;
    }
    closesocket(m_Socket);
    m_Socket = INVALID_SOCKET;
    return bRet;
}
bool CHTTPSocket::ReConnectSTServer()
{
    if(m_Socket != INVALID_SOCKET)
    {
        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
    }
    m_Socket = socket(AF_INET, SOCK_STREAM, 0);
    int nRet = connect(m_Socket, (SOCKADDR*)&m_addrSTServer, sizeof(m_addrSTServer));
    if (nRet == SOCKET_ERROR)
    {
        int nError = GetLastError();
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__,
            "****Error: TCP���ӷ���[%s:%d]ʧ��, Error: %d.", m_pServerIP, m_nServerPort, nError);
        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
        return false;
    }
    else
    {
        return true;
    }
}
bool CHTTPSocket::SendMsg(const char * pMsg, unsigned int nSize)
{
    bool bRet = false;
    if (ReConnectSTServer())
    {
        int nStatus = send(m_Socket, pMsg, nSize, 0);
        if (nStatus == SOCKET_ERROR)
        {
            int nError = GetLastError();
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__,
                "***Warning: TCP send Failed, �����ַ[%s:%d], Error: %d.", m_pServerIP, m_nServerPort, nError);
        }
        else
        {
            bRet = true;
        }
    }
    else
    {
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "����ST����[%s:%d]ʧ��.", m_pServerIP, m_nServerPort);
    }
    
    return bRet;
}
int CHTTPSocket::RecvMsg(char * pRecvMsg, unsigned int * nSize)
{
    int nRet = -1;
    if (INVALID_SOCKET != m_Socket)
    {
        fd_set set;
        
        timeval outTime;
        outTime.tv_sec = 0;
        outTime.tv_usec = 50;
        bool bRecvEnd = false;

        m_pHttpMsg->Init();

        while (!bRecvEnd)
        {
            FD_ZERO(&set);
            FD_SET(m_Socket, &set);
            int nSelRet = select(NULL, &set, NULL, NULL, &outTime);
            if (nSelRet == 0)
            {
                time_t tCurTime = time(&tCurTime);
                if (tCurTime - m_pHttpMsg->tRecvTime > RESOVERTIME - 1)
                {
                    g_LogRecorder.WriteWarnLogEx(__FUNCTION__,
                        "***Warning: ��Ϣ��ʱ%d��, ɾ��.\r\n", RESOVERTIME - 1);
                    *nSize = 0;
                    nRet = STSDK_STRESPOVERTIME;
                    break;
                }
                else
                {
                    continue;
                }
            }
            else if (nSelRet < 0)
            {
                g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error: select ����, �˳�Recv, Error: %d.", GetLastError());
                break;
            }

            m_nRecvSize = 4096;
            int nStatus = recv(m_Socket, m_pRecvMsg, m_nRecvSize, 0);
            if (nStatus == SOCKET_ERROR)
            {
                int nError = GetLastError();
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: recv Failed, Error: %d.", nError);
                if (10054 == nError || 10053 == nError)
                {
                    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "Զ������ǿ�ƹر�����...");
                    closesocket(m_Socket);
                    InitConnect(m_pServerIP, m_nServerPort);
                    nRet = STSDK_SOCKETCLOSE;
                    break;
                }
            }
            else
            {
                int nReturn = ParseHTTPMsg(m_pRecvMsg, nStatus);
                if (nReturn == 1)   //HTTP��Ϣ���������
                {
                    bRecvEnd = true;
                    break;
                }
                else if (nReturn == 0)  //HTTP��Ϣδ������, ��Ҫ�ٴν���
                {
                    continue;
                }
                else if (nReturn < 0)   //����HTTP��Ϣ�д���, ���ٽ���
                {
                    nRet = nReturn;
                    break;
                }
            }
        }

        if (bRecvEnd)
        {
            string sHTTPMsg = m_pHttpMsg->sHttpBody;
            if (*nSize < sHTTPMsg.size())
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, 
                    "����Socket�ַ�������[%d]����, ����HTTP��Ϣ����[%d].", nSize, sHTTPMsg.size());
                nRet = STSDK_RESPONSEMSGLENFAILED;
            }
            else
            {
                strcpy_s(pRecvMsg, *nSize, sHTTPMsg.c_str());
                *nSize = sHTTPMsg.size();
                nRet = 0;
            }
        }
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: SOCKETδ��ʼ��.");
        nRet = STSDK_SOCKETINVALID;
    }

    closesocket(m_Socket);
    m_Socket = INVALID_SOCKET;
    return nRet;
}
int CHTTPSocket::ParseHTTPMsg(const char * pMsg, unsigned int nSize)
{
    int nRet = 0;
    if(nSize == 0)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: ����HTTP��Ӧ��ϢΪ��.");
        return STSDK_RECVMSGZERO;
    }
    else
    {
        string sRecvMsg(pMsg, nSize);
        if (m_pHttpMsg->sHttpHead == "")
        {
            time(&m_pHttpMsg->tRecvTime);
            size_t nPos = sRecvMsg.find("Content-Length:");
            if (nPos == string::npos)
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__,
                    "***Warning: �յ�HTTP��Ϣ, ��Content-Length�ֶ�.\r\n%s", pMsg);
                return STSDK_HTTPMSGFORMATWRONG;
            }
            else
            {
                nPos += strlen("Content-Length:");
                size_t nPos2 = sRecvMsg.find("\r\n", nPos);
                if (nPos2 == string::npos)
                {
                    g_LogRecorder.WriteWarnLogEx(__FUNCTION__,
                        "***Warning: �յ�HTTP��Ϣ, Content-Length�ֶκ�û�ҵ����лس���.\r\n%s", pMsg);
                    return STSDK_HTTPMSGFORMATWRONG;
                }

                string sBodyLen(sRecvMsg, nPos, nPos2 - nPos);
                int nBodyLen = atoi(sBodyLen.c_str());

                nPos = sRecvMsg.find("\r\n\r\n");
                if (nPos == string::npos)
                {
                    g_LogRecorder.WriteWarnLogEx(__FUNCTION__,
                        "***Warning: �յ�HTTP��Ϣ, δ�ҵ��ֽ��.\r\n%s", pMsg);
                    return STSDK_HTTPMSGFORMATWRONG;
                }
                nPos += 4;
                string sHead(sRecvMsg, 0, nPos);

                string sHttpBody(sRecvMsg, nPos, nSize - nPos);
                int nCurBodyLen = sHttpBody.size();
                if (nCurBodyLen >= nBodyLen)
                {
                    m_pHttpMsg->sHttpHead = sHead;
                    m_pHttpMsg->sHttpBody = sHttpBody;
                    nRet = 1;
                }
                else
                {
                    m_pHttpMsg->sHttpHead = sHead;
                    m_pHttpMsg->nBodyLen = atoi(sBodyLen.c_str());
                    if (nSize > nPos)
                    {
                        m_pHttpMsg->sHttpBody = sHttpBody;
                        m_pHttpMsg->nCurBodyLen = nCurBodyLen;
                        nRet = 0;
                    }
                }
            }
        }
        else
        {
            m_pHttpMsg->sHttpBody += sRecvMsg;
            m_pHttpMsg->nCurBodyLen += nSize;
            if (m_pHttpMsg->nCurBodyLen >= m_pHttpMsg->nBodyLen)
            {
                nRet = 1;
            }
            else
            {
                nRet = 0;
            }
        }
    }
    
    return nRet;
}
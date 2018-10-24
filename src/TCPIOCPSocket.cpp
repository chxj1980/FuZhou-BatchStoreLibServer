#include "StdAfx.h"
#include "LogRecorder.h"
#include "TCPIOCPSocket.h"

extern CLogRecorder g_LogRecorder;
CTCPIOCPSocket::CTCPIOCPSocket(void)
{
    m_LocalSocket = INVALID_SOCKET ;
    m_sLocalIP = "";
    m_nLocalPort = 0;
    m_bStopSocket = false;
    m_hStopEvent = CreateEvent(NULL, true, false, NULL);
    m_pMsgCallback = NULL;
    m_nConnectServerFailed = 0;
    InitializeCriticalSection(&m_cs);
}

CTCPIOCPSocket::~CTCPIOCPSocket(void)
{
    CloseHandle(m_hStopEvent);
    DeleteCriticalSection(&m_cs);
}
bool CTCPIOCPSocket::InitSocket(string sLocalIP, int nLocalPort, LPMSGCALLBACK pMsgCallback, bool bIsServer)
{
    m_sLocalIP = sLocalIP;
    m_nLocalPort = nLocalPort;
    m_pMsgCallback = pMsgCallback;
    m_bIsServer = bIsServer;
    WSADATA     wsaData;
    if ((WSAStartup(0x0202, &wsaData)) != 0)
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d].: WSAStartup Failed", GetLastError());
        return false;
    }
    if ((m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL) 
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d]: CreateIoCompletionPort Failed.", GetLastError());
        return false;
    }

    GetSystemInfo(&SystemInfo);
    for(DWORD i = 0; i < SystemInfo.dwNumberOfProcessors; i++)
    {
        HANDLE ThreadHandle;
        if ((ThreadHandle = CreateThread(NULL, 0, ServerWorkerThread, this,                
            0, NULL)) == NULL)
        {
            g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d]: CreateThread Failed.", GetLastError());
            return false;
        }
        CloseHandle(ThreadHandle);
    }
    CreateThread(NULL, 0, DestroyResourceThread, this, NULL, 0);     //开启资源销毁线程
    if(m_bIsServer)
    {
        if ((m_LocalSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
        {
            g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d]: WSASocket() Failed.", WSAGetLastError());
            return false;
        } 

        //传入端口不为0时, 如传入IP为空, 则先取得本机IP, 再绑定地址; 否则返回失败.
        if(m_nLocalPort != 0)
        {
            if(m_sLocalIP.size() == 0)
            {
                PHOSTENT hostinfo; 
                char name[155];
                string sHostIP;
                if( gethostname ( name, sizeof(name)) == 0) 
                { 
                    if((hostinfo = gethostbyname(name)) != NULL) 
                    { 
                        sHostIP = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);
                    }
                }
                m_sLocalIP = sHostIP;
            }

            m_LocalAddr.sin_family = AF_INET;
            m_LocalAddr.sin_addr.s_addr = inet_addr(m_sLocalIP.c_str());
            m_LocalAddr.sin_port = htons(m_nLocalPort);

            if (bind(m_LocalSocket, (PSOCKADDR) &m_LocalAddr, sizeof(m_LocalAddr)) == SOCKET_ERROR)
            {
                g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d]: 绑定Socket失败, Address[%s:%d].", 
                    WSAGetLastError(), m_sLocalIP.c_str(), m_nLocalPort);
                return false;
            }
        }
        else
        {
            g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error: 作为服务端未传入指定Port.");
            return false;
        }
        CreateThread(NULL, 0, TCPStreamThread, this, NULL, 0);     //开启TCP服务线程
    }
    return true;
}
DWORD WINAPI CTCPIOCPSocket::TCPStreamThread(LPVOID lParam)
{
    CTCPIOCPSocket * pThis = (CTCPIOCPSocket*)lParam;
    pThis->RunAccept();
    return 0;
}
bool CTCPIOCPSocket::RunAccept()
{  
    int nRecvBuf = 1024 * 1024 * 5;
    setsockopt(m_LocalSocket, SOL_SOCKET, SO_SNDBUF, (const char * )&nRecvBuf, sizeof(int));
    if (listen(m_LocalSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d]: listen() Failed.", WSAGetLastError());
        return false;
    }

    while(!m_bStopSocket)
    {   
        SOCKADDR_IN RemoteAddr;
        int nLen = sizeof(RemoteAddr);
        SOCKET RemoteSocket = WSAAccept(m_LocalSocket, (SOCKADDR*)&RemoteAddr, &nLen, NULL, 0);   // 客户端连接Socket
        if (RemoteSocket == SOCKET_ERROR)
        {
            g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d]: WSAAccept() Failed.", WSAGetLastError());
            continue;
        }       

        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "=====客户端[%s:%d]TCP连接, Socket[%d].",
            inet_ntoa(RemoteAddr.sin_addr), ntohs(RemoteAddr.sin_port), RemoteSocket);

        m_pMsgCallback(RemoteSocket, RemoteAddr, "", 1);

        LPSOCKETRESOURCE pSocketResource = new SOCKETRESOURCE;
        if(pSocketResource == NULL)
        {
            g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d]: new SOCKETRESOURCE Failed, 动态分配内存空间失败.", GetLastError());
        }
        pSocketResource->RemoteSocket = RemoteSocket;
        memcpy(&pSocketResource->RemoteAddr, &RemoteAddr, sizeof(RemoteAddr));
        pSocketResource->pHandleData->RemoteSocket = RemoteSocket;
        if (CreateIoCompletionPort((HANDLE) RemoteSocket, m_CompletionPort, (DWORD)pSocketResource->pHandleData, 0) == NULL)
        {
            g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d]: CreateIoCompletionPort Failed.", GetLastError());
            continue;
        }             
        m_mapSocketResource.insert(make_pair(RemoteSocket, pSocketResource));

        if(!RecvData(RemoteSocket))
        {
            continue;
        }
    }

    return 0;
}
bool CTCPIOCPSocket::ConnectServer(string sServerIP, int nServerPort)
{
    m_sServerIP = sServerIP;
    m_nServerPort = nServerPort;
    CreateThread(NULL, 0, ConnectServerThread, this, 0, NULL);     //开启连接服务线程
    return true;

    
}
DWORD WINAPI CTCPIOCPSocket::ConnectServerThread(LPVOID lParam)
{
    CTCPIOCPSocket * pThis = (CTCPIOCPSocket*)lParam;
    pThis->ConnectServerAction();
    return 0;
}
void CTCPIOCPSocket::ConnectServerAction()
{
    //如m_LocalSocket已经初始化过, 说明此时为连接断开后重连服务
    if(INVALID_SOCKET != m_LocalSocket)     
    {
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "--重连服务[%s:%d]中...", m_sServerIP.c_str(), m_nServerPort);
        map<SOCKET, LPSOCKETRESOURCE>::iterator it = m_mapSocketResource.find(m_LocalSocket);
        if(it != m_mapSocketResource.end())
        {
            it->second->bDestroy = true;
            time(&it->second->tDestroyTime);
        }
        m_LocalSocket = INVALID_SOCKET;
        Sleep(1000);
    }
    else
    {
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "--连接服务[%s:%d]中...", m_sServerIP.c_str(), m_nServerPort);
    }

    //初始化m_LocalSocket
    if ((m_LocalSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d]: WSASocket() Failed.", WSAGetLastError());
        m_LocalSocket = INVALID_SOCKET;
        return;
    } 

    //传入端口不为0时, 如传入IP为空, 则先取得本机IP, 再绑定地址; 否则由系统自动分配.
    if(m_nLocalPort != 0)
    {
        if(m_sLocalIP == "")
        {
            PHOSTENT hostinfo; 
            char name[155];
            string sHostIP;
            if( gethostname ( name, sizeof(name)) == 0) 
            { 
                if((hostinfo = gethostbyname(name)) != NULL) 
                { 
                    sHostIP = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);
                }
            }
            m_sLocalIP = sHostIP;
        }

        m_LocalAddr.sin_family = AF_INET;
        m_LocalAddr.sin_addr.s_addr = inet_addr(m_sLocalIP.c_str());
        m_LocalAddr.sin_port = htons(m_nLocalPort);

        if (bind(m_LocalSocket, (PSOCKADDR) &m_LocalAddr, sizeof(m_LocalAddr)) == SOCKET_ERROR)
        {
            g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d]: 绑定Socket失败, Address[%s:%d].", 
                WSAGetLastError(), m_sLocalIP.c_str(), m_nLocalPort);
            return;
        }
    }

    m_RemoteAddr.sin_family = AF_INET;
    m_RemoteAddr.sin_addr.s_addr = inet_addr(m_sServerIP.c_str());
    m_RemoteAddr.sin_port = htons(m_nServerPort);

    int nRet = 0;
    do 
    {
        nRet = WSAConnect(m_LocalSocket, (SOCKADDR*)&m_RemoteAddr, sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL);
        if(nRet == SOCKET_ERROR)
        {
            int nError = WSAGetLastError();
            if(m_nConnectServerFailed % 1000 == 0)
            {
                g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d]: WSAConnect[%s:%d] Failed.",
                    nError, m_sServerIP.c_str(), m_nServerPort);
                m_pMsgCallback(m_LocalSocket, m_RemoteAddr, "", nError);
            }
            m_nConnectServerFailed = m_nConnectServerFailed > 1000 ? 0 :  ++m_nConnectServerFailed;
            Sleep(2 * 1000);
        }
    } while (!m_bStopSocket && nRet == SOCKET_ERROR);

    if(m_bStopSocket || nRet == SOCKET_ERROR)
    {
        return;
    }

    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "--SOCKET[%d]TCP连接服务端[%s:%d]成功.", 
        m_LocalSocket, m_sServerIP.c_str(), m_nServerPort);
    m_nConnectServerFailed = 0;

    LPSOCKETRESOURCE pSocketResource = new SOCKETRESOURCE;
    if(pSocketResource == NULL)
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d]: new SOCKETRESOURCE Failed, 动态分配内存空间失败.", GetLastError());
        return;
    }
    pSocketResource->RemoteSocket = m_LocalSocket;
    memcpy(&pSocketResource->RemoteAddr, &m_RemoteAddr, sizeof(m_RemoteAddr));
    pSocketResource->pHandleData->RemoteSocket = m_LocalSocket;
    m_mapSocketResource.insert(make_pair(m_LocalSocket, pSocketResource));

    if (CreateIoCompletionPort((HANDLE) m_LocalSocket, m_CompletionPort, (DWORD)pSocketResource->pHandleData, 0) == NULL)
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d]: CreateIoCompletionPort Failed.", GetLastError());
        return;
    }

    if(m_pMsgCallback != NULL)
    {
        m_pMsgCallback(m_LocalSocket, m_RemoteAddr, "", 1);
    }

    if(!RecvData(m_LocalSocket))
    {
        int nError = WSAGetLastError();
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d]: RecvData Failed.", WSAGetLastError());
        if(m_pMsgCallback != NULL)
        {
            m_pMsgCallback(m_LocalSocket, m_RemoteAddr, "", nError);
        }
    }
    return;
}
bool CTCPIOCPSocket::SendData(char * pSendData, int nLen, SOCKET RemoteSocket)
{      
    if(m_bStopSocket)
    {
        return true;
    }
    if(m_bIsServer)
    {
        if(RemoteSocket == INVALID_SOCKET)
        {
            return false;
        }
    }
    else
    {
        RemoteSocket = m_LocalSocket;
    }

    map<SOCKET, LPSOCKETRESOURCE>::iterator it = m_mapSocketResource.find(RemoteSocket);
    if(it == m_mapSocketResource.end() || it->second->bDestroy)  //没找到对应Socket资源, 或者己处于销毁状态
    {
        return false;
    }

    ZeroMemory(&it->second->pSendIocpData->Overlapped, sizeof(it->second->pSendIocpData->Overlapped));
    it->second->pSendIocpData->DataBuf.buf = pSendData;
    it->second->pSendIocpData->DataBuf.len = nLen;

    int nRet = WSASend(RemoteSocket, &(it->second->pSendIocpData->DataBuf), 1, &m_nByteOfSend, 0,
        &(it->second->pSendIocpData->Overlapped), NULL);
    if(nRet < 0)
    {
        int nError = WSAGetLastError();
        if(nError != WSA_IO_PENDING)
        {
            m_pMsgCallback(RemoteSocket, it->second->RemoteAddr, "", nError);
            g_LogRecorder.WriteInfoLogEx(__FUNCTION__, "****Error[%d]: SendData Failed, Remote[%s:%d].",
                nError, inet_ntoa(it->second->RemoteAddr.sin_addr), ntohs(it->second->RemoteAddr.sin_port));      

            return false;
        }       
    }
    return true;
}
bool CTCPIOCPSocket::RecvData(SOCKET RemoteSocket)
{
    if(m_bStopSocket)
    {
        return true;
    }
    if(m_bIsServer)
    {
        if(RemoteSocket == INVALID_SOCKET)
        {
            return false;
        }
    }
    else
    {
        RemoteSocket = m_LocalSocket;
    }

    map<SOCKET, LPSOCKETRESOURCE>::iterator it = m_mapSocketResource.find(RemoteSocket);
    if(it == m_mapSocketResource.end() || it->second->bDestroy)
    {
        return false;
    }

    ZeroMemory(&it->second->pRecvIocpData->Overlapped, sizeof(it->second->pRecvIocpData->Overlapped));
    ZeroMemory(it->second->pRecvIocpData->DataBuf.buf, it->second->pRecvIocpData->DataBuf.len);
    DWORD Flags = 0;
    int nRet = WSARecv(RemoteSocket, &(it->second->pRecvIocpData->DataBuf), 1, &m_nByteOfSend, &Flags,
        &(it->second->pRecvIocpData->Overlapped), NULL);
    if(nRet < 0)
    {
        int nError = WSAGetLastError();
        if(nError != WSA_IO_PENDING)
        {
            m_pMsgCallback(RemoteSocket, it->second->RemoteAddr, "", nError);
            g_LogRecorder.WriteInfoLogEx(__FUNCTION__, "****Error[%d]: RecvData Failed, Remote[%s:%d].",
                nError, inet_ntoa(it->second->RemoteAddr.sin_addr), ntohs(it->second->RemoteAddr.sin_port));           
            return false;
        }       
    }
    return true;
}
void CTCPIOCPSocket::StopSocket()
{
    m_bStopSocket = true;   
    SetEvent(m_hStopEvent);
    Sleep(500);
    for(DWORD i = 0; i<SystemInfo.dwNumberOfProcessors; i++)
    {
        if(!PostQueuedCompletionStatus(m_CompletionPort, 0, 0, NULL))
        {
            g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d]: PostQueuedCompletionStatus退出工作线程失败.", WSAGetLastError());
        }
        else
        {
            g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "^^PostQueuedCompletionStatus退出工作线程成功.");
        }
    }
    CloseHandle(m_CompletionPort);
    map<SOCKET, LPSOCKETRESOURCE>::iterator it = m_mapSocketResource.begin();
    while(it != m_mapSocketResource.end())
    {
        delete it->second;
        it = m_mapSocketResource.erase(it);
    }
    closesocket(m_LocalSocket);
    WSACleanup();
    return;
}
DWORD WINAPI CTCPIOCPSocket :: ServerWorkerThread(LPVOID lParam)
{
    CTCPIOCPSocket * pThis = (CTCPIOCPSocket*)lParam;
    pThis->ServerWorkerAction();
    return 0;
}
void CTCPIOCPSocket :: ServerWorkerAction()
{
    DWORD dwBytesTransferred = 0;
    LPHANDLEDATA pHandleData = NULL;
    LPIOCPDATA  pIOCPData = NULL;

    while(!m_bStopSocket)
    {
         bool bStatus = GetQueuedCompletionStatus(m_CompletionPort, &dwBytesTransferred,
            (PULONG_PTR)&pHandleData, (LPOVERLAPPED *) &pIOCPData, INFINITE);

        if (!bStatus || dwBytesTransferred == 0)    //关闭 Socket 
        {
            if(pHandleData == NULL)
            {
                break;
            }

            EnterCriticalSection(&m_cs);
            map<SOCKET, LPSOCKETRESOURCE>::iterator it = m_mapSocketResource.find(pHandleData->RemoteSocket);
            if(it != m_mapSocketResource.end() && !it->second->bDestroy)
            {
                LPSOCKETRESOURCE pSocketResource = it->second;
                g_LogRecorder.WriteInfoLogEx(__FUNCTION__, "--TCP连接关闭, Remote Address[%s:%d], Socket[%d].",
                    inet_ntoa(it->second->RemoteAddr.sin_addr), ntohs(it->second->RemoteAddr.sin_port), 
                    it->second->pHandleData->RemoteSocket);
                m_pMsgCallback(pSocketResource->pHandleData->RemoteSocket, pSocketResource->RemoteAddr, "", 10000);
                if(m_bIsServer)
                {
                    it->second->bDestroy = true;
                    time(&it->second->tDestroyTime);
                }
                else
                {
                    m_pMsgCallback(pSocketResource->pHandleData->RemoteSocket, pSocketResource->RemoteAddr, "", 2);
                    ConnectServer(m_sServerIP, m_nServerPort);           
                }
            } 
            LeaveCriticalSection(&m_cs);
            continue;
        }                                    
        else
        {
            if(pIOCPData->nOperatorType == 1)
            {
                continue;
            }
            else if(pIOCPData->nOperatorType == 2)
            {
                string sRecvBuf(pIOCPData->DataBuf.buf, dwBytesTransferred);
                map<SOCKET, LPSOCKETRESOURCE>::iterator it = m_mapSocketResource.find(pHandleData->RemoteSocket);
                if(it != m_mapSocketResource.end())
                {
                    m_pMsgCallback(pHandleData->RemoteSocket, it->second->RemoteAddr, sRecvBuf, 0);
                }    

                bool bRet = false;
                do 
                {
                    bRet = RecvData(pHandleData->RemoteSocket);
                    if(!bRet && !m_bIsServer)
                    {
                        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "***Warning: 接收数据错误, 尝试重新接收...");
                        Sleep(3 * 1000);
                    }
                } while(!bRet && !m_bIsServer && !m_bStopSocket);  //客户端时, 如接收错误, 则反复接收到成功; 服务器时, 接收错误则直接返回.
            }
        }
    }

    return;
}

DWORD WINAPI CTCPIOCPSocket::DestroyResourceThread(LPVOID lParam)
{
    CTCPIOCPSocket * pThis = (CTCPIOCPSocket*)lParam;
    pThis->DestroyResourceAction();
    return 0;
}
void CTCPIOCPSocket::DestroyResourceAction()
{
    time_t tCurrent = 0;
    while(!m_bStopSocket)
    {
        time(&tCurrent);
        map<SOCKET, LPSOCKETRESOURCE>::iterator it = m_mapSocketResource.begin();
        while(it != m_mapSocketResource.end())
        {
            if(it->second->bDestroy && tCurrent - it->second->tDestroyTime > 10)
            {
                g_LogRecorder.WriteInfoLogEx(__FUNCTION__, "--DestroyResource, Socket[%d].", it->second->RemoteSocket);
                delete it->second;
                it = m_mapSocketResource.erase(it);
            }
            else
            {
                it ++;
            }
        }

        if(WAIT_OBJECT_0 == WaitForSingleObject(m_hStopEvent, 10 * 1000))
        {
            break;
        }
    }    
    return;
}
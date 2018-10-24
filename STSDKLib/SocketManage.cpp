#include "stdafx.h"
#include "SocketManage.h"


extern CLogRecorder g_LogRecorder;
map<string, LPSTSERVERCOUNT> CSocketManage::m_mapDBCountInfo;  
int CSocketManage::m_nNum = 0;
CSocketManage::CSocketManage()
{
    InitializeCriticalSection(&m_cs);
    m_hStopEvent = CreateEvent(NULL, true, false, NULL);
    m_pRecvMsg = new char[HTTPRECVMSGSIZE];
    m_nRecvLen = 0;
    m_nNum ++;
}

CSocketManage::~CSocketManage()
{
    DeleteCriticalSection(&m_cs);
    CloseHandle(m_hStopEvent);
}
bool CSocketManage::Init(STSERVERINFO STServerInfo[], int nQuantity)
{
    WSADATA     wsaData;
    if ((WSAStartup(0x0202, &wsaData)) != 0)
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error[%d].: WSAStartup Failed", GetLastError());
        return false;
    }
    for (int i = 0; i < nQuantity; i++)
    {
        CHTTPSocket * pHttpSocket = new CHTTPSocket;
        LPSOCKETINFO pSocketInfo = new SOCKETINFO;
        strcpy_s(pSocketInfo->pSTServerIP, sizeof(pSocketInfo->pSTServerIP), STServerInfo[i].pSTServerIP);
        pSocketInfo->nSTServerPort = STServerInfo[i].nSTServerPort;
        pSocketInfo->nType = STServerInfo[i].nType;
        if (!pHttpSocket->InitConnect(STServerInfo[i].pSTServerIP, STServerInfo[i].nSTServerPort))
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: ���ӷ���[%s:%d]ʧ��.", STServerInfo[i].pSTServerIP, STServerInfo[i].nSTServerPort);
            pSocketInfo->bStatus = false;
        }
        else
        {
            if(m_nNum == 1)
            {
                g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "���ӷ���[%s:%d]�ɹ�.", STServerInfo[i].pSTServerIP, STServerInfo[i].nSTServerPort);
            }
            pSocketInfo->bStatus = true;
        }
        
        m_mapHTTPSocket.insert(make_pair(pHttpSocket, pSocketInfo));
        CreateThread(NULL, 0, PostSTMsgThread, this, NULL, 0);
    }
    return true;
}
bool CSocketManage::Uninit()
{
    SetEvent(m_hStopEvent);
    map<CHTTPSocket*, LPSOCKETINFO>::iterator it = m_mapHTTPSocket.begin();
    while (it != m_mapHTTPSocket.end())
    {
        delete it->first;
        delete it->second;
        it = m_mapHTTPSocket.erase(it);
    }
    return true;
}
bool CSocketManage::SendMsg(char * pHttpMsg, int nHttpMsgLen, int nSTType, OPERATORFUN nFunType, int nSynNum)
{
    if(nFunType == SYNADDIMAGE || nFunType == SYNADDFEATURE || nFunType == SYNADDMULTIPLEIMAGE)    //ͼƬ������ֵ���, ���ؾ���ѡ��ǰ�����������С��ST������
    {
        string sSTServerIP = GetAddImageSTServer(nSynNum);
        if(sSTServerIP == "")
        {
            return false;
        }

        map<CHTTPSocket*, LPSOCKETINFO>::iterator it = m_mapHTTPSocket.begin();
        for(; it != m_mapHTTPSocket.end(); it ++)
        {
            if(sSTServerIP == string(it->second->pSTServerIP))
            {
                ResetEvent(it->second->m_hSTEvent);
                it->second->pHTTPMsg = pHttpMsg;
                it->second->nHTTPMsgLen = nHttpMsgLen;
                it->second->bDeal = false;
                it->second->nFunType = nFunType;

                break;
            }
        }
    }
    else
    {
        map<CHTTPSocket*, LPSOCKETINFO>::iterator it = m_mapHTTPSocket.begin();
        for(; it != m_mapHTTPSocket.end(); it ++)
        {
            if(nSTType == 3 && it->second->bStatus)   //��������, ѡ��һ������ST��������������
            {
                ResetEvent(it->second->m_hSTEvent);
                it->second->pHTTPMsg = pHttpMsg;
                it->second->nHTTPMsgLen = nHttpMsgLen;
                it->second->bDeal = false;
                it->second->nFunType = nFunType;

                break;
            }
            else if(nSTType == it->second->nType)
            {
                ResetEvent(it->second->m_hSTEvent);
                it->second->pHTTPMsg = pHttpMsg;
                it->second->nHTTPMsgLen = nHttpMsgLen;
                it->second->bDeal = false;
                it->second->nFunType = nFunType;
            }
        }

        if(nSTType == 3 && it == m_mapHTTPSocket.end())   //��������, û���ҵ����ߵ�ST������
        {
            for(it = m_mapHTTPSocket.begin(); it != m_mapHTTPSocket.end(); it ++)
            {
                //ѡ���һ��ST���������Բ���
                ResetEvent(it->second->m_hSTEvent);
                it->second->pHTTPMsg = pHttpMsg;
                it->second->nHTTPMsgLen = nHttpMsgLen;
                it->second->bDeal = false;
                it->second->nFunType = nFunType;

                break;
            }
        }
    }

    return true;
}
string CSocketManage::GetAddImageSTServer(int nSynNum)
{
    string sIP = "";
    map<string, LPSTSERVERCOUNT>::iterator it = m_mapDBCountInfo.begin();
    map<string, LPSTSERVERCOUNT>::iterator itTemp = it;
    for(; it != m_mapDBCountInfo.end(); it ++)
    {
        if(it->second->nCurCount < itTemp->second->nCurCount)
        {
            itTemp = it;
        }
    }

    if(itTemp != m_mapDBCountInfo.end())
    {
        itTemp->second->nCurCount += nSynNum;
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "ѡ��ST������[%s].", itTemp->first.c_str());
        sIP = itTemp->first;
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: δ�ҵ�����ST������!");
    }
    return sIP;
}
int CSocketManage::RecvMsg(char * pRecvMsg, unsigned int * nSize)
{
    int nRet = 0;
    m_nRecvLen = 0;
    map<CHTTPSocket*, LPSOCKETINFO>::iterator it = m_mapHTTPSocket.begin();
    for(; it != m_mapHTTPSocket.end(); it ++)
    {
        if(!it->second->bDeal)
        {
            if(WAIT_TIMEOUT == WaitForSingleObject(it->second->m_hSTEvent, 1000 * RESOVERTIME))
            {
                g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "***Warning: �̷߳���ST������[%s:%d]��Ӧ��Ϣ��ʱ.", 
                    it->second->pSTServerIP, it->second->nSTServerPort);
            }
            else
            {
                if(it->second->nRecvLen > 0 && it->second->nError == 0)
                {
                    DealwithRepMsg(it->second->pRecvMsg, it->second->nRecvLen, it->second->pSTServerIP, it->second->nFunType);
                }
            }

            it->second->Init();
        }
    }

    if(m_nRecvLen < 0)
    {
        ParseErrorJson(m_nRecvLen);
        *nSize = 0;
        nRet = STSDK_STJSONFORMATERROR;
    }
    else if(m_nRecvLen == 0)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: ����|����ST������Ϣʧ��.");
        *nSize = 0;
        nRet = STSDK_DISCONNECTSERVER;
    }
    else if(*nSize < m_nRecvLen)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, 
            "***Warning: ����STSDKLib�����ST��Ӧ��Ϣbuf����[%d]С��ST��Ӧ��Ϣ����[%d].", *nSize, m_nRecvLen);
        nRet = STSDK_RESPONSEMSGLENFAILED;
        *nSize = 0;
    }
    else
    {
        strcpy_s(pRecvMsg, *nSize, m_pRecvMsg);
        *nSize = m_nRecvLen;
    }

    return nRet;
}
//�ϲ�Json������
int CSocketManage::DealwithRepMsg(char * pMsg, int nLen, char * pSTServerIP, int nFunType)
{
    string sBody = "";
    if(m_nRecvLen <= 0)
    {
        strcpy_s(m_pRecvMsg, HTTPRECVMSGSIZE, pMsg);
        m_nRecvLen = nLen;
        
        //ͼƬ���, ����������ST������IP
        if(nFunType == SYNADDIMAGE || nFunType == SYNADDFEATURE)    
        {
            rapidjson::Document document;
            document.Parse(m_pRecvMsg);
            if(document.HasParseError())
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "����Json��ʧ��[%s]", m_pRecvMsg);
                m_nRecvLen = -1;
                return -1;
            }
            rapidjson::Document::AllocatorType&allocator = document.GetAllocator();
            document.AddMember("STServerIP", rapidjson::StringRef(pSTServerIP), allocator);

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.Accept(writer);
            sBody = buffer.GetString();
            strcpy_s(m_pRecvMsg, HTTPRECVMSGSIZE, sBody.c_str());
            m_nRecvLen = sBody.size();
        }
        //���浱ǰST���������������Ϣ����������
        else if(nFunType == GETALLDBINFO || nFunType == GETDETAIL) 
        {
            rapidjson::Document document;
            document.Parse(m_pRecvMsg);
            if(document.HasParseError())
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "����Json��ʧ��[%s]", m_pRecvMsg);
                m_nRecvLen = -1;
                return -1;
            }
            if(!document.HasMember("result") || !document["result"].IsString())
            {
                m_nRecvLen = -2;
                return -1;
            }
            string sResult = document["result"].GetString();
            if("success" == sResult)
            {
                int nCount = 0;
                if(nFunType == GETALLDBINFO)
                {
                    if(document.HasMember("data") && document["data"].IsArray() && document["data"].Size() > 0)
                    {
                        for(int i = 0; i < document["data"].Size(); i ++)
                        {
                            if(document["data"][i].HasMember("count") && document["data"][i]["count"].IsInt())
                            {
                                nCount += document["data"][i]["count"].GetInt();
                            }
                        }
                    }
                }
                else
                {
                    if(document.HasMember("data") && document["data"].HasMember("totalMaxImageCount") && document["data"]["totalMaxImageCount"].IsInt())
                    {
                        nCount = document["data"]["totalMaxImageCount"].GetInt();
                    }
                    else
                    {
                        m_nRecvLen = -3;
                        return -1;
                    }
                }

                map<string, LPSTSERVERCOUNT>::iterator it = m_mapDBCountInfo.find(pSTServerIP);
                if(it != m_mapDBCountInfo.end())
                {
                    if(nFunType == GETALLDBINFO)
                    {
                        it->second->nCurCount = nCount;
                        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "ST������[%s]��ǰ���������: %d.", it->first.c_str(), nCount);
                    }
                    else
                    {
                        it->second->nMaxCount = nCount;
                        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "ST������[%s]��������: %d.", it->first.c_str(), nCount);
                    }
                }
                else
                {
                    LPSTSERVERCOUNT pSTServerCount = new STSERVERCOUNT;
                    m_mapDBCountInfo.insert(make_pair(pSTServerIP, pSTServerCount));

                    if(nFunType == GETALLDBINFO)
                    {
                        pSTServerCount->nCurCount = nCount;
                        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "ST������[%s]��ǰ���������: %d.", pSTServerIP, nCount);
                    }
                    else
                    {
                        pSTServerCount->nMaxCount = nCount;
                        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "ST������[%s]��������: %d.", pSTServerIP, nCount);
                    }
                }
            }
            else
            {
                string sTemp = nFunType == GETALLDBINFO ? "���������" : "��������";
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: ��ȡ����%sʧ��, ����[%s]", sTemp.c_str(), m_pRecvMsg);
            }
        }
    }
    else
    {
        switch(nFunType)
        {
        case ADDDB: case DELDB: case CLEARDB:   //����, ɾ��, ��տ�
            {
                rapidjson::Document document;
                document.Parse(pMsg);
                if(document.HasParseError())
                {
                    g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "����Json��ʧ��[%s]", pMsg);
                    return -1;
                }
                if(document.HasMember("result") && document["result"].IsString())
                {
                    string sResult = document["result"].GetString();
                    if(sResult == "success")
                    {
                        sBody = pMsg;
                    }
                }
                break;
            }
        case GETALLDBINFO:      //��ȡ���п���Ϣ
            {
                map<string, int> mapDBInfo;
                rapidjson::Document document;
                document.Parse(m_pRecvMsg);
                if(document.HasParseError())
                {
                    g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "����Json��ʧ��[%s]", m_pRecvMsg);
                    return -1;
                }
                if(!document.HasMember("result") || !document["result"].IsString())
                {
                    return -1;
                }
                string sResult = document["result"].GetString();
                if("success" == sResult)
                {
                    if(document.HasMember("data") && document["data"].IsArray() && document["data"].Size() > 0)
                    {
                        for(int i = 0; i < document["data"].Size(); i ++)
                        {
                            if(!document["data"][i].HasMember("dbName") || !document["data"][i].HasMember("count"))
                            {
                                continue;
                            }
                            string sDBName = document["data"][i]["dbName"].GetString();
                            int nCount = document["data"][i]["count"].GetInt();
                            mapDBInfo.insert(make_pair(sDBName, nCount));
                        }
                    }
                }

                document.Parse(pMsg);
                if(document.HasParseError())
                {
                    g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "����Json��ʧ��[%s]", pMsg);
                    return -1;
                }
                if(!document.HasMember("result") || !document["result"].IsString())
                {
                    return -1;
                }
                sResult = document["result"].GetString();
                if(sResult == "success")
                {
                    int nSTServerCount = 0;
                    if(document.HasMember("data") && document["data"].IsArray() && document["data"].Size() > 0)
                    {
                        for(int i = 0; i < document["data"].Size(); i ++)
                        {
                            if(!document["data"][i].HasMember("dbName") || !document["data"][i].HasMember("count"))
                            {
                                continue;
                            }
                            string sDBName = document["data"][i]["dbName"].GetString();
                            int nCount = document["data"][i]["count"].GetInt();

                            nSTServerCount += nCount;

                            map<string, int>::iterator it= mapDBInfo.find(sDBName);
                            if(it != mapDBInfo.end())
                            {
                                it->second += nCount;
                            }
                            else
                            {
                                mapDBInfo.insert(make_pair(sDBName, nCount));
                            }
                        }
                    }
                    
                    map<string, LPSTSERVERCOUNT>::iterator it = m_mapDBCountInfo.find(pSTServerIP);
                    if(it != m_mapDBCountInfo.end())
                    {
                        it->second->nCurCount = nSTServerCount;
                        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "ST������[%s]��ǰ���������: %d.", pSTServerIP, nSTServerCount);
                    }
                    else
                    {
                        LPSTSERVERCOUNT pSTServerCount = new STSERVERCOUNT;
                        pSTServerCount->nCurCount = nSTServerCount;
                        m_mapDBCountInfo.insert(make_pair(pSTServerIP, pSTServerCount));
                        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "ST������[%s]��ǰ���������: %d.", pSTServerIP, nSTServerCount);
                    }
                }

                document.SetObject();
                rapidjson::Document::AllocatorType&allocator = document.GetAllocator();
                document.AddMember("result", "success", allocator);
                document.AddMember("totalImageCount", 0, allocator);
                rapidjson::Value array(rapidjson::kArrayType);
                map<string, int>::iterator it= mapDBInfo.begin();
                int i = 0;
                for(; it != mapDBInfo.end(); it++)
                {
                    rapidjson::Value object(rapidjson::kObjectType);
                    object.AddMember("dbName", rapidjson::StringRef(it->first.c_str()), allocator);
                    object.AddMember("maxCount", 0, allocator);
                    object.AddMember("count", it->second, allocator);

                    array.PushBack(object, allocator);
                }
                document.AddMember("data", array, allocator);

                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                document.Accept(writer);
                sBody = buffer.GetString();

                break;
            }
        case GETIMAGEBYID:      //����ID��ȡͼƬ
            {
                

                break;
            }
        case DELIMAGEBYID:      //����IDɾ��ͼƬ
            {
                rapidjson::Document document;
                document.Parse(pMsg);
                if(document.HasParseError())
                {
                    g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "����Json��ʧ��[%s]", pMsg);
                    return -1;
                }
                if(document.HasMember("result") && document["result"].IsString())
                {
                    string sResult = document["result"].GetString();
                    if(sResult == "success")
                    {
                        sBody = pMsg;
                    }
                }
                break;
            }
        case SEARCHIMAGE:  case SEARCHIMAGEBYFEATURE:      //��������ͼƬ������ֵ����
            {
                rapidjson::Document document;
                rapidjson::Document::AllocatorType&allocator = document.GetAllocator();

                document.Parse(m_pRecvMsg);
                if(document.HasParseError())
                {
                    g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "����Json��ʧ��[%s]", m_pRecvMsg);
                    return -1;
                }
                if(!document.HasMember("result") || !document["result"].IsString())
                {
                    return -1;
                }
                string sResult = document["result"].GetString();
                if(sResult == "success")
                {
                    if(document.HasMember("data") && document["data"].IsArray())
                    {
                        if(document["data"].Size() == 0)    //���֮ǰ������û��������ƥ����, ֱ������, ��ֵΪ��ǰ�������Ϣ
                        {
                            sBody = pMsg;
                        }
                        else
                        {
                            rapidjson::Document document2;
                            document2.Parse(pMsg);
                            if(document2.HasParseError())
                            {
                                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "����Json��ʧ��[%s]", pMsg);
                                return -1;
                            }
                            if(!document2.HasMember("result") || !document2["result"].IsString())
                            {
                                return -1;
                            }
                            string sResult2 = document2["result"].GetString();
                            if(sResult2 == "success")
                            {
                                if(document2.HasMember("data") && document2["data"].IsArray())
                                {
                                    if(document2["data"].Size() > 0)    //��ǰ�������Ϣ��ƥ����, �����, û���򲻴���
                                    {
                                        document.AddMember("data", document2["data"], allocator);

                                        rapidjson::StringBuffer buffer;
                                        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                                        document.Accept(writer);
                                        sBody = buffer.GetString();
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
        case GETDETAIL:             //��ȡ���������
            {
                int nMax = 0;
                rapidjson::Document document;

                document.Parse(m_pRecvMsg);
                if(document.HasParseError())
                {
                    g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "����Json��ʧ��[%s]", m_pRecvMsg);
                    return -1;
                }
                if(!document.HasMember("result") || !document["result"].IsString())
                {
                    return -1;
                }
                string sResult = document["result"].GetString();
                if(sResult == "success")
                {
                    if(document["data"].HasMember("totalMaxImageCount") && document["data"]["totalMaxImageCount"].IsInt())
                    {
                        nMax = document["data"]["totalMaxImageCount"].GetInt();
                    }
                }

                document.Parse(pMsg);
                if(document.HasParseError())
                {
                    g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "����Json��ʧ��[%s]", pMsg);
                    return -1;
                }
                if(!document.HasMember("result") || !document["result"].IsString())
                {
                    return -1;
                }
                sResult = document["result"].GetString();
                if(sResult == "success")
                {
                    if(document["data"].HasMember("totalMaxImageCount") && document["data"]["totalMaxImageCount"].IsInt())
                    {
                        int nCurMax = document["data"]["totalMaxImageCount"].GetInt();
                        nMax += nCurMax;

                        //���浱ǰST��������������
                        map<string, LPSTSERVERCOUNT>::iterator it = m_mapDBCountInfo.find(pSTServerIP);
                        if(it != m_mapDBCountInfo.end())
                        {
                            it->second->nMaxCount = nCurMax;
                            g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "ST������[%s]��������: %d.", it->first.c_str(), nCurMax);
                        }
                        else
                        {
                            LPSTSERVERCOUNT pSTServerCount = new STSERVERCOUNT;
                            pSTServerCount->nMaxCount = nCurMax;
                            m_mapDBCountInfo.insert(make_pair(pSTServerIP, pSTServerCount));
                            g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "ST������[%s]��������: %d.", pSTServerIP, nCurMax);
                        }

                        document["data"]["totalMaxImageCount"]= nMax;
                        rapidjson::StringBuffer buffer;
                        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                        document.Accept(writer);
                        sBody = buffer.GetString();
                    }
                }
                break;
            }
        default:
            break;
        }

        if(sBody != "")
        {
            strcpy_s(m_pRecvMsg, HTTPRECVMSGSIZE, sBody.c_str());
            m_nRecvLen = sBody.size();
        }
        
    }
    
    return 0;
}

DWORD WINAPI CSocketManage::PostSTMsgThread(LPVOID lParam)
{
    CSocketManage * pThis = (CSocketManage*)lParam;
    pThis->PostSTMsgAction();
    return 0;
}

void CSocketManage::PostSTMsgAction()
{
    CHTTPSocket * pHttpSocket = NULL;
    LPSOCKETINFO pSocketInfo = NULL;
    int nNum = 0;
    do 
    {
        EnterCriticalSection(&m_cs);
        map<CHTTPSocket*, LPSOCKETINFO>::iterator it = m_mapHTTPSocket.begin();
        for (; it != m_mapHTTPSocket.end(); it ++)
        {
            if(!it->second->bUsed)
            {
                it->second->bUsed = true;
                pHttpSocket = it->first;
                pSocketInfo = it->second;
                break;
            }
        }
        LeaveCriticalSection(&m_cs);
        if(NULL == pHttpSocket)
        {
            Sleep(200);
            nNum ++;
        }
        else
        {
            break;
        }
    }while(nNum < 10);

    if(NULL == pHttpSocket)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: �߳�δ��������ST��Դ");
        return;
    }

    while(WAIT_TIMEOUT == WaitForSingleObject(m_hStopEvent, 10))
    {
        if(pSocketInfo->nHTTPMsgLen != 0)  //����Ϣ����
        {
            pSocketInfo->nRecvLen = 0;
            if(!pHttpSocket->SendMsg(pSocketInfo->pHTTPMsg, pSocketInfo->nHTTPMsgLen))
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: ��ST������������Ϣʧ��.");
                pSocketInfo->bStatus = false;
                pSocketInfo->nError = STSDK_DISCONNECTSERVER;
            }
            else
            {
                pSocketInfo->bStatus = true;
                pSocketInfo->nRecvLen = HTTPRECVMSGSIZE;
                int nRet = pHttpSocket->RecvMsg(pSocketInfo->pRecvMsg, &pSocketInfo->nRecvLen);
                if(nRet != 0)
                {
                    g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: ����ST��������Ϣʧ��.");
                    pSocketInfo->nRecvLen = 0;
                    pSocketInfo->nError = nRet;
                }
            }
            pSocketInfo->nHTTPMsgLen = 0;
            SetEvent(pSocketInfo->m_hSTEvent);
        }
    }


    return;
}
void CSocketManage::ParseErrorJson(int nError)
{
    switch(nError)
    {
    case -1:
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: ��������Json����ʽ����.");
            break;
        }
    case -2:
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: ��������Json����Result�ֶ�ֵ.");
            break;
        }
    default:
        break;
    }
    return;
}

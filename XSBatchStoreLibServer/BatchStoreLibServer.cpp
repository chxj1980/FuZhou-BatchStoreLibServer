#include "StdAfx.h"
#include "BatchStoreLibServer.h"

CLogRecorder g_LogRecorder;
CBatchStoreLibServer::CBatchStoreLibServer(void)
{
    InitializeCriticalSection(&m_cs);
    InitializeCriticalSection(&m_Httpcs);
    m_hStopEvent = CreateEvent(NULL, true, false, NULL);
    m_pGetFeature = NULL;
    m_pAnalyseStore = NULL;
    m_pHttpServer = NULL;
    m_pConfigRead = new CConfigRead;

    ZeroMemory(m_pServerIP, sizeof(m_pServerIP));
    m_nServerPort = 0;

    ZeroMemory(m_pProxyServerIP, sizeof(m_pProxyServerIP));
    m_nProxyPort = 0;
}

CBatchStoreLibServer::~CBatchStoreLibServer(void)
{
    Sleep(100);
    CloseHandle(m_hStopEvent);
    if (NULL != m_pGetFeature)
    {
        delete m_pGetFeature;
        m_pGetFeature = NULL;
    }
    if (NULL != m_pHttpServer)
    {
        delete m_pHttpServer;
        m_pHttpServer = NULL;
    }
    if (NULL != m_pAnalyseStore)
    {
        delete m_pAnalyseStore;
        m_pAnalyseStore = NULL;
    }
    DeleteCriticalSection(&m_cs);
    DeleteCriticalSection(&m_Httpcs);
}
bool CBatchStoreLibServer::StartBatchStoreLibServer()
{
    if (!Init())
    {
        g_LogRecorder.WriteErrorLog(__FUNCTION__, "****Error: 服务初始化失败.");
        return false;
    }

    Sleep(1000 * 1);
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "\r\n**************************批量入库服务启动成功**************************");

    CreateThread(NULL, 0, HTTPRequestParseThread, this, NULL, 0);   //http请求处理线程

#ifdef _DEBUG
    while (true)
    {
        char pIn;
        cin >> pIn;
        switch (pIn)
        {
        case 'a':
        {
            printf("\n重点库信息:\n");
            MAPSTORELIBINFO::iterator itLib = m_mapStoreLibInfo.begin();
            for (; itLib != m_mapStoreLibInfo.end(); itLib++)
            {
                printf("\t重点库[%d]: 己入库图片数量[%d].\n", itLib->first, itLib->second->setStoreFace.size());
            }
            break;
        }
        case 'e':
            StopBatchStoreLibServer();
            Sleep(500);
            return true;
        default:
            break;
        }
    }
#else
    WaitForSingleObject(m_hStopEvent, INFINITE);
#endif
    return true;
}
bool CBatchStoreLibServer::StopBatchStoreLibServer()
{
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "**********结束服务**********");
    m_pGetFeature->UnInit();
    m_pHttpServer->StopHttpServer();
    m_pAnalyseStore->UnInit();
    while (m_listStoreImageInfo.size() > 0)
    {
        delete m_listStoreImageInfo.front();
        m_listStoreImageInfo.pop_front();
    }
    SetEvent(m_hStopEvent);
    return true;
}
bool CBatchStoreLibServer::Init()
{
    //初始化日志
    string sPath = m_pConfigRead->GetCurrentPath();
    string sConfigPath = sPath + "/Config/XSBatchStoreLibServer_config.properties";
#ifdef _DEBUG
    sConfigPath = "./Config/XSBatchStoreLibServer_config.properties";
#endif
    g_LogRecorder.InitLogger(sConfigPath.c_str(), "XSBatchStoreLibServerLogger", "XSBatchStoreLibServer");

    //读取配置文件
    if (!m_pConfigRead->ReadConfig())
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error: 读取配置文件参数错误!");
        return false;
    }
    else
    {
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "读取配置文件成功.");
    }

    //创建重点库图片保存路径文件夹
    strcpy(m_pSavePath, m_pConfigRead->m_sSavePath.c_str());
    CreateDirectory(m_pSavePath, NULL);
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "图片保存路径: %s", m_pSavePath);
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "图片过滤质量分数: %d", m_pConfigRead->m_nQuality);

    for(int i = 0; i < THREADNUM; i++)
    {
        LPSTOREFACEINFO pStoreImageInfo = new STOREFACEINFO;
        m_listStoreImageInfo.push_back(pStoreImageInfo);
    }

    //初始化连接DB并获取服务相关信息
    if (!InitDB())
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error: Connect DB Failed!");
        return false;
    }
    //数据库查询重点库人脸信息
    if (!GetStoreLibInfo())
    {
        g_LogRecorder.WriteDebugLog(__FUNCTION__, "****Error: 获取重点库信息失败.");
    }
    //初始化特征值服务连接
    if (!InitGetFeature())
    {
        g_LogRecorder.WriteDebugLog(__FUNCTION__, "****Error: 获取特征值服务连接失败!");
        return false;
    }
    if (!InitAnalyse())
    {
        g_LogRecorder.WriteDebugLog(__FUNCTION__, "****Error: 初始化连接代理服务失败!");
        return false;
    }
    //初始化HTTP服务
    if (!InitHttpServer(m_pServerIP, m_nServerPort))
    {
        g_LogRecorder.WriteDebugLog(__FUNCTION__, "****Error: HTTP服务初始化失败!");
        return false;
    }
    
    return true;
}


bool CBatchStoreLibServer::InitDB()
{
    //连接MySQL数据库
    if (!m_mysqltool.mysql_connectDB(m_pConfigRead->m_sDBIP.c_str(), m_pConfigRead->m_nDBPort, m_pConfigRead->m_sDBName.c_str(),
        m_pConfigRead->m_sDBUid.c_str(), m_pConfigRead->m_sDBPwd.c_str(), "gb2312"))
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error: 连接数据库失败[%s:%d:%s]!",
            m_pConfigRead->m_sDBIP.c_str(), m_pConfigRead->m_nDBPort, m_pConfigRead->m_sDBName.c_str());
        return false;
    }
    else
    {
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "连接数据库成功[%s:%d:%s].",
            m_pConfigRead->m_sDBIP.c_str(), m_pConfigRead->m_nDBPort, m_pConfigRead->m_sDBName.c_str());
    }

    //获取本地服务, 特征值服务, 分析服务代理信息
    char pSQL[1024] = { 0 };
    sprintf_s(pSQL, sizeof(pSQL),
        "select serverType, IP, Port, servercode from %s where serverType in (247, 252, 253, 254, 255) order by servertype", SERVERINFOTABLE);
    int nRet = m_mysqltool.mysql_exec(_MYSQL_SELECT, pSQL);
    if (nRet > 0)
    {
        int nServerType = 0;
        while (m_mysqltool.mysql_getNextRow())
        {
            nServerType = m_mysqltool.mysql_getRowIntValue("serverType");
            
            if (247 == nServerType)    //当前批量入库服务
            {
                char pServerCode[64] = { 0 };
                strcpy_s(pServerCode, sizeof(pServerCode), m_mysqltool.mysql_getRowStringValue("servercode"));
                if (string(pServerCode) == m_pConfigRead->m_sServerCode)
                {
                    strcpy_s(m_pServerIP, sizeof(m_pServerIP), m_mysqltool.mysql_getRowStringValue("IP"));
                    m_nServerPort = m_mysqltool.mysql_getRowIntValue("Port");
                }
                printf("当前批量入库服务[%s:%d].\n", m_pServerIP, m_nServerPort);
            }
            else if (252 == nServerType)    //分析服务(核查库)
            {
                LPANALYSESERVERINFO pAnalyseServerInfo = new ANALYSESERVERINFO;
                strcpy_s(pAnalyseServerInfo->pServerID, sizeof(pAnalyseServerInfo->pServerID), m_mysqltool.mysql_getRowStringValue("servercode"));
                pAnalyseServerInfo->nServerType = 2;
                m_listAnalyseServerInfo.push_back(pAnalyseServerInfo);
                printf("分析核查服务[%s].\n", pAnalyseServerInfo->pServerID);
            }
            else if (253 == nServerType)    //分析服务(布控库)
            {
                LPANALYSESERVERINFO pAnalyseServerInfo = new ANALYSESERVERINFO;
                strcpy_s(pAnalyseServerInfo->pServerID, sizeof(pAnalyseServerInfo->pServerID), m_mysqltool.mysql_getRowStringValue("servercode"));
                pAnalyseServerInfo->nServerType = 3;
                m_listAnalyseServerInfo.push_back(pAnalyseServerInfo);
                printf("分析布控服务[%s].\n", pAnalyseServerInfo->pServerID);
            }
            else if (254 == nServerType) //特征值获取服务
            {
                char pSTServerIP[20] = { 0 };
                strcpy_s(pSTServerIP, sizeof(pSTServerIP), m_mysqltool.mysql_getRowStringValue("IP"));
                MAPSTSERVERINFO::iterator it = m_mapSTServerInfo.find(pSTServerIP);
                if (it == m_mapSTServerInfo.end())
                {
                    LPSTSERVERINFO pSTServerInfo = new STSERVERINFO;
                    strcpy_s(pSTServerInfo->pSTServerIP, sizeof(pSTServerInfo->pSTServerIP), pSTServerIP);
                    pSTServerInfo->nSTServerPort = m_mysqltool.mysql_getRowIntValue("Port");
                    pSTServerInfo->nType = 2;
                    m_mapSTServerInfo.insert(make_pair(pSTServerIP, pSTServerInfo));
                    printf("特征值获取服务[%s:%d].\n", pSTServerIP, pSTServerInfo->nSTServerPort);
                }
                else
                {
                    g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: ST服务器[%s]重复...", pSTServerIP);
                }
            }
            else if (255 == nServerType)    //代理服务
            {
                strcpy_s(m_pProxyServerIP, sizeof(m_pProxyServerIP), m_mysqltool.mysql_getRowStringValue("IP"));
                m_nProxyPort = m_mysqltool.mysql_getRowIntValue("Port");
                printf("代理服务[%s:%d].\n", m_pProxyServerIP, m_nProxyPort);
            }
        }
    }
    if(m_nServerPort == 0 || string(m_pServerIP) == "" || m_nProxyPort == 0 || string(m_pProxyServerIP) == "")
    {
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "****Error: 未获取到正确的服务配置, 程序结束...");
        return false;
    }
    return true;
}
bool CBatchStoreLibServer::GetStoreLibInfo()
{
    //获取重点库人脸信息(FaceUUID对应入ST库ImageID)
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "获取重点库人脸信息...");
    int nNum = 0;
    char pSQL[SQLMAXLEN] = { 0 };
    sprintf_s(pSQL, sizeof(pSQL),
        "select a.ID, c.FaceUUID, a.Type "
        "from %s a, %s c "
        "where a.id = c.LayoutLibId "
        "order by a.id", 
        STORELIBTABLE, STOREFACEINFOTABLE);
    int nRet = m_mysqltool.mysql_exec(_MYSQL_SELECT, pSQL);
    if (nRet > 0)
    {
        char pFaceUUID[64] = { 0 };

        int nStoreLibID = 0;
        while (m_mysqltool.mysql_getNextRow())
        {
            nStoreLibID =  m_mysqltool.mysql_getRowIntValue("id");
            LPSTORELIBINFO pStoreLibInfo = NULL;
            MAPSTORELIBINFO::iterator itLib = m_mapStoreLibInfo.find(nStoreLibID);
            if(itLib == m_mapStoreLibInfo.end())
            {
                pStoreLibInfo = new STORELIBINFO;
                pStoreLibInfo->nStoreLibID = nStoreLibID;
                pStoreLibInfo->nLibType = m_mysqltool.mysql_getRowIntValue("Type");
                m_mapStoreLibInfo.insert(make_pair(nStoreLibID, pStoreLibInfo));
            }
            else
            {
                pStoreLibInfo = itLib->second;
            }

            strcpy_s(pFaceUUID, sizeof(pFaceUUID), m_mysqltool.mysql_getRowStringValue("FaceUUID"));
            if(string(pFaceUUID) == "")
            {
                continue;
            }
            nNum ++;
            set<string>::iterator itFace = pStoreLibInfo->setStoreFace.find(pFaceUUID);
            if(itFace != pStoreLibInfo->setStoreFace.end())
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 人脸[%s]在重点库[%d]中信息重复!", pFaceUUID, nStoreLibID);
            }
            else
            {
                pStoreLibInfo->setStoreFace.insert(pFaceUUID);
            }

            if(nNum % 5000 == 0)
            {
                g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "己获取入重点库人脸信息: %d.", nNum);
            }
        }
    }
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "获取入重点库人脸信息完毕, 总数: %d.", nNum);
    return true;
}
bool CBatchStoreLibServer::InitHttpServer(string sHttpIP, int nHttpPort)
{
    if (NULL == m_pHttpServer)
    {
        m_pHttpServer = new CHttpServerAction;
    }
    if (!m_pHttpServer->StartHttpListen((char*)sHttpIP.c_str(), nHttpPort, HttpClientRequestCallback, this))
    {
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, 
            "****Error: 本地HTTP服务初始化失败!",
            sHttpIP.c_str(), nHttpPort);
        return false;
    }
    return true;
}
bool CBatchStoreLibServer::InitGetFeature()
{
    if (NULL == m_pGetFeature)
    {
        m_pGetFeature = new CGetFeature;
    }
    LPSTSERVERINFO pSTServerInfo = NULL;
    int i = 0;
    pSTServerInfo = new STSERVERINFO[m_mapSTServerInfo.size()];
    for (MAPSTSERVERINFO::iterator it = m_mapSTServerInfo.begin(); it != m_mapSTServerInfo.end(); it++)
    {
        strcpy_s(pSTServerInfo[i].pSTServerIP, sizeof(pSTServerInfo[i].pSTServerIP), it->second->pSTServerIP);
        pSTServerInfo[i].nSTServerPort = it->second->nSTServerPort;
        pSTServerInfo[i].nType = it->second->nType;

        i++;
    }
        
    if (0 != m_pGetFeature->Init(ImageInfoCallback, this, pSTServerInfo, i))
    {
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "****Error: 初始化连接商汤服务失败!");
        return false;
    }

    return true;
}
bool CBatchStoreLibServer::InitAnalyse()
{
    if (NULL == m_pAnalyseStore)
    {
        m_pAnalyseStore = new CAnalyseStore(m_pSavePath[0]);
    }

    if (!m_pAnalyseStore->Init(m_pProxyServerIP, m_nProxyPort, (char *)m_pConfigRead->m_sServerCode.c_str(), 
                                m_pServerIP, m_listAnalyseServerInfo, m_pConfigRead->m_nQuality))
    {
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "****Error: 初始化连接分析代理服务失败!");
        return false;
    }

    //同步重点库名
    Sleep(1000);
    MAPSTORELIBINFO::iterator itLib = m_mapStoreLibInfo.begin();
    for (; itLib != m_mapStoreLibInfo.end(); itLib++)
    {
        m_pAnalyseStore->AddKeyLib(itLib->first, itLib->second->nLibType);
    }
    return true;
}

//HTTP服务信息回调
void CALLBACK CBatchStoreLibServer::HttpClientRequestCallback(LPHTTPREQUEST pHttpRequest, void * pUser)
{
    CBatchStoreLibServer * pThis = (CBatchStoreLibServer *)pUser;
    EnterCriticalSection(&pThis->m_Httpcs);
    pThis->m_listHttpRequest.push_back(pHttpRequest);
    LeaveCriticalSection(&pThis->m_Httpcs);
}
DWORD WINAPI CBatchStoreLibServer::HTTPRequestParseThread(LPVOID lParam)
{
    CBatchStoreLibServer * pThis = (CBatchStoreLibServer*)lParam;
    pThis->HTTPRequestParseAction();
    return 0;
}
//http请求信息处理线程
void CBatchStoreLibServer::HTTPRequestParseAction()
{
    LPHTTPREQUEST pHttpRequest = NULL;
    int nTaskNum = 0;
    while (WAIT_TIMEOUT == WaitForSingleObject(m_hStopEvent, THREADWAITTIME))
    {
        do
        {
            EnterCriticalSection(&m_Httpcs);
            if (m_listHttpRequest.size() > 0)
            {
                pHttpRequest = m_listHttpRequest.front();
                m_listHttpRequest.pop_front();
                nTaskNum = m_listHttpRequest.size();
            }
            else
            {
                pHttpRequest = NULL;
                nTaskNum = 0;
            }
            LeaveCriticalSection(&m_Httpcs);

            if(NULL != pHttpRequest)
            {
                switch (pHttpRequest->nOperatorType)
                {
                case 1:     //增加重点库人脸图片
                    {
                        AddStoreLibFace(pHttpRequest);
                        break;
                    }
                case 2:     //删除重点库人脸图片
                    {
                        DelStoreLibFace(pHttpRequest);
                        break;
                    }   
                case 5:     //删除重点库
                    {
                        DelStoreLib(pHttpRequest);
                        break;
                    }
                default:
                    break;
                }

                delete pHttpRequest;
                pHttpRequest = NULL;
            }
        }while(nTaskNum > 0);
    }
    return;
}

bool CBatchStoreLibServer::AddStoreLibFace(LPHTTPREQUEST pHttpRequest)
{
    GetLocalTime(&m_sysTime);
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "收到增加重点库[%d]人脸消息, 数量[%d], 时间[%02d:%03d].", 
        pHttpRequest->pStoreInfo->nStoreLibID, pHttpRequest->pStoreInfo->mapZBase64FaceInfo.size(), m_sysTime.wSecond, m_sysTime.wMilliseconds);
    LPSTOREFACEINFO pStoreImageInfo = GetFreeFaceInfo();
    if(NULL == pStoreImageInfo)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 无可用资源...请稍候上传");
        //向客户端发送回应消息
        SendResponseMsg(pHttpRequest->ClientSocket, NotEnoughResource);
    }
    else
    {
        //将图片ZBase64流数据保存到pStoreImageInfo
        for(MAPZBASE64FACE::iterator itZBase = pHttpRequest->pStoreInfo->mapZBase64FaceInfo.begin();
            itZBase != pHttpRequest->pStoreInfo->mapZBase64FaceInfo.end(); itZBase ++)
        {
            LPPUSHSTIMAGEINFO pPushSTImageInfo = NULL;
            LISTPUSHSTIMAGEINFO::iterator itImage = pStoreImageInfo->listPushSTImageInfo.begin();
            for(; itImage != pStoreImageInfo->listPushSTImageInfo.end(); itImage ++)
            {
                if(!(*itImage)->bUsed)
                {
                    pPushSTImageInfo = *itImage;
                    break;
                }
            }
            if(itImage == pStoreImageInfo->listPushSTImageInfo.end())
            {
                pPushSTImageInfo = new PUSHSTIMAGEINFO;
                pStoreImageInfo->listPushSTImageInfo.push_back(pPushSTImageInfo);
            }
            if(pPushSTImageInfo->nImageBufMaxLen < itZBase->second.size())
            {
                delete pPushSTImageInfo->pImageBuf;
                pPushSTImageInfo->pImageBuf = new char[itZBase->second.size() + 1];
                pPushSTImageInfo->nImageBufMaxLen = itZBase->second.size() + 1;
            }
            memcpy(pPushSTImageInfo->pImageBuf, itZBase->second.c_str(), itZBase->second.size());
            pPushSTImageInfo->nImageLen = itZBase->second.size() ;

            //保存图片ID
            strcpy_s(pPushSTImageInfo->pImageID, sizeof(pPushSTImageInfo->pImageID), itZBase->first.c_str());

            //保存真实图片名
            MAPZBASE64FACE::iterator itName = pHttpRequest->pStoreInfo->mapZBaseImageName.find(itZBase->first);
            if (itName != pHttpRequest->pStoreInfo->mapZBaseImageName.end())
            {
                strcpy_s(pPushSTImageInfo->pImageName, sizeof(pPushSTImageInfo->pImageName), itName->second.c_str());
            }

            //if(itImage == pStoreImageInfo->listPushSTImageInfo.end())
            {
                pPushSTImageInfo->bUsed = true;
            }
        }

        pStoreImageInfo->nStoreLibID = pHttpRequest->pStoreInfo->nStoreLibID;
        strcpy(pStoreImageInfo->pStoreLibName, pHttpRequest->pStoreInfo->pStoreLibName);
        pStoreImageInfo->nLibType = pHttpRequest->pStoreInfo->nLibType;
        pStoreImageInfo->ClientSocket = pHttpRequest->ClientSocket;

        //传入商汤重点库
        m_pGetFeature->AddFaceImageInfo(pStoreImageInfo);
    }
    
    return true;
}
bool CBatchStoreLibServer::DelStoreLibFace(LPHTTPREQUEST pHttpRequest)
{
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "收到删除重点库人脸消息...");

    int nRet = 0;
    std::list<std::string>::iterator itFaceUUID = pHttpRequest->pStoreInfo->listFaceUUID.begin();
    if(itFaceUUID == pHttpRequest->pStoreInfo->listFaceUUID.end())  // 删除重点库所有人脸图片
    {
        //清空分析服务重点库
        m_pAnalyseStore->ClearLib(pHttpRequest->pStoreInfo->nStoreLibID);
        //清空本地磁盘重点库文件夹
        DelLibFaceFromDisk(pHttpRequest->pStoreInfo->nStoreLibID, false);
        //清空map中保存的重点库人脸信息
        DelStoreLibToMap(pHttpRequest->pStoreInfo->nStoreLibID, false);
    }
    else        //删除重点库指定图片
    {
        for(; itFaceUUID != pHttpRequest->pStoreInfo->listFaceUUID.end(); itFaceUUID ++)
        {
            //从分析服务重点库删除图片
            m_pAnalyseStore->DelFeature(pHttpRequest->pStoreInfo->nStoreLibID, (*itFaceUUID).c_str());
            
            //从本地磁盘中删除图片
            char pFilePath[128] = {0};
            sprintf_s(pFilePath, sizeof(pFilePath), "%s/%d/%s.jpg", m_pSavePath, pHttpRequest->pStoreInfo->nStoreLibID, (*itFaceUUID).c_str());
            if(remove(pFilePath) < 0)
            {
                g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "删除文件[%s]失败, 错误码[%d].", pFilePath, GetLastError());
            }
            else
            {
                g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "从本地磁盘删除重点库人脸图片成功[%s].", pFilePath);
            }

            //从本地map删除图片信息
            DelStoreFaceToMap(*itFaceUUID, pHttpRequest->pStoreInfo->nStoreLibID);
        }
    }

    //向客户端发送回应消息
    SendResponseMsg(pHttpRequest->ClientSocket, nRet);
    return true;
}
bool CBatchStoreLibServer::DelStoreLib(LPHTTPREQUEST pHttpRequest)
{
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "收到删除重点库消息...");

    //删除分析服务重点库
    int nRet = m_pAnalyseStore->DelLib(pHttpRequest->pStoreInfo->nStoreLibID);

    //向客户端发送回应消息
    SendResponseMsg(pHttpRequest->ClientSocket, nRet);

    //从本地磁盘删除重点库文件夹
    DelLibFaceFromDisk(pHttpRequest->pStoreInfo->nStoreLibID);

    //从本地map里删除重点库
    DelStoreLibToMap(pHttpRequest->pStoreInfo->nStoreLibID);
    return true;
}
//ST入库后回调入库结果
void CALLBACK CBatchStoreLibServer::ImageInfoCallback(LPSTOREFACEINFO pStoreImageInfo, void * pUser)
{
    CBatchStoreLibServer * pThis = (CBatchStoreLibServer *)pUser;
    if (pStoreImageInfo->nEvent != 0)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: Get Image Feature Failed");
        pThis->SendResponseMsg(pStoreImageInfo->ClientSocket, pStoreImageInfo->nEvent);
    }
    else
    {
        //推送特征值给分析服务入库
        //先查找当前重点库是否己存在, 不存在则创建, 并通知分析服务创建重点库
        MAPSTORELIBINFO::iterator itLib = pThis->m_mapStoreLibInfo.find(pStoreImageInfo->nStoreLibID);
        if (itLib == pThis->m_mapStoreLibInfo.end())
        {
            pThis->m_pAnalyseStore->AddKeyLib(pStoreImageInfo->nStoreLibID, pStoreImageInfo->nLibType);
            Sleep(1500);
        }
        int nRet = pThis->m_pAnalyseStore->AddFeature(pStoreImageInfo, pThis->m_pSavePath);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 向分析服务入库失败, 返回[%d].", nRet);
            pThis->SendResponseMsg(pStoreImageInfo->ClientSocket, PushFeatureToAnalyseFailed);
        }
        else
        {
            //分析服务入库后返回给Web回应消息
            //生成回应JSON串
            rapidjson::Document document;
            document.SetObject();
            rapidjson::Document::AllocatorType&allocator = document.GetAllocator();
            rapidjson::Value array(rapidjson::kArrayType);

            LISTPUSHSTIMAGEINFO::iterator itImage = pStoreImageInfo->listPushSTImageInfo.begin();
            for (; itImage != pStoreImageInfo->listPushSTImageInfo.end(); itImage++)
            {
                if ((*itImage)->bUsed)
                {
                    rapidjson::Value object(rapidjson::kObjectType);
                    if ((*itImage)->nFaceQuality >= pThis->m_pConfigRead->m_nQuality && (*itImage)->nFaceQuality <= 100)
                    {
                        sprintf_s((*itImage)->pFilePath, sizeof((*itImage)->pFilePath),
                            "%s/%d/%s.jpg", pThis->m_pSavePath, pStoreImageInfo->nStoreLibID, (*itImage)->pFaceUUID);

                        object.AddMember("Name", rapidjson::StringRef((*itImage)->pImageID), allocator);
                        object.AddMember("FaceUUID", rapidjson::StringRef((*itImage)->pFaceUUID), allocator);
                        object.AddMember("ImageID", rapidjson::StringRef("NULL"), allocator);
                        object.AddMember("Feature", rapidjson::StringRef("NULL"/*(*itImage)->pFeature*/), allocator);
                        object.AddMember("SavePath", rapidjson::StringRef((*itImage)->pFilePath), allocator);
                        object.AddMember(JSONFACEURL, rapidjson::StringRef((*itImage)->pFaceURL), allocator);
                        object.AddMember("errormessage", rapidjson::StringRef(""), allocator);

                        array.PushBack(object, allocator);

                        pThis->SaveFileToDisk(pStoreImageInfo->nStoreLibID, (*itImage)->pImageBuf, (*itImage)->nImageLen, (*itImage)->pFilePath);

                        //将增加的图片信息保存到本地map
                        pThis->AddStoreFaceToMap((*itImage)->pFaceUUID, (*itImage)->pSTImageID, pStoreImageInfo->nStoreLibID);

                        g_LogRecorder.WriteInfoLogEx(__FUNCTION__, "Name[%s], FaceUUID[%s], ImageID[%s], Path[%s]",
                            (*itImage)->pImageID, (*itImage)->pFaceUUID, (*itImage)->pSTImageID, (*itImage)->pFilePath);
                    }
                    else
                    {
                        object.AddMember("Name", rapidjson::StringRef((*itImage)->pImageID), allocator);
                        object.AddMember("FaceUUID", rapidjson::StringRef(""), allocator);
                        object.AddMember("ImageID", rapidjson::StringRef(""), allocator);
                        object.AddMember("Feature", rapidjson::StringRef(""), allocator);
                        object.AddMember(JSONFACEURL, rapidjson::StringRef(""), allocator);
                        object.AddMember("SavePath", rapidjson::StringRef(""), allocator);

                        if ((*itImage)->nFaceQuality > 100)
                        {
                            pThis->GetErrorMsg((*itImage)->nFaceQuality, (*itImage)->pErrorMsg);
                        }
                        else
                        {
                            strcpy((*itImage)->pErrorMsg, "QualityScore too low");
                        }
                        object.AddMember("errormessage", rapidjson::StringRef((*itImage)->pErrorMsg), allocator);

                        array.PushBack(object, allocator);
                    }
                }

            }
            document.AddMember("Result", rapidjson::StringRef("success"), allocator);
            document.AddMember("Photo", array, allocator);

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.Accept(writer);
            string sBody = buffer.GetString();
            //向客户端发送回应消息
            pThis->m_pHttpServer->ResponseBody(pStoreImageInfo->ClientSocket, "", sBody);

            GetLocalTime(&pThis->m_sysTime);
            g_LogRecorder.WriteInfoLogEx(__FUNCTION__, "回应客户端时间[%02d:%03d].", pThis->m_sysTime.wSecond, pThis->m_sysTime.wMilliseconds);
        }
    }
    //回收资源
    pThis->RecoverFaceInfo(pStoreImageInfo);

    return;
}
void CBatchStoreLibServer::SendResponseMsg(SOCKET ClientSocket, int nEvent)
{
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType&allocator = document.GetAllocator();
    if(nEvent == 0)
    {
        document.AddMember("Result", rapidjson::StringRef("success"), allocator);
    }
    else
    {
        char pMsg[128] = { 0 };
        GetErrorMsg(nEvent, pMsg);
        document.AddMember("Result", rapidjson::StringRef("error"), allocator);
        document.AddMember("ErrorMessage", rapidjson::StringRef(pMsg), allocator);
    }
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    string sBody = buffer.GetString();
    //向客户端发送回应消息
    m_pHttpServer->ResponseBody(ClientSocket, "", sBody);
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "回应客户端json[%s].", sBody.c_str());
    return;
}
//添加重点人脸图片到本地map保存
bool CBatchStoreLibServer::AddStoreFaceToMap(std::string sFaceUUID, std::string sSTImageID, unsigned int nStoreLibID)
{
    EnterCriticalSection(&m_cs);
    MAPSTORELIBINFO::iterator itLib = m_mapStoreLibInfo.find(nStoreLibID);
    if(itLib != m_mapStoreLibInfo.end())
    {
        set<string>::iterator itFace = itLib->second->setStoreFace.find(sFaceUUID);
        if(itFace == itLib->second->setStoreFace.end())
        {
            itLib->second->setStoreFace.insert(sFaceUUID);
        }
        else
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 增加重点人脸图片[FaceUUID: %s]己存在重点库[%d].", sFaceUUID.c_str(), nStoreLibID);
        }
    }
    else
    {
        LPSTORELIBINFO pStoreLibInfo = new STORELIBINFO;
        m_mapStoreLibInfo.insert(make_pair(nStoreLibID, pStoreLibInfo));
        pStoreLibInfo->nStoreLibID = nStoreLibID;
        pStoreLibInfo->setStoreFace.insert(sFaceUUID);
    }
    LeaveCriticalSection(&m_cs);
    return true;
}
//从本地map删除重点人脸图片
bool CBatchStoreLibServer::DelStoreFaceToMap(std::string sFaceUUID, unsigned int nStoreLibID)
{
    EnterCriticalSection(&m_cs);
    MAPSTORELIBINFO::iterator itLib = m_mapStoreLibInfo.find(nStoreLibID);
    if(itLib != m_mapStoreLibInfo.end())
    {
        set<string>::iterator itFace = itLib->second->setStoreFace.find(sFaceUUID);
        if(itFace != itLib->second->setStoreFace.end())
        {
            itLib->second->setStoreFace.erase(sFaceUUID);
        }
        else
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 删除重点人脸图片[FaceUUID: %s]不存在于重点库[%d].", sFaceUUID.c_str(), nStoreLibID);
        }
    }
    LeaveCriticalSection(&m_cs);
    return true;
}
bool CBatchStoreLibServer::DelStoreLibToMap(unsigned int nStoreLibID, bool bDelLib)
{
    EnterCriticalSection(&m_cs);
    MAPSTORELIBINFO::iterator itLib = m_mapStoreLibInfo.find(nStoreLibID);
    if(itLib != m_mapStoreLibInfo.end())
    {
        if(bDelLib) //删除重点库
        {
            m_mapStoreLibInfo.erase(itLib);
        }
        else        //清空重点库人脸图片信息
        {
            itLib->second->setStoreFace.clear();
        }
    }
    LeaveCriticalSection(&m_cs);
    return true;
}
bool CBatchStoreLibServer::SaveFileToDisk(unsigned int nStoreLibID, char * pImageBuf, int nLen, char * pFilePath)
{
    int nError = 0;
    //写入本地磁盘保存
    HANDLE hFileHandle = CreateFile(pFilePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (INVALID_HANDLE_VALUE == hFileHandle)
    {
        nError = GetLastError();
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, 
            "***Warning: 创建图片失败, 检查图片路径[%s], errorCode[%d]...", pFilePath, nError);
        if(3 == nError) //文件夹不存在
        {
            char pPath[128] = {0};
            sprintf_s(pPath, sizeof(pPath), "%s/%d", m_pSavePath, nStoreLibID);
            if(!CreateDirectory(pPath, NULL))
            {
                nError = -1;
            }
            else
            {
                g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "创建图片路径成功[%s].", pPath);
                hFileHandle = CreateFile(pFilePath,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    CREATE_NEW,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
                if (INVALID_HANDLE_VALUE == hFileHandle)
                {
                    nError = -1;
                }
                else
                {
                    nError = 0;
                }
            }
        }
    }

    if(nError == 0)
    {
        DWORD dwWrite = 0;
        WriteFile(hFileHandle, pImageBuf, nLen, &dwWrite, NULL);
        CloseHandle(hFileHandle);
        g_LogRecorder.WriteInfoLogEx(__FUNCTION__, "重点库人脸图片保存到本地磁盘[%s].", pFilePath);
        return true;
    }
    else
    {
        return false;
    }
}
bool CBatchStoreLibServer::DelLibFaceFromDisk(unsigned int nStoreLibID, bool bDelDir)
{
    MAPSTORELIBINFO::iterator itLib = m_mapStoreLibInfo.find(nStoreLibID);
    if(m_mapStoreLibInfo.end() != itLib)
    {
        char pFilePath[128] = {0};
        set<string>::iterator itFace = itLib->second->setStoreFace.begin();
        for(; itFace != itLib->second->setStoreFace.end(); itFace ++)
        {
            //从本地磁盘中删除布控图片
            sprintf_s(pFilePath, sizeof(pFilePath), "%s/%d/%s.jpg", m_pSavePath, nStoreLibID,  (*itFace).c_str());
            int nRet = remove(pFilePath);
            if(nRet < 0)
            {
                g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "删除文件[%s]失败, 错误码[%d].", pFilePath, GetLastError());
            }
            else
            {
                g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "从本地磁盘删除重点库人脸图片成功[%s].", pFilePath);
            }
        }
        sprintf_s(pFilePath, sizeof(pFilePath), "%s/%d", m_pSavePath, nStoreLibID);
        if(bDelDir) //删除文件夹
        {
            if(!RemoveDirectory(pFilePath))
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "删除文件夹[%s]失败, 错误码[%d].", pFilePath, GetLastError());
            }
        }
        
    }
    return true;
}
LPSTOREFACEINFO CBatchStoreLibServer::GetFreeFaceInfo()
{
    LPSTOREFACEINFO pStoreImageInfo = NULL;
    EnterCriticalSection(&m_cs);
    if (m_listStoreImageInfo.size() > 0)
    {
        pStoreImageInfo = m_listStoreImageInfo.front();
        m_listStoreImageInfo.pop_front();
    }
    LeaveCriticalSection(&m_cs);
    return pStoreImageInfo;
}
void CBatchStoreLibServer::RecoverFaceInfo(LPSTOREFACEINFO pStoreImageInfo)
{
    EnterCriticalSection(&m_cs);
    pStoreImageInfo->Init();
    m_listStoreImageInfo.push_back(pStoreImageInfo);
    LeaveCriticalSection(&m_cs);
}

int CBatchStoreLibServer::GetErrorMsg(int nError, char * pMsg)
{
    int nRet = 0;
    switch ((ErrorCode)nError)
    {
    case ServerNotInit:
        strcpy(pMsg, "SERVER_NOT_INIT");
        break;
    case DBAleadyExist:
        strcpy(pMsg, "DB_ALEADY_EXIST");
        break;
    case DBNotExist:
        strcpy(pMsg, "DB_NOT_EXIST");
        break;
    case FaceUUIDAleadyExist:
        strcpy(pMsg, "FACEUUID_ALEADY_EXIST");
        break;
    case FaceUUIDNotExist:
        strcpy(pMsg, "FACEUUID_NOT_EXIST");
        break;
    case ParamIllegal:
        strcpy(pMsg, "PARAM_ILLEGAL");
        break;
    case NewFailed:
        strcpy(pMsg, "NEW_FAILED");
        break;
    case JsonFormatError:
        strcpy(pMsg, "JSON_FORMAT_ERROR");
        break;
    case CommandNotFound:
        strcpy(pMsg, "COMMAND_NOT_FOUND");
        break;
    case HttpMsgUpperLimit:
        strcpy(pMsg, "HTTPMSG_UPPERLIMIT");
        break;
    case PthreadMutexInitFailed:
        strcpy(pMsg, "PTHREAD_MUTEX_INIT_FAILED");
        break;
    case FeatureNumOverMax:
        strcpy(pMsg, "FEATURE_NUM_OVER_MAX");
        break;
    case JsonParamIllegal:
        strcpy(pMsg, "JSON_PARAM_ILLEGAL");
        break;
    case MysqlQueryFailed:
        strcpy(pMsg, "MYSQL_QUERY_FAILED");
        break;
    case PushFeatureToAnalyseFailed:
        strcpy(pMsg, "PUSH_FEATURE_TO_ANALYSE_FAILED");
        break;
    case STSDKInitFailed:
        strcpy(pMsg, "STSDK_INIT_FAILED");
        break;
    case GetLocalPathFailed:
        strcpy(pMsg, "GET_LOCAL_PATH_FAILED");
        break;
    case LoadLibFailed:
        strcpy(pMsg, "LOAD_LIB_FAILED");
        break;
    case GetProcAddressFailed:
        strcpy(pMsg, "GET_PROC_ADDRESS_FAILED");
        break;
    case FRSEngineInitFailed:
        strcpy(pMsg, "FRSENGINE_INIT_FAILED");
        break;
    case ConvertBMPFailed:
        strcpy(pMsg, "CONVERT_BMP_FAILED");
        break;
    case GetLocalFeatureFailed:
        strcpy(pMsg, "GET_LOCAL_FEATURE_FAILED");
        break;
    case STJsonFormatFailed:
        strcpy(pMsg, "STJSON_FORMAT_FAILED");
        break;
    case AddImageToSTFailed:
        strcpy(pMsg, "ADD_IMAGE_TO_ST_FAILED");
        break;
    case STRepGetFeatureFailed:
        strcpy(pMsg, "STREP_GET_FEATURE_FAILED");
        break;
    case GetSTFeatureFailed:
        strcpy(pMsg, "GET_ST_FEATURE_FAILED");
        break;
    case NotEnoughResource:
        strcpy(pMsg, "NOT_ENOUGH_RESOURCE");
        break;
    case GetPictureFailed:
        strcpy(pMsg, "GET_PICTURE_FAILED");
        break;
    default:
        nRet = -1;
        break;
    }

    return nRet;
}

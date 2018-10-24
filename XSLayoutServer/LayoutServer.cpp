#include "StdAfx.h"
#include "LayoutServer.h"
#include <iostream>
#include <objbase.h>

CLogRecorder g_LogRecorder;
CLayoutServer::CLayoutServer(void)
{
    InitializeCriticalSection(&m_cs);
    InitializeCriticalSection(&m_csHttp);
    InitializeCriticalSection(&m_csFeature);
    InitializeCriticalSection(&m_csTask);
    m_hStopEvent = CreateEvent(NULL, true, false, NULL);
    m_pRedisManage = NULL;
    m_pAnalyseSearch = NULL;
    m_pHttpServer = NULL;
    m_pCheckSubManage = NULL;
    m_pZeromqManage = NULL;

    m_nImageNum = 0;
    m_nAnalyseSearchNum = 0;
    m_pConfigRead = new CConfigRead;

    ZeroMemory(m_pServerIP, sizeof(m_pServerIP));
    m_nServerPort = 0;

    ZeroMemory(m_pRedisIP, sizeof(m_pRedisIP));
    m_nRedisPort = 0;
    m_bPrintSnap = false;
}

CLayoutServer::~CLayoutServer(void)
{
    CloseHandle(m_hStopEvent);
    Sleep(100);
    if (NULL != m_pRedisManage)
    {
        delete m_pRedisManage;
        m_pRedisManage = NULL;
    }
    if (NULL != m_pAnalyseSearch)
    {
        delete m_pAnalyseSearch;
        m_pAnalyseSearch = NULL;
    }
    if (NULL != m_pHttpServer)
    {
        delete m_pHttpServer;
        m_pHttpServer = NULL;
    }
    if (NULL != m_pCheckSubManage)
    {
        delete m_pCheckSubManage;
        m_pCheckSubManage = NULL;
    }
    if (NULL != m_pZeromqManage)
    {
        delete m_pZeromqManage;
        m_pZeromqManage = NULL;
    }
    DeleteCriticalSection(&m_cs);
    DeleteCriticalSection(&m_csHttp);
    DeleteCriticalSection(&m_csFeature);
    DeleteCriticalSection(&m_csTask);
}
bool CLayoutServer::Init()
{
    //初始化日志
    string sPath = m_pConfigRead->GetCurrentPath();
    string sConfigPath = sPath + "/Config/XSLayoutServer_config.properties";
#ifdef _DEBUG
    sConfigPath = "./Config/XSLayoutServer_config.properties";
#endif
    g_LogRecorder.InitLogger(sConfigPath.c_str(), "XSLayoutServerLogger", "XSLayoutServer");

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

    //初始化连接DB并获取服务相关信息
    if (!InitDB())
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error: 初始化连接数据库获取服务配置信息失败!");
        return false;
    }
    //数据库查询布控卡口信息
    if (!GetLayoutCameraInfo())
    {
        g_LogRecorder.WriteDebugLog(__FUNCTION__, "****Error: 获取布控信息失败.");
        return false;
    }
    //数据库查询接口服务布控任务信息
    if (!GetLayoutTaskInfo())
    {
        g_LogRecorder.WriteDebugLog(__FUNCTION__, "****Error: 获取接口服务布控任务失败.");
        return false;
    }
    //初始化HTTP服务
    if (!InitHttpServer(m_pServerIP, m_nServerPort))
    {
        g_LogRecorder.WriteDebugLog(__FUNCTION__, "****Error: HTTP服务初始化失败!");
        return false;
    }
    //初始化分析服务服务连接
    if (!InitAnalyseSearch())
    {
        g_LogRecorder.WriteDebugLog(__FUNCTION__, "****Error: 分析服务服务连接失败!");
        return false;
    }
    //初始化Redis连接
    if (!InitRedisManage(m_pRedisIP, m_nRedisPort))
    {
        g_LogRecorder.WriteDebugLog(__FUNCTION__, "****Error: Redis初始化连接失败!");
        return false;
    }
   
    if (!InitCheckpointSubManage())
    {
        g_LogRecorder.WriteDebugLog(__FUNCTION__, "****Error: InitCheckpointSubManage初始化失败.");
        return false;
    }
    //初始化Zeromq
    if (!InitZeroMq())
    {
        g_LogRecorder.WriteDebugLog(__FUNCTION__, "****Error: InitZeroMq初始化失败.");
        return false;
    }
    return true;
}
bool CLayoutServer::StopLayoutServer()
{
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "**********结束服务**********");
    m_pRedisManage->DisConnect();
    m_pAnalyseSearch->UnInit();
    m_pHttpServer->StopHttpServer();
    m_pCheckSubManage->UnInit();
    SetEvent(m_hStopEvent);
    return true;
}
bool CLayoutServer::StartLayoutServer()
{
    if(!Init())
    {
        g_LogRecorder.WriteErrorLog(__FUNCTION__, "****Error: 服务初始化失败.");
        return false;
    }

    Sleep(1000 * 1);
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "\r\n**************************布控服务启动成功**************************");   


    CreateThread(NULL, 0, HTTPRequestParseThread, this, NULL, 0);   //http请求处理线程
    CreateThread(NULL, 0, SubCheckpointThread, this, NULL, 0);   //http请求处理线程

#ifdef _DEBUG
    while (true)
    {
        char pIn;
        cin >> pIn;
        switch (pIn)
        {
        case 'a':
            {
                printf("布控库信息列表: \n");
                for (MAPLAYOUTCHECKPOINTINFO::iterator it = m_mapLayoutCheckpointInfo.begin(); it != m_mapLayoutCheckpointInfo.end(); it++)
                {
                    printf("布控卡口[%s]\n", it->first.c_str());
                    for (MAPLAYOUTLIBINFO::iterator itLib = it->second->mapLayoutLibInfo.begin();
                        itLib != it->second->mapLayoutLibInfo.end(); itLib++)
                    {
                        printf("\t布控库[%03d][%s - %s][%d]\n", itLib->first, ChangeSecondToTime(itLib->second->nBeginTime).c_str(),
                            ChangeSecondToTime(itLib->second->nEndTime).c_str(), itLib->second->nScore);
                    }
                }
                    
                printf("\n---zmq回调图片数量[%d], SearchResult[%d]---\n==================================\n", m_nImageNum, m_nAnalyseSearchNum);

                printf("布控任务信息列表: \n");
                EnterCriticalSection(&m_csTask);
                MAPLAYOUTTASKINFO::iterator itTask = m_mapLayoutTaskInfo.begin();
                for (; itTask != m_mapLayoutTaskInfo.end(); itTask++)
                {
                    printf("TaskID[%s]\r\n[%s - %s]\n", itTask->first.c_str(), itTask->second->pBeginTime, itTask->second->pEndTime);
                    LPLAYOUTTASKINFO pTaskInfo = itTask->second;
                    map<int, LPTASKCHECKPOINT>::iterator itLib = pTaskInfo->mapTaskLib.begin();
                    for (; itLib != pTaskInfo->mapTaskLib.end(); itLib++)
                    {
                        printf("\tLibID[%d]:\n", itLib->first);
                        map<string, int>::iterator itCheckpoint = itLib->second->mapTaskCheckpoint.begin();
                        for (; itCheckpoint != itLib->second->mapTaskCheckpoint.end(); itCheckpoint++)
                        {
                            printf("\t\tCheckpoint[%s]:%d\n", itCheckpoint->first.c_str(), itCheckpoint->second);
                        }
                    }
                }
                LeaveCriticalSection(&m_csTask);
            }
            
            break;
        case 't':
        {
            m_pRedisManage->PublishAlarmInfo("123", "1235");
            break;
        }
        case 'p':
        {
            m_bPrintSnap = !m_bPrintSnap;
        }
        case 's':
        {
            LPSUBMESSAGE pSubMessage = new SUBMESSAGE;
            strcpy(pSubMessage->pHead, "KAFKAALARM");
            strcpy(pSubMessage->pOperationType, "AlarmInfo");
            strcpy(pSubMessage->pSource, m_pConfigRead->m_sServerCode.c_str());

            rapidjson::Document document;
            document.SetObject();
            rapidjson::Document::AllocatorType&allocator = document.GetAllocator();

            document.AddMember("device_id", rapidjson::StringRef("ZPJ15102209650960007079"), allocator);
            document.AddMember("faceuuid", rapidjson::StringRef("ce8df7cf8f454c3799eef16feb0b0522"), allocator);
            document.AddMember("layoutlib_id", rapidjson::StringRef("42"), allocator);
            document.AddMember("layoutfaceuuid", rapidjson::StringRef("8A2275F2E76944f0A1C57D6FE27F8228"), allocator);
            document.AddMember("face_time", rapidjson::StringRef("1512616864000"), allocator);
            document.AddMember("score", rapidjson::StringRef("72"), allocator);

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.Accept(writer);
            pSubMessage->sPubJsonValue = buffer.GetString();

            m_pZeromqManage->PubMessage(pSubMessage);
            break;
        }
        case 'e':
            StopLayoutServer();
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
bool CLayoutServer::InitDB()
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

    //获取Redis, 分析服务服务器信息
    char pSQL[1024] = { 0 };
    sprintf_s(pSQL, sizeof(pSQL),
        "select servercode, serverType, IP, Port, servercode from %s where serverType in (236, 255, 242)", SERVERINFOTABLE);
    int nRet = m_mysqltool.mysql_exec(_MYSQL_SELECT, pSQL);
    if (nRet > 0)
    {
        int nServerType = 0;
        while (m_mysqltool.mysql_getNextRow())
        {
            nServerType = m_mysqltool.mysql_getRowIntValue("serverType");
            if (236 == nServerType)     //Redis结果数据库
            {
                strcpy_s(m_pRedisIP, sizeof(m_pRedisIP), m_mysqltool.mysql_getRowStringValue("IP"));
                m_nRedisPort = m_mysqltool.mysql_getRowIntValue("Port");
                printf("Redis结果数据库[%s:%d].\n", m_pRedisIP, m_nRedisPort);
            }
            else if (255 == nServerType)     //分析代理服务
            {
                strcpy_s(m_pProxyServerIP, sizeof(m_pProxyServerIP), m_mysqltool.mysql_getRowStringValue("IP"));
                m_nProxyPort = m_mysqltool.mysql_getRowIntValue("Port");
                printf("代理服务[%s:%d].\n", m_pProxyServerIP, m_nProxyPort);
            }
            else if (242 == nServerType)    //本地布控服务
            {
                char pServerCode[64] = { 0 };
                strcpy_s(pServerCode, sizeof(pServerCode), m_mysqltool.mysql_getRowStringValue("servercode"));
                if (string(pServerCode) == m_pConfigRead->m_sServerCode)
                {
                    strcpy_s(m_pServerIP, sizeof(m_pServerIP), m_mysqltool.mysql_getRowStringValue("IP"));
                    m_nServerPort = m_mysqltool.mysql_getRowIntValue("Port");
                }
            }
        }
        if(m_nServerPort == 0 || string(m_pServerIP) == "" || m_nRedisPort == 0 || string(m_pRedisIP) == "")
        {
            g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "****Error: 未获取到正确的服务配置, 程序结束...");
            return false;
        }
    }
    return true;
}
bool CLayoutServer::GetLayoutCameraInfo()
{
    //获取布控卡口信息
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "获取布控卡口信息...");
    char pSQL[SQLMAXLEN] = { 0 };
    sprintf_s(pSQL, sizeof(pSQL),
        "select a.ID, a.BeginTime, a.EndTime, b.CheckPoint, b.Score "
        "from %s a, %s b "
        "where a.id = b.LayoutLibId "
        "order by a.id, b.CheckPoint", 
        STORELIBTABLE, LAYOUTCHECKPOINTTABLE);
    int nRet = m_mysqltool.mysql_exec(_MYSQL_SELECT, pSQL);
    if (nRet > 0)
    {
        unsigned int nLayoutLibID = 0;
        char pCheckPoint[64] = { 0 };
        char pBeginTime[20] = { 0 };
        char pEndTime[20] = { 0 };
        int nScore = 0;
        while (m_mysqltool.mysql_getNextRow())
        {
            nLayoutLibID =  m_mysqltool.mysql_getRowIntValue("id");
            strcpy_s(pBeginTime, sizeof(pBeginTime), m_mysqltool.mysql_getRowStringValue("BeginTime"));         
            strcpy_s(pEndTime, sizeof(pEndTime), m_mysqltool.mysql_getRowStringValue("EndTime"));   
            strcpy_s(pCheckPoint, sizeof(pCheckPoint), m_mysqltool.mysql_getRowStringValue("CheckPoint"));  
            nScore = m_mysqltool.mysql_getRowIntValue("Score");
            AddSubCheckpointInfo(nLayoutLibID, pBeginTime, pEndTime, pCheckPoint, nScore);
        }
    }
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "获取布控卡口信息结束.");
    return true;

    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "获取布控人脸信息...");
    int nNum = 0;
    //获取布控人脸信息
    sprintf_s(pSQL, sizeof(pSQL),
        "select a.ID, a.BeginTime, a.EndTime, c.FaceUUID "
        "from %s a, %s c "
        "where a.id = c.LayoutLibId and c.controlstatus = 0 "
        "order by a.id",
        STORELIBTABLE, STOREFACEINFOTABLE);
    nRet = m_mysqltool.mysql_exec(_MYSQL_SELECT, pSQL);
    if (nRet > 0)
    {
        char pFaceUUID[64] = { 0 };
        char pBeginTime[20] = { 0 };
        char pEndTime[20] = { 0 };
        int nLayoutLibID = 0;
        while (m_mysqltool.mysql_getNextRow())
        {
            nLayoutLibID = m_mysqltool.mysql_getRowIntValue("id");
            strcpy_s(pBeginTime, sizeof(pBeginTime), m_mysqltool.mysql_getRowStringValue("BeginTime"));
            strcpy_s(pEndTime, sizeof(pEndTime), m_mysqltool.mysql_getRowStringValue("EndTime"));
            strcpy_s(pFaceUUID, sizeof(pFaceUUID), m_mysqltool.mysql_getRowStringValue("FaceUUID"));
            AddLayoutImage(nLayoutLibID, pBeginTime, pEndTime, pFaceUUID);
           
            nNum++;
            if (nNum % 50000 == 0)
            {
                g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "己获取布控人脸图片数量: %d.", nNum);
            }
        }
    }
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "获取布控人脸图片结束, 数量: %d.", nNum);
    return true;
}
//获取接口服务布控任务
bool CLayoutServer::GetLayoutTaskInfo()
{
    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "获取接口服务布控任务信息...");
    char pSQL[SQLMAXLEN] = { 0 };
    sprintf_s(pSQL, sizeof(pSQL),
        "select taskid, taskinfo, publishurl from layouttaskinfo",
        STORELIBTABLE, LAYOUTCHECKPOINTTABLE);
    int nRet = m_mysqltool.mysql_exec(_MYSQL_SELECT, pSQL);
    if (nRet > 0)
    {
        char pTaskID[MAXLEN] = { 0 };
        char * pTaskJsonInfo = new char[1024 * 1024];
        memset(pTaskJsonInfo, 0, 1024 * 1024);
        char pPublishURL[1024] = { 0 };
        while (m_mysqltool.mysql_getNextRow())
        {
            strcpy_s(pTaskID, sizeof(pTaskID), m_mysqltool.mysql_getRowStringValue("taskid"));
            strcpy_s(pTaskJsonInfo, 1024 * 1024, m_mysqltool.mysql_getRowStringValue("taskinfo"));
            strcpy_s(pPublishURL, sizeof(pPublishURL), m_mysqltool.mysql_getRowStringValue("publishurl"));


            rapidjson::Document document;
            document.Parse(pTaskJsonInfo);
            if (document.HasParseError())
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "解析Json串失败[%s]", pTaskJsonInfo);
                continue;
            }

            if (document.HasMember("publishurl")    && document["publishurl"].IsString()    &&
                document.HasMember("begintime")     && document["begintime"].IsInt64()      &&
                document.HasMember("endtime")       && document["endtime"].IsInt64()        &&
                document.HasMember("taskinfo")      && document["taskinfo"].IsArray())
            {
                LPLAYOUTTASKINFO pTaskInfo = new LAYOUTTASKINFO;
                strcpy(pTaskInfo->pTaskID, pTaskID);
                strcpy(pTaskInfo->pPublishURL, document["publishurl"].GetString());
                __int64 nBeginTime = document["begintime"].GetInt64();
                __int64 nEndTime = document["endtime"].GetInt64();
                if (nBeginTime > 4294967295 || nEndTime > 4294967295)
                {
                    printf("Time Format Wrong!\n");
                    continue;
                }
                printf("%d -- %d\n", (int)nBeginTime, (int)nEndTime);
                string sBeginTime = ChangeSecondToTime((int)nBeginTime);
                string sEndtime = ChangeSecondToTime((int)nEndTime);
                if ("" != sBeginTime && "" != sEndtime)
                {
                    strcpy(pTaskInfo->pBeginTime, sBeginTime.c_str());
                    strcpy(pTaskInfo->pEndTime, sEndtime.c_str());
                }
                else
                {
                    printf("Time Wrong[%s  %s]\n", sBeginTime.c_str(), sEndtime.c_str());
                    continue;
                }
                //保存布控任务对应布控库信息
                for (int i = 0; i < document["taskinfo"].Size(); i++)
                {
                    if (document["taskinfo"][i].HasMember("layoutlibid") && document["taskinfo"][i]["layoutlibid"].IsString() &&
                        document["taskinfo"][i].HasMember("checkpoint") && document["taskinfo"][i]["checkpoint"].IsArray() && document["taskinfo"][i]["checkpoint"].Size() > 0)
                    {
                        LPTASKCHECKPOINT pTaskCheckpoint = new TASKCHECKPOINT;
                        string sLibID = document["taskinfo"][i]["layoutlibid"].GetString();
                        pTaskCheckpoint->nLayoutLibID = atoi(sLibID.c_str());
                        //保存布控库对应卡口和阈值
                        for (int j = 0; j < document["taskinfo"][i]["checkpoint"].Size(); j++)
                        {
                            if (document["taskinfo"][i]["checkpoint"][j].HasMember("deviceid") && document["taskinfo"][i]["checkpoint"][j]["deviceid"].IsString() &&
                                document["taskinfo"][i]["checkpoint"][j].HasMember("score") && document["taskinfo"][i]["checkpoint"][j]["score"].IsInt())
                            {
                                pTaskCheckpoint->mapTaskCheckpoint.insert(make_pair(document["taskinfo"][i]["checkpoint"][j]["deviceid"].GetString(),
                                    document["taskinfo"][i]["checkpoint"][j]["score"].GetInt()));
                            }
                        }
                        pTaskInfo->mapTaskLib.insert(make_pair(pTaskCheckpoint->nLayoutLibID, pTaskCheckpoint));
                    }
                }

                m_mapLayoutTaskInfo.insert(make_pair(pTaskID, pTaskInfo));
                //将布控任务布控信息保存到本地布控卡口对应布控库map中
                map<int, LPTASKCHECKPOINT>::iterator itLib = pTaskInfo->mapTaskLib.begin();
                for (; itLib != pTaskInfo->mapTaskLib.end(); itLib++)
                {
                    map<string, int>::iterator itCheckpoint = itLib->second->mapTaskCheckpoint.begin();
                    for (; itCheckpoint != itLib->second->mapTaskCheckpoint.end(); itCheckpoint++)
                    {
                        AddSubCheckpointInfo(itLib->first, pTaskInfo->pBeginTime, pTaskInfo->pEndTime, (char*)itCheckpoint->first.c_str(), itCheckpoint->second, 1);
                    }
                }
            }
        }
    }

    return true;
}
bool CLayoutServer::InitHttpServer(string sHttpIP, int nHttpPort)
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
bool CLayoutServer::InitRedisManage(string sRedisIP, int nRedisPort)
{
    if (NULL == m_pRedisManage)
    {
        m_pRedisManage = new CRedisManage;
    }
    if (!m_pRedisManage->InitConnect(sRedisIP, nRedisPort))
    {
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "****Error: 连接Redis[%s:%d]失败!", 
            sRedisIP.c_str(), nRedisPort);
        return false;
    }
    return true;
}
bool CLayoutServer::InitAnalyseSearch()
{
    if (NULL == m_pAnalyseSearch)
    {
        m_pAnalyseSearch = new CAnalyseSearch;
    }
    if (!m_pAnalyseSearch->Init(m_pServerIP, m_nServerPort + 1, m_nServerPort + 2, AnalyseLayoutResultCallback, this))
    {
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "****Error: 初始化连接分析服务服务失败!");
        return false;
    }

    return true;
}
//布控结果回调
void CLayoutServer::AnalyseLayoutResultCallback(LPSUBMESSAGE pSubMessage, void * pUser)
{
    CLayoutServer * pThis = (CLayoutServer *)pUser;
    char pSQL[SQLMAXLEN] = { 0 };

    rapidjson::Document document;
    if (string(pSubMessage->pSubJsonValue) != "")
    {
        document.Parse(pSubMessage->pSubJsonValue);
        if (document.HasParseError())
        {
            printf("***Warning: Parse Json Format Failed[%s].\n", pSubMessage->pSubJsonValue);
            return;
        }

        if (document.HasMember(JSONFACEUUID)    && document[JSONFACEUUID].IsString()    && strlen(document[JSONFACEUUID].GetString()) < MAXLEN &&
            document.HasMember(JSONCHECKPOINT)  && document[JSONCHECKPOINT].IsString()  && strlen(document[JSONCHECKPOINT].GetString()) < MAXLEN &&
            document.HasMember(JSONTIME)        && document[JSONTIME].IsInt()           &&
            document.HasMember(JSONDRIVE)       && document[JSONDRIVE].IsString()       && strlen(document[JSONDRIVE].GetString()) == 1 &&
            document.HasMember(JSONSERVERIP)    && document[JSONSERVERIP].IsString()    && strlen(document[JSONSERVERIP].GetString()) < MAXIPLEN &&
            document.HasMember(JSONFACERECT) && document[JSONFACERECT].IsString() && strlen(document[JSONFACERECT].GetString()) < MAXIPLEN &&
            document.HasMember(JSONLAYOUTDATA)  && document[JSONLAYOUTDATA].IsArray()   && document[JSONLAYOUTDATA].Size() > 0)
        {
            //1. 保存FaceUUID
            strcpy(pSubMessage->pFaceUUID, document[JSONFACEUUID].GetString());
            //2. 保存卡口ID
            strcpy(pSubMessage->pDeviceID, document[JSONCHECKPOINT].GetString());
            //3. 保存抓拍图片时间
            pSubMessage->nImageTime = document[JSONTIME].GetInt();
            string sImageTime = pThis->ChangeSecondToTime(pSubMessage->nImageTime);
            //4. 保存磁盘信息
            strcpy(pSubMessage->pDisk, document[JSONDRIVE].GetString());
            //5. 保存图片服务器IP
            strcpy(pSubMessage->pImageIP, document[JSONSERVERIP].GetString());
            //6.  保存人脸坐标
            strcpy(pSubMessage->pFaceRect, document[JSONFACERECT].GetString());

            if (document.HasMember(JSONFACEURL) && document[JSONFACEURL].IsString() && strlen(document[JSONFACEURL].GetString()) < 2048 &&
                document.HasMember(JSONBKGURL) && document[JSONBKGURL].IsString() && strlen(document[JSONBKGURL].GetString()) < 2048)
            {
                strcpy(pSubMessage->pFaceURL, document[JSONFACEURL].GetString());
                strcpy(pSubMessage->pBkgURL, document[JSONBKGURL].GetString());
            }
            //7.  获取匹配布控图片信息
            for (int i = 0; i < document[JSONLAYOUTDATA].Size(); i++)
            {
                if (document[JSONLAYOUTDATA][i].HasMember(JSONLAYOUTFACEUUID)   && document[JSONLAYOUTDATA][i][JSONLAYOUTFACEUUID].IsString() &&
                    document[JSONLAYOUTDATA][i].HasMember(JSONSCORE)            && document[JSONLAYOUTDATA][i][JSONSCORE].IsInt()             &&
                    document[JSONLAYOUTDATA][i].HasMember(JSONLIBID)           && document[JSONLAYOUTDATA][i][JSONLIBID].IsString() )
                {
                    string sLayoutFaceUUID = document[JSONLAYOUTDATA][i][JSONLAYOUTFACEUUID].GetString();
                    int nScore = document[JSONLAYOUTDATA][i][JSONSCORE].GetInt();
                    int nLibID = atoi(document[JSONLAYOUTDATA][i][JSONLIBID].GetString());

                    //由于公安网图片kafka经常重复传递, 导致相同的布控报警过多, 插入报警信息前先查询下是否存在;
                    sprintf_s(pSQL, sizeof(pSQL),
                        "select * from layoutresult where LayoutFaceUUID = '%s' and FaceUUID = '%s'",
                        sLayoutFaceUUID.c_str(), pSubMessage->pFaceUUID);
                    EnterCriticalSection(&pThis->m_cs);
                    int nRet = pThis->m_mysqltool.mysql_exec(_MYSQL_SELECT, pSQL);
                    LeaveCriticalSection(&pThis->m_cs);
                    if (nRet > 0)
                    {
                        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "报警记录Layout[%s], FaceUUID[%s]己存在, 不重复插入!", sLayoutFaceUUID.c_str(), pSubMessage->pFaceUUID);
                        continue;
                    }

                    //在逃有效和在逃撤消分离
                    if (nLibID == 50)       //如果是在逃有效的报警, 则查下是否属于在逃撤消
                    {
                        sprintf_s(pSQL, sizeof(pSQL),
                            "select layoutlibid from storefaceinfo where faceuuid = '%s'",
                            sLayoutFaceUUID.c_str());
                        EnterCriticalSection(&pThis->m_cs);
                        int nRet = pThis->m_mysqltool.mysql_exec(_MYSQL_SELECT, pSQL);
                        LeaveCriticalSection(&pThis->m_cs);
                        if (nRet > 0)
                        {
                            while (pThis->m_mysqltool.mysql_getNextRow())
                            {
                                nLibID = pThis->m_mysqltool.mysql_getRowIntValue("layoutlibid");
                                if (50 != nLibID)
                                {
                                    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "在逃记录: 报警记录Layout[%s], 在逃撤消[%d]!", sLayoutFaceUUID.c_str(), nLibID);

                                }
                                break;
                            }
                        }
                    }
                    
                    //保存布控结果到数据库
                    if (strlen(pSubMessage->pFaceURL) > 0 && strlen(pSubMessage->pBkgURL) > 0)
                    {
                        sprintf_s(pSQL, sizeof(pSQL),
                            "insert into layoutResult ( LayoutFaceUUID, checkpoint, FaceUUID, imagedisk, imageip, facerect, Time, Score, LayoutLibID, face_url, bkg_url) "
                            "values('%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, '%s', '%s')",
                            sLayoutFaceUUID.c_str(), pSubMessage->pDeviceID, pSubMessage->pFaceUUID, pSubMessage->pDisk, pSubMessage->pImageIP,
                            pSubMessage->pFaceRect, sImageTime.c_str(), nScore, nLibID, pSubMessage->pFaceURL, pSubMessage->pBkgURL);
                    }
                    else
                    {
                        sprintf_s(pSQL, sizeof(pSQL),
                            "insert into layoutResult ( LayoutFaceUUID, checkpoint, FaceUUID, imagedisk, imageip, facerect, Time, Score, LayoutLibID) "
                            "values('%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, %d)",
                            sLayoutFaceUUID.c_str(), pSubMessage->pDeviceID, pSubMessage->pFaceUUID, pSubMessage->pDisk, pSubMessage->pImageIP,
                            pSubMessage->pFaceRect, sImageTime.c_str(), nScore, nLibID);
                    }
                    EnterCriticalSection(&pThis->m_cs);
                    nRet = pThis->m_mysqltool.mysql_exec(_MYSQL_INSERT, pSQL);
                    LeaveCriticalSection(&pThis->m_cs);
                    if (nRet < 0)
                    {
                        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 数据库增加布控比对信息到layoutresult表失败.");
                    }
                    else
                    {
                        sprintf_s(pSQL, sizeof(pSQL),
                            "update storefaceinfo set updatetime = '%s' where faceuuid = '%s'",
                            sImageTime.c_str(), sLayoutFaceUUID.c_str());
                        EnterCriticalSection(&pThis->m_cs);
                        pThis->m_mysqltool.mysql_exec(_MYSQL_INSERT, pSQL);
                        LeaveCriticalSection(&pThis->m_cs);

                        pThis->m_nAnalyseSearchNum ++;
                        g_LogRecorder.WriteDebugLog(__FUNCTION__, "----------------------------------------------------------------------------");
                        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "比对结果: 布控库[%d], LayoutFaceUUID[%s], 卡口[%s], FaceUUID[%s], Score[%d]",
                            nLibID, sLayoutFaceUUID.c_str(), pSubMessage->pDeviceID, pSubMessage->pFaceUUID, nScore);

                        //推送报警到Redis(给web页面实时显示预警)
                        char pPublish[64] = { 0 };
                        char pPublishMessage[128] = { 0 };
                        sprintf_s(pPublish, sizeof(pPublish), "Layout.%s", sLayoutFaceUUID.c_str());
                        sprintf_s(pPublishMessage, sizeof(pPublishMessage), "%s#%s#%s",
                            pSubMessage->pFaceUUID, pSubMessage->pDeviceID, sImageTime.c_str());
                        pThis->m_pRedisManage->PublishAlarmInfo(pPublish, pPublishMessage);



                        //插入报警信息到接口服务布控任务表
                        {
                            EnterCriticalSection(&pThis->m_csTask);
                            MAPLAYOUTTASKINFO::iterator itTask = pThis->m_mapLayoutTaskInfo.begin();
                            for (; itTask != pThis->m_mapLayoutTaskInfo.end(); itTask++)
                            {
                                LPLAYOUTTASKINFO pTaskInfo = itTask->second;
                                bool bFind = false;
                                map<int, LPTASKCHECKPOINT>::iterator itLib = pTaskInfo->mapTaskLib.begin();
                                for (; itLib != pTaskInfo->mapTaskLib.end(); itLib++)
                                {
                                    //报警重点库匹配
                                    if (nLibID == itLib->second->nLayoutLibID && sImageTime >= pTaskInfo->pBeginTime && sImageTime <= pTaskInfo->pEndTime)
                                    {
                                        map<string, int>::iterator itCheckpoint = itLib->second->mapTaskCheckpoint.begin();
                                        for (; itCheckpoint != itLib->second->mapTaskCheckpoint.end(); itCheckpoint++)
                                        {
                                            //报警卡口匹配且报警阈值大于比对分数
                                            if (itCheckpoint->first == pSubMessage->pDeviceID && itCheckpoint->second <= nScore)
                                            {
                                                //将结果写入接口服务布控任务结果表
                                                sprintf_s(pSQL, sizeof(pSQL),
                                                    "insert into layouttaskresult "
                                                    "(TaskID, LayoutLibID, Checkpoint, LayoutFaceUUID, FaceUUID, "
                                                    "FaceRect, Time, Score, face_url, bkg_url) "
                                                    "values('%s', %d, '%s', '%s', '%s', '%s', '%s', %d, '%s', '%s')",
                                                    pTaskInfo->pTaskID, nLibID, pSubMessage->pDeviceID, sLayoutFaceUUID.c_str(), pSubMessage->pFaceUUID,
                                                    pSubMessage->pFaceRect, sImageTime.c_str(), nScore, pSubMessage->pFaceURL, pSubMessage->pBkgURL);
                                                EnterCriticalSection(&pThis->m_cs);
                                                pThis->m_mysqltool.mysql_exec(_MYSQL_INSERT, pSQL);
                                                LeaveCriticalSection(&pThis->m_cs);

                                                g_LogRecorder.WriteDebugLog(__FUNCTION__, "----------------------------");
                                                g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "插入布控任务[%s]命中结果.", pTaskInfo->pTaskID);
                                                g_LogRecorder.WriteDebugLog(__FUNCTION__, "----------------------------");

                                                bFind = true;
                                                break;
                                            }
                                        }
                                        if (bFind)
                                        {
                                            break;
                                        }
                                    }
                                }
                                if (bFind)
                                {
                                    break;
                                }
                            }
                            LeaveCriticalSection(&pThis->m_csTask);
                        }

                        //推送报警到zmq代理服务(提供给专网Kafka服务, 推送到公安网显示)
                        {
                            strcpy(pSubMessage->pHead, "KAFKAALARM");
                            strcpy(pSubMessage->pOperationType, "AlarmInfo");
                            strcpy(pSubMessage->pSource, pThis->m_pConfigRead->m_sServerCode.c_str());

                            rapidjson::Document document;
                            document.SetObject();
                            rapidjson::Document::AllocatorType&allocator = document.GetAllocator();

                            document.AddMember("device_id", rapidjson::StringRef(pSubMessage->pDeviceID), allocator);
                            document.AddMember("faceuuid", rapidjson::StringRef(pSubMessage->pFaceUUID), allocator);
                            char pLibID[128] = { 0 };
                            itoa(nLibID, pLibID, 10);
                            document.AddMember("layoutlib_id", rapidjson::StringRef(pLibID), allocator);
                            document.AddMember("layoutfaceuuid", rapidjson::StringRef(sLayoutFaceUUID.c_str()), allocator);

                            char pTime[128] = { 0 };
                            sprintf_s(pTime, sizeof(pTime), "%d000", pSubMessage->nImageTime);
                            document.AddMember("face_time", rapidjson::StringRef(pTime), allocator);
                            char pScore[10];
                            itoa(nScore, pScore, 10);
                            document.AddMember("score", rapidjson::StringRef(pScore), allocator);

                            rapidjson::StringBuffer buffer;
                            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                            document.Accept(writer);
                            pSubMessage->sPubJsonValue = buffer.GetString();

                            pThis->m_pZeromqManage->PubMessage(pSubMessage);
                        }
                    }
                }
            }
        }
        else
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: Layout Result Json Format Wrong.\n[%s]", pSubMessage->pSubJsonValue);
        }
    }
    return; 

    
    GetLocalTime(&pThis->m_sysTime);
    printf("当前处理时间[%04d-%02d-%02d %02d:%02d:%02d]\n==================\n",
        pThis->m_sysTime.wYear, pThis->m_sysTime.wMonth,  pThis->m_sysTime.wDay, 
        pThis->m_sysTime.wHour, pThis->m_sysTime.wMinute, pThis->m_sysTime.wSecond);

    return ;
}
//初始化卡口图片订阅
bool CLayoutServer::InitCheckpointSubManage()
{
    if (NULL == m_pCheckSubManage)
    {
        m_pCheckSubManage = new CCheckpointSubManage;
        m_pCheckSubManage->Init(m_pProxyServerIP, m_nProxyPort, ImageSubCallback, this);
    }
    return true;
}
//卡口图片订阅回调的数据, 推送到分析服务搜索线程处理
void CLayoutServer::ImageSubCallback(LPSUBMESSAGE pSubMessage, void * pUser)
{
    CLayoutServer * pThis = (CLayoutServer *)pUser;
    pThis->m_nImageNum++;
    if (pThis->m_bPrintSnap)
    {
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "卡口[%s]回调图片.", pSubMessage->pHead);
    }

    strcpy(pSubMessage->pOperationType, COMMANDLAYOUTSEARCH);
    strcpy(pSubMessage->pSource, (char *)pThis->m_pConfigRead->m_sServerCode.c_str());

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType&allocator = document.GetAllocator();
    document.AddMember(JSONFACEUUID, rapidjson::StringRef(pSubMessage->pFaceUUID), allocator);
    document.AddMember(JSONFEATURE, rapidjson::StringRef(pSubMessage->pFeature), allocator);
    document.AddMember(JSONCHECKPOINT, rapidjson::StringRef(pSubMessage->pDeviceID), allocator);
    document.AddMember(JSONTIME, pSubMessage->nImageTime, allocator);
    document.AddMember(JSONDRIVE, rapidjson::StringRef(pSubMessage->pDisk), allocator);
    document.AddMember(JSONSERVERIP, rapidjson::StringRef(pSubMessage->pImageIP), allocator);
    document.AddMember(JSONFACERECT, rapidjson::StringRef(pSubMessage->pFaceRect), allocator);
    if (strlen(pSubMessage->pFaceURL) > 0 && strlen(pSubMessage->pBkgURL) > 0)
    {
        document.AddMember(JSONFACEURL, rapidjson::StringRef(pSubMessage->pFaceURL), allocator);
        document.AddMember(JSONBKGURL, rapidjson::StringRef(pSubMessage->pBkgURL), allocator);
    }
    

    //搜索订阅回调卡口是否存在
    MAPLAYOUTCHECKPOINTINFO::iterator it = pThis->m_mapLayoutCheckpointInfo.find(pSubMessage->pDeviceID);
    if (pThis->m_mapLayoutCheckpointInfo.end() != it)
    {
        char pLibName[MAXLEN] = { 0 };
        //遍历此卡口下面的所有布控库
        for (MAPLAYOUTLIBINFO::iterator itLib = it->second->mapLayoutLibInfo.begin(); itLib != it->second->mapLayoutLibInfo.end(); itLib++)
        {
            //判断抓拍时间是否在布控时间范围内
            if (itLib->second->nBeginTime <= pSubMessage->nImageTime && itLib->second->nEndTime >= pSubMessage->nImageTime)
            {
                sprintf_s(pLibName, sizeof(pLibName), "%dkeylib", itLib->first);
                strcpy(pSubMessage->pHead, pLibName);
                document.RemoveMember(JSONSCORE);
                document.AddMember(JSONSCORE, itLib->second->nScore, allocator);

                //搜索布控库是否有特征值结点指针信息保存
                EnterCriticalSection(&pThis->m_csFeature);
                MAPANALYSEFEATURELIBINFO::iterator itFeature = pThis->m_mapAnalyseFeatureLibInfo.find(pLibName);
                if (itFeature != pThis->m_mapAnalyseFeatureLibInfo.end())
                {
                    //遍历此布控库所有特征值结点指针发送搜索任务
                    for (set<int>::iterator itIndex = itFeature->second->setFeatureInfo.begin();
                            itIndex != itFeature->second->setFeatureInfo.end(); itIndex++)
                    {
                        document.RemoveMember(JSONLIBINDEX);
                        document.AddMember(JSONLIBINDEX, *itIndex, allocator);

                        rapidjson::StringBuffer buffer;
                        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                        document.Accept(writer);
                        pSubMessage->sPubJsonValue = buffer.GetString();
                        pThis->m_pAnalyseSearch->PushLayoutSearchMessage(pSubMessage);
                        if (pThis->m_bPrintSnap)
                        {
                            g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "发布搜索任务[%s][%s][%s].",
                                pSubMessage->pHead, pSubMessage->pDeviceID, pSubMessage->pFaceUUID);
                        }
                        
                    }
                }
                LeaveCriticalSection(&pThis->m_csFeature);
            }
        }
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 卡口[%s]回调图片找不到订阅卡口信息.", pSubMessage->pHead);
    }

    return;
}
//增加|删除卡口订阅命令线程
DWORD WINAPI CLayoutServer::SubCheckpointThread(LPVOID lParam)
{
    CLayoutServer * pThis = (CLayoutServer*)lParam;
    pThis->SubCheckpointAction();
    return 0;
}
void CLayoutServer::SubCheckpointAction()
{
    do
    {
        EnterCriticalSection(&m_cs);
        time_t tCurrentTime = time(NULL);
        for (MAPLAYOUTCHECKPOINTINFO::iterator it = m_mapLayoutCheckpointInfo.begin(); it != m_mapLayoutCheckpointInfo.end(); it++)
        {
            bool bLayout = false;
            for (MAPLAYOUTLIBINFO::iterator itLib = it->second->mapLayoutLibInfo.begin(); itLib != it->second->mapLayoutLibInfo.end(); itLib++)
            {
                if (itLib->second->nBeginTime <= tCurrentTime && itLib->second->nEndTime > tCurrentTime)
                {
                    bLayout = true;
                    break;
                }
            }

            if (!bLayout && it->second->bLayoutStatus)       //己布控且己不需要布控, 则停止布控 
            {
                it->second->bLayoutStatus = false;
                m_pCheckSubManage->DelSubCheckpoint((char*)it->first.c_str());
            }
            else if(bLayout && !it->second->bLayoutStatus)  //尚未布控且需要布控, 则开始布控
            {
                it->second->bLayoutStatus = true;
                m_pCheckSubManage->AddSubCheckpoint((char*)it->first.c_str());
            }
        }
        LeaveCriticalSection(&m_cs);
    } while (WAIT_TIMEOUT == WaitForSingleObject(m_hStopEvent, THREADWAITTIME));
}
bool CLayoutServer::InitZeroMq()
{
    if (NULL == m_pZeromqManage)
    {
        m_pZeromqManage = new CZeromqManage;
    }
    if (!m_pZeromqManage->InitSub(NULL, 0, m_pProxyServerIP, m_nProxyPort, AnalyseLibInfoCallback, this, 1))    //订阅分析布控服务库信息
    {
        printf("****Error: InitSub[%s:%d]失败!", m_pProxyServerIP, m_nProxyPort);
        return false;
    }
    if (!m_pZeromqManage->InitPub(NULL, 0, m_pProxyServerIP, m_nProxyPort + 1))
    {
        printf("****Error: InitPub[%s:%d]失败!", m_pProxyServerIP, m_nProxyPort + 1);
        return false;
    }
    m_pZeromqManage->AddSubMessage(LAYOUTLIBADDRESSINFO);
    return true;
}
//分析服务特征值结点指针信息保存
void CLayoutServer::AnalyseLibInfoCallback(LPSUBMESSAGE pSubMessage, void * pUser)
{
    CLayoutServer * pThis = (CLayoutServer *)pUser;

    LPANALYSEFEATURELIBINFO pAnalyseFeatureLibInfo = NULL;
    EnterCriticalSection(&pThis->m_csFeature);
    MAPANALYSEFEATURELIBINFO::iterator it = pThis->m_mapAnalyseFeatureLibInfo.find(pSubMessage->pSource);
    if (pThis->m_mapAnalyseFeatureLibInfo.end() != it)
    {
        pAnalyseFeatureLibInfo = it->second;
    }
    else
    {
        pAnalyseFeatureLibInfo = new ANALYSEFEATURELIBINFO;
        pThis->m_mapAnalyseFeatureLibInfo.insert(make_pair(pSubMessage->pSource, pAnalyseFeatureLibInfo));
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "Recv Lib[%s] Feature Address Info...", pSubMessage->pSource);
    }
    rapidjson::Document document;
    document.Parse(pSubMessage->pSubJsonValue);
    if (document.HasParseError())
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 解析Json串失败[%s]", pSubMessage->pSubJsonValue);
    }
    else
    {
        if (document.HasMember(JSONLIBINFO) && document[JSONLIBINFO].IsArray() && document[JSONLIBINFO].Size() > 0)
        {
            pAnalyseFeatureLibInfo->setFeatureInfo.clear();
            for (int i = 0; i < document[JSONLIBINFO].Size(); i++)
            {
                if (document[JSONLIBINFO][i].HasMember(JSONLIBINDEX) && document[JSONLIBINFO][i][JSONLIBINDEX].IsInt())
                {
                    int nIndex = document[JSONLIBINFO][i][JSONLIBINDEX].GetInt();
                    set<int>::iterator it = pAnalyseFeatureLibInfo->setFeatureInfo.find(nIndex);
                    if (it == pAnalyseFeatureLibInfo->setFeatureInfo.end())
                    {
                        pAnalyseFeatureLibInfo->setFeatureInfo.insert(nIndex);
                    }
                }
            }
        }
    }
    LeaveCriticalSection(&pThis->m_csFeature);

    return;
}
//增加布控卡口
bool CLayoutServer::AddSubCheckpointInfo(int nLibID, char * pBeginTime, char * pEndtime, char * pCheckpointID, int nScore, int nTaskType)
{
    nScore = nScore < 50 ? 50 : nScore;
    EnterCriticalSection(&m_cs);
    LPLAYOUTCHECKPOINTINFO pLayoutCheckpointInfo = NULL;
    MAPLAYOUTCHECKPOINTINFO::iterator it = m_mapLayoutCheckpointInfo.find(pCheckpointID);
    if (it != m_mapLayoutCheckpointInfo.end())
    {
        pLayoutCheckpointInfo = it->second;
    }
    else
    {
        pLayoutCheckpointInfo = new LAYOUTCHECKPOINTINFO;
        m_mapLayoutCheckpointInfo.insert(make_pair(pCheckpointID, pLayoutCheckpointInfo));
    }

    MAPLAYOUTLIBINFO::iterator itLib = pLayoutCheckpointInfo->mapLayoutLibInfo.find(nLibID);
    if (itLib != pLayoutCheckpointInfo->mapLayoutLibInfo.end())
    {
        int nBeginTime = ChangeTimeToSecond(pBeginTime);
        int nEndTime = ChangeTimeToSecond(pEndtime);
        itLib->second->nBeginTime = nBeginTime < itLib->second->nBeginTime ? nBeginTime : itLib->second->nBeginTime;
        itLib->second->nEndTime = nEndTime > itLib->second->nEndTime ? nEndTime : itLib->second->nEndTime;
        if (0 == nTaskType)
        {
            itLib->second->nSystemScore = nScore;
        }
        else if(1 == nTaskType)
        {
            itLib->second->nTaskScore = nScore < itLib->second->nTaskScore ? nScore : itLib->second->nTaskScore;
        }
        else if (2 == nTaskType)
        {
            itLib->second->nTaskScore = nScore;
        }
        itLib->second->nScore = itLib->second->nSystemScore < itLib->second->nTaskScore ? itLib->second->nSystemScore : itLib->second->nTaskScore;
    }
    else
    {
        LPLAYOUTLIBINFO pLayoutLibInfo = new LAYOUTLIBINFO;
        pLayoutLibInfo->nBeginTime = ChangeTimeToSecond(pBeginTime);
        pLayoutLibInfo->nEndTime = ChangeTimeToSecond(pEndtime);
        if (0 == nTaskType)
        {
            pLayoutLibInfo->nSystemScore = nScore;
        }
        else if (1 == nTaskType)
        {
            pLayoutLibInfo->nTaskScore = nScore;
        }
        else if (2 == nTaskType)
        {
            pLayoutLibInfo->nTaskScore = nScore;
        }
        pLayoutLibInfo->nScore = nScore;
        pLayoutCheckpointInfo->mapLayoutLibInfo.insert(make_pair(nLibID, pLayoutLibInfo));
    }
    LeaveCriticalSection(&m_cs);
    return true;
}
//删除布控卡口
bool CLayoutServer::DelLayoutCheckpoint(int nLibID, char * pCheckpointID)
{
    EnterCriticalSection(&m_cs);
    MAPLAYOUTCHECKPOINTINFO::iterator it = m_mapLayoutCheckpointInfo.find(pCheckpointID);
    if (it != m_mapLayoutCheckpointInfo.end())
    {
        MAPLAYOUTLIBINFO::iterator itLib = it->second->mapLayoutLibInfo.find(nLibID);
        if (itLib != it->second->mapLayoutLibInfo.end())
        {
            it->second->mapLayoutLibInfo.erase(itLib);
            g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "从布控库[%d]删除布控卡口[%s]成功.", nLibID, pCheckpointID);
        }
        else
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 布控库[%d]未布控卡口[%s].", nLibID, pCheckpointID);
        }
    }
    else
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 卡口[%s]未布控.", pCheckpointID);
    }

    LeaveCriticalSection(&m_cs);
    return true;
}
//增加布控人脸
bool CLayoutServer::AddLayoutImage(int nLibID, char * pBeginTime, char * pEndtime, char * pFaceUUID)
{
    m_mapLayoutFaceUUIDInfo.insert(make_pair(pFaceUUID, nLibID));
    return true;
}
//删除布控人脸
bool CLayoutServer::DelLayoutImage(int nLibID, char * pFaceUUID)
{
    return true;
}
bool CLayoutServer::StopLayoutLib(int nLibID)
{
    EnterCriticalSection(&m_cs);
    MAPLAYOUTCHECKPOINTINFO::iterator it = m_mapLayoutCheckpointInfo.begin();
    for (; it != m_mapLayoutCheckpointInfo.end(); it++)
    {
        MAPLAYOUTLIBINFO::iterator itLib = it->second->mapLayoutLibInfo.find(nLibID);
        if (itLib != it->second->mapLayoutLibInfo.end())
        {
            it->second->mapLayoutLibInfo.erase(itLib);
        }
    }
    LeaveCriticalSection(&m_cs);

    /*EnterCriticalSection(&m_csFeature);
    char pLibName[MAXLEN] = { 0 };
    sprintf_s(pLibName, sizeof(pLibName), "%dkeylib", nLibID);
    MAPANALYSEFEATURELIBINFO::iterator itFeature = m_mapAnalyseFeatureLibInfo.find(pLibName);
    if (itFeature != m_mapAnalyseFeatureLibInfo.end())
    {
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "删除库[%d]特征值结点信息.", nLibID);
        delete itFeature->second;
        m_mapAnalyseFeatureLibInfo.erase(itFeature);
    }
    LeaveCriticalSection(&m_csFeature);*/
    return true;
}
//增加|修改布控任务
int CLayoutServer::AddLayoutTaskInfo(LPLAYOUTTASKINFO pTaskInfo)
{
    int nRet = 0;
    int nType = 1;
    EnterCriticalSection(&m_csTask);
    if (strlen(pTaskInfo->pTaskID) != 0)    //修改布控任务
    {
        MAPLAYOUTTASKINFO::iterator it = m_mapLayoutTaskInfo.find(pTaskInfo->pTaskID);
        if (it != m_mapLayoutTaskInfo.end())
        {
            //找到任务ID, 替换对应任务
            delete it->second;
            it->second = pTaskInfo;

            nType = 2;
        }
        else
        {
            nRet = TaskIDNotExist;  //任务ID未找到, 无法修改对应任务
        }
    }
    else  //增加布控任务
    {
        string sTaskID = GetUUID();
        strcpy(pTaskInfo->pTaskID, sTaskID.c_str());
        m_mapLayoutTaskInfo.insert(make_pair(pTaskInfo->pTaskID, pTaskInfo));
    }

    //将布控任务布控信息保存到本地布控卡口对应布控库map中
    if (0 == nRet)
    {
        map<int, LPTASKCHECKPOINT>::iterator itLib = pTaskInfo->mapTaskLib.begin();
        for (; itLib != pTaskInfo->mapTaskLib.end(); itLib++)
        {
            map<string, int>::iterator itCheckpoint = itLib->second->mapTaskCheckpoint.begin();
            for (; itCheckpoint != itLib->second->mapTaskCheckpoint.end(); itCheckpoint++)
            {
                AddSubCheckpointInfo(itLib->first, pTaskInfo->pBeginTime, pTaskInfo->pEndTime, (char*)itCheckpoint->first.c_str(), itCheckpoint->second, nType);
            }
        }
    }


    LeaveCriticalSection(&m_csTask);
    return nRet;
}
//删除布控任务
int CLayoutServer::StopLayoutTask(string sTaskID)
{
    int nRet = 0;
    EnterCriticalSection(&m_csTask);
    MAPLAYOUTTASKINFO::iterator it = m_mapLayoutTaskInfo.find(sTaskID);
    if (it != m_mapLayoutTaskInfo.end())
    {
        delete it->second;
        m_mapLayoutTaskInfo.erase(it);
    }
    else
    {
        nRet = TaskIDNotExist;
    }
    LeaveCriticalSection(&m_csTask);
    return nRet;
}
bool CLayoutServer::UpdateTaskInfoToDB(string sTaskID, bool bAdd, string sTaskInfo, string sPublishURL)
{
    char * pSQL = new char[1024 * 1024];
    ZeroMemory(pSQL, 1024 * 1024);
    if (bAdd)
    {
        if (sTaskInfo.find("layouttaskid") == string::npos)    //add
        {
            sprintf_s(pSQL, 1024 * 1024,
                "insert into layouttaskinfo ( TaskID, TaskInfo, PublishURL) "
                "values('%s', '%s', '%s')",
                sTaskID.c_str(), sTaskInfo.c_str(), sPublishURL.c_str());
        }
        else     //update
        {
            sprintf_s(pSQL, 1024 * 1024,
                "update layouttaskinfo set TaskInfo = '%s' where TaskID = '%s'",
                sTaskInfo.c_str(), sTaskID.c_str());
        }
    }
    else         //delete
    {
        sprintf_s(pSQL, 1024 * 1024, "delete from layouttaskinfo where taskid = '%s'", sTaskID.c_str());
    }
    EnterCriticalSection(&m_cs);
    m_mysqltool.mysql_exec(_MYSQL_INSERT, pSQL);
    LeaveCriticalSection(&m_cs);
    return true;
}

//HTTP服务信息回调
void CALLBACK CLayoutServer::HttpClientRequestCallback(LPHTTPREQUEST pHttpRequest, void * pUser)
{
    CLayoutServer * pThis = (CLayoutServer *)pUser;
    EnterCriticalSection(&pThis->m_csHttp);
    pThis->m_listHttpRequest.push_back(pHttpRequest);
    LeaveCriticalSection(&pThis->m_csHttp);
}
DWORD WINAPI CLayoutServer::HTTPRequestParseThread(LPVOID lParam)
{
    CLayoutServer * pThis = (CLayoutServer*)lParam;
    pThis->HTTPRequestParseAction();
    return 0;
}
//http请求信息处理线程
void CLayoutServer::HTTPRequestParseAction()
{
    LPHTTPREQUEST pHttpRequest = NULL;
    int nTaskNum = 0;
    int nRet = 0;
    while (WAIT_TIMEOUT == WaitForSingleObject(m_hStopEvent, THREADWAITTIME))
    {
        do
        {
            EnterCriticalSection(&m_csHttp);
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
            LeaveCriticalSection(&m_csHttp);

            if(NULL != pHttpRequest)
            {
                nRet = 0;
                switch (pHttpRequest->nOperatorType)
                {
                case 1:     //增加布控人脸图片
                {
                    break;
                }
                case 2:     //删除布控人脸图片
                {
                    break;
                }   
                case 3:     //增加布控卡口
                {
                    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "新增布控库: %d", pHttpRequest->pLayoutInfo->nLayoutLibID);
                    MAPLAYOUTCHECKPOINT::iterator it = pHttpRequest->pLayoutInfo->mapCheckpoint.begin();
                    for (; it != pHttpRequest->pLayoutInfo->mapCheckpoint.end(); it++)
                    {
                        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "\t新增布控卡口[%s][%d]", it->first.c_str(), it->second);
                        AddSubCheckpointInfo(pHttpRequest->pLayoutInfo->nLayoutLibID, pHttpRequest->pLayoutInfo->pBeginTime,
                            pHttpRequest->pLayoutInfo->pEndTime, (char*)it->first.c_str(), it->second);
                    }
                    
                    break;
                }
                case 4:     //删除布控卡口
                {
                    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "删除布控库[%d]卡口: ", pHttpRequest->pLayoutInfo->nLayoutLibID);
                    MAPLAYOUTCHECKPOINT::iterator it = pHttpRequest->pLayoutInfo->mapCheckpoint.begin();
                    for (; it != pHttpRequest->pLayoutInfo->mapCheckpoint.end(); it++)
                    {
                        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "\t删除布控卡口[%s]", it->first.c_str());
                        DelLayoutCheckpoint(pHttpRequest->pLayoutInfo->nLayoutLibID, (char*)it->first.c_str());
                    }
                    break;
                }
                case 5:     //删除布控库
                {
                    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "删除布控库: %d", pHttpRequest->pLayoutInfo->nLayoutLibID);
                    StopLayoutLib(pHttpRequest->pLayoutInfo->nLayoutLibID);
                    break;
                }
                case 6: //增加|修改布控任务
                {
                    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "增加|修改布控任务[%s].", pHttpRequest->pTaskInfo->pTaskID);
                    nRet = AddLayoutTaskInfo(pHttpRequest->pTaskInfo);
                    pHttpRequest->sTaskID = pHttpRequest->pTaskInfo->pTaskID;
                    if (0 == nRet)  //返回成功, 说明pHttpRequest->pTaskInfo被复用, 不用delete
                    {
                        string sURL = pHttpRequest->pTaskInfo->pPublishURL;
                        pHttpRequest->pTaskInfo = NULL;
                        UpdateTaskInfoToDB(pHttpRequest->sTaskID, true, pHttpRequest->sHttpBody, sURL);
                    }
                    break;
                }
                case 7: //删除布控任务
                {
                    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "删除布控任务[%s].", pHttpRequest->sTaskID.c_str());
                    nRet = StopLayoutTask(pHttpRequest->sTaskID);
                    if (0 == nRet)
                    {
                        UpdateTaskInfoToDB(pHttpRequest->sTaskID, false);
                    }
                    break;
                }
                default:
                    break;
                }

                SendResponseMsg(pHttpRequest->ClientSocket, nRet, pHttpRequest->sTaskID);

                delete pHttpRequest;
                pHttpRequest = NULL;
            }
        }while(nTaskNum > 0);
    }
    return;
}
void CLayoutServer::SendResponseMsg(SOCKET ClientSocket, int nEvent, string sTaskID)
{
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType&allocator = document.GetAllocator();
    if(nEvent == 0)
    {
        document.AddMember("Result", rapidjson::StringRef("success"), allocator);
        if ("" != sTaskID)
        {
            document.AddMember("taskid", rapidjson::StringRef(sTaskID.c_str()), allocator);
        }
    }
    else
    {
        char pMsg[128] = {0};
        GetErrorMsg(nEvent, pMsg);
        document.AddMember("Result", rapidjson::StringRef("error"), allocator);
        document.AddMember("ErrorMessage", rapidjson::StringRef(pMsg), allocator);
        if ("" != sTaskID)
        {
            document.AddMember("taskid", rapidjson::StringRef(sTaskID.c_str()), allocator);
        }
    }
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    string sBody = buffer.GetString();
    //向客户端发送回应消息
    m_pHttpServer->ResponseBody(ClientSocket, "", sBody);
    printf("Resp:\n%s\n", sBody.c_str());

    return;
}
unsigned long long CLayoutServer::ChangeTimeToSecond(string sTime)
{
    tm tTime;
    try
    {
        sscanf_s(sTime.c_str(), "%d-%d-%d %d:%d:%d", &(tTime.tm_year), &(tTime.tm_mon), &(tTime.tm_mday),
            &(tTime.tm_hour), &(tTime.tm_min), &(tTime.tm_sec));
    }
    catch (...)
    {
        return -3;
    }

    tTime.tm_year -= 1900;
    tTime.tm_mon -= 1;
    unsigned long long nBeginTime = (unsigned long long)mktime(&tTime);
    if (nBeginTime == -1)
    {
        return -3;
    }

    return nBeginTime;
}
string CLayoutServer::ChangeSecondToTime(unsigned long long nSecond)
{
    time_t ctime = nSecond;
    tm *tTime = localtime(&ctime);
    char sTime[20];
    sprintf_s(sTime, 20, "%04d-%02d-%02d %02d:%02d:%02d", tTime->tm_year + 1900, tTime->tm_mon + 1, tTime->tm_mday,
        tTime->tm_hour, tTime->tm_min, tTime->tm_sec);
    string RetTime = sTime;
    return sTime;
}
int CLayoutServer::GetErrorMsg(int nError, char * pMsg)
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
    case TaskIDNotExist:
        strcpy(pMsg, "TASKID_NOT_EXIST");
        break;
    default:
        nRet = -1;
        break;
    }

    return nRet;
}
string CLayoutServer::GetUUID()
{
    char buffer[64] = { 0 };
    GUID guid;
    if (CoCreateGuid(&guid))
    {
        //fprintf(stderr, "create guid error\n");  
    }
    _snprintf(buffer, sizeof(buffer),
        "%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2],
        guid.Data4[3], guid.Data4[4], guid.Data4[5],
        guid.Data4[6], guid.Data4[7]);
    return buffer;
}
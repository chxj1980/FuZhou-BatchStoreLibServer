#include "StdAfx.h"
#include "STSDKClient.h"

extern CLogRecorder g_LogRecorder;
unsigned int CSTSDKClient::m_nBoundaryLow = 0;
unsigned int CSTSDKClient::m_nBoundaryHigh = 0;
CSTSDKClient::CSTSDKClient(void)
{
    ZeroMemory(m_pSTIP, sizeof(m_pSTIP));
    ZeroMemory(m_pSTAddr, sizeof(m_pSTAddr));
    m_nSTPort = 0;

    if(m_nBoundaryLow == 0)
    {
        srand((unsigned int)time(NULL));
        m_nBoundaryLow = (rand() * rand()) % 40000 + 1000000;
        m_nBoundaryHigh = (rand() * rand()) % 40000 + 5000000;
    }
    

    m_pSocketManage = NULL;
    m_pHttpMsg = new char[HTTPSENDMSGSIZE];
    m_pHttpBody = new char[HTTPSENDMSGSIZE];
}

CSTSDKClient::~CSTSDKClient(void)
{
    if (NULL != m_pSocketManage)
    {
        delete m_pSocketManage;
        m_pSocketManage = NULL;
    }
    if(NULL == m_pHttpMsg)
    {
        delete m_pHttpMsg;
        m_pHttpMsg = NULL;
    }
    if(NULL == m_pHttpBody)
    {
        delete m_pHttpBody;
        m_pHttpBody = NULL;
    }
}
int CSTSDKClient::Init(STSERVERINFO STServerInfo[], int nQuantity)
{
    strcpy(m_pSTIP, STServerInfo[0].pSTServerIP);
    m_nSTPort = STServerInfo[0].nSTServerPort;

    sprintf_s(m_pSTAddr, sizeof(m_pSTAddr), "%s:%d", m_pSTIP, m_nSTPort);
    m_pSocketManage = new CSocketManage;
    if (!m_pSocketManage->Init(STServerInfo, nQuantity))
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, 
            "****Error: Socket[%s:%d]初始化失败!", m_pSTIP, m_nSTPort);
        return STSDK_SOCKETINITFAILED;
    }

    return 0;
}
int CSTSDKClient::UnInit()
{
    if (NULL != m_pSocketManage)
    {
        m_pSocketManage->Uninit();
    }
    return 0;
}
int CSTSDKClient::OperationDB(const char * pDBName, int nOperationType, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    
    char pURL[1024] = { 0 };
    OPERATORFUN nFunType = OPERATORINIT;
    switch (nOperationType)
    {
    case 1:
        sprintf_s(pURL, sizeof(pURL), "/verify/target/add");
        nFunType = ADDDB;
        break;
    case 2:
        sprintf_s(pURL, sizeof(pURL), "/verify/target/deletes");
        nFunType = DELDB;
        break;
    case 3:
        sprintf_s(pURL, sizeof(pURL), "/verify/target/clear");
        nFunType = CLEARDB;
        break;
    default:
        return STSDK_OPERATIONNOTFOUND;
    }
    HttpProtocol HttpInfo; 
    HttpInfo.SetAddr(m_pSTAddr);
    HttpInfo.SetURL(pURL);
    HttpInfo.setRequestMethod(HL_HTTP_POST);
    HttpInfo.setRequestProperty("X-Requested-With", "XMLHttpRequest");
    HttpInfo.setRequestProperty("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
    HttpInfo.setRequestProperty("Accept", "text/plain, */*; q=0.01");
    HttpInfo.setRequestProperty("Referer", "XSCollectionServer");
    HttpInfo.setRequestProperty("Accept-Language", "en-US,en;q=0.8,zh-Hans-CN;q=0.5,zh-Hans;q=0.3");
    HttpInfo.setRequestProperty("Accept-Encoding", "gzip, deflate");
    HttpInfo.setRequestProperty("User-Agent", "XSCollectionServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("DNT", "1");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");
    HttpInfo.setRequestProperty("Cache-Control", "no-cache");

    m_nHttpBodyLen = 0;

    sprintf_s(m_pHttpBody, HTTPSENDMSGSIZE, "dbName=%s", pDBName);
    m_nHttpBodyLen += strlen(m_pHttpBody);

    char pBodyLen[12] = {0};
    sprintf_s(pBodyLen, sizeof(pBodyLen), "%d", m_nHttpBodyLen);
    HttpInfo.setRequestProperty("Content-Length", pBodyLen);
    
    string sHttp = HttpInfo.GetHttpHead();

    m_nHttpMsgLen = 0;
    if(sHttp.size() + m_nHttpBodyLen > HTTPSENDMSGSIZE)
    {
        delete []m_pHttpMsg;
        m_pHttpMsg = new char[sHttp.size() + m_nHttpBodyLen];
    }
    strcpy_s(m_pHttpMsg, HTTPSENDMSGSIZE, sHttp.c_str());
    m_nHttpMsgLen = sHttp.size();
    
    memcpy(m_pHttpMsg + m_nHttpMsgLen, m_pHttpBody, m_nHttpBodyLen);
    m_nHttpMsgLen += m_nHttpBodyLen;

    bool bRet = m_pSocketManage->SendMsg(m_pHttpMsg, m_nHttpMsgLen, nType, nFunType);
    if (!bRet)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 发送消息失败!\r\n%s", sHttp.c_str());
        nRet = STSDK_SENDMSGFAILED;
    }
    else
    {
        nRet = m_pSocketManage->RecvMsg(pResponseMsg, (unsigned int*)nMsgLen);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 接收HTTP回应消息失败");
        }
    }
    
    return nRet;
}
int CSTSDKClient::GetAllDBInfo(char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    HttpProtocol HttpInfo;
    HttpInfo.SetAddr(m_pSTAddr);
    HttpInfo.SetURL(string("/verify/target/gets"));
    HttpInfo.setRequestMethod(HL_HTTP_GET);
    HttpInfo.setRequestProperty("X-Requested-With", "XMLHttpRequest");
    HttpInfo.setRequestProperty("Accept", "text/plain, */*; q=0.01");
    HttpInfo.setRequestProperty("Referer", "XSCollectionServer");
    HttpInfo.setRequestProperty("Accept-Language", "en-US,en;q=0.8,zh-Hans-CN;q=0.5,zh-Hans;q=0.3");
    HttpInfo.setRequestProperty("Accept-Encoding", "gzip, deflate");                
    HttpInfo.setRequestProperty("User-Agent", "XSCollectionServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("DNT", "1");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");

    string sHttp = HttpInfo.GetHttpHead();

    m_nHttpMsgLen = 0;
    strcpy_s(m_pHttpMsg, HTTPSENDMSGSIZE, sHttp.c_str());
    m_nHttpMsgLen = sHttp.size();

    bool bRet = m_pSocketManage->SendMsg(m_pHttpMsg, m_nHttpMsgLen, nType, GETALLDBINFO);
    if (!bRet)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 发送消息失败!\r\n%s", sHttp.c_str());
        nRet = STSDK_SENDMSGFAILED;
    }
    else
    {
        nRet = m_pSocketManage->RecvMsg(pResponseMsg, (unsigned int*)nMsgLen);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 接收HTTP回应消息失败");
        }
    }

    return nRet;
}
int CSTSDKClient::FaceQuality(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    HttpProtocol HttpInfo; 
    HttpInfo.SetAddr(m_pSTAddr);
    HttpInfo.SetURL(string("/verify/face/detectAndQuality"));
    HttpInfo.setRequestMethod(HL_HTTP_POST);
    HttpInfo.setRequestProperty("Accept", "image/jpeg, application/x-ms-application, image/gif, application/xaml+xml, image/pjpeg, application/x-ms-xbap, */*");
    HttpInfo.setRequestProperty("Referer", "XSCollectionServer");
    HttpInfo.setRequestProperty("Accept-Language", "en-US,en;q=0.8,zh-Hans-CN;q=0.5,zh-Hans;q=0.3");
    HttpInfo.setRequestProperty("Accept-Encoding", "gzip, deflate");
    char pContentType[128] = {0};
    string sBoundary = GetBoundary();
    sprintf_s(pContentType, sizeof(pContentType), "multipart/form-data; boundary=---------------------------%s", sBoundary.c_str());
    HttpInfo.setRequestProperty("Content-Type", pContentType);
    HttpInfo.setRequestProperty("User-Agent", "XSCollectionServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("DNT", "1");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");
    HttpInfo.setRequestProperty("Cache-Control", "no-cache");

    ZeroMemory(m_pHttpBody, HTTPSENDMSGSIZE);
    m_nHttpBodyLen = 0;

    sprintf_s(m_pHttpBody, HTTPSENDMSGSIZE,
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"imageData\"; filename=\"%s.jpg\"\r\n"
        "Content-Type: image/pjpeg\r\n\r\n", 
        sBoundary.c_str(), sBoundary.c_str());
    m_nHttpBodyLen += strlen(m_pHttpBody);

    memcpy(m_pHttpBody + m_nHttpBodyLen, pImage, nLen);
    m_nHttpBodyLen += nLen;

    char pTail[128] = {0};
    sprintf_s(pTail, sizeof(pTail), 
        "\r\n-----------------------------%s--\r\n", 
        sBoundary.c_str());
    memcpy(m_pHttpBody + m_nHttpBodyLen, pTail, strlen(pTail));
    m_nHttpBodyLen += strlen(pTail);

    char pBodyLen[12] = {0};
    sprintf_s(pBodyLen, sizeof(pBodyLen), "%d", m_nHttpBodyLen);
    HttpInfo.setRequestProperty("Content-Length", pBodyLen);
    
    string sHttp = HttpInfo.GetHttpHead();

    m_nHttpMsgLen = 0;
    if(sHttp.size() + m_nHttpBodyLen > HTTPSENDMSGSIZE)
    {
        delete []m_pHttpMsg;
        m_pHttpMsg = new char[sHttp.size() + m_nHttpBodyLen];
    }
    strcpy_s(m_pHttpMsg, HTTPSENDMSGSIZE, sHttp.c_str());
    m_nHttpMsgLen = sHttp.size();

    memcpy(m_pHttpMsg + m_nHttpMsgLen, m_pHttpBody, m_nHttpBodyLen);
    m_nHttpMsgLen += m_nHttpBodyLen;

    bool bRet = m_pSocketManage->SendMsg(m_pHttpMsg, m_nHttpMsgLen, 3, FACEQUALITY);
    if (!bRet)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 发送消息失败!\r\n%s", sHttp.c_str());
        nRet = STSDK_SENDMSGFAILED;
    }
    else
    {
        nRet = m_pSocketManage->RecvMsg(pResponseMsg, (unsigned int*)nMsgLen);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 接收HTTP回应消息失败");
        }
    }
        
    return nRet;
}
int CSTSDKClient::SynAddImage(const char * pDBName, const char * pImage, int nLen, int nGetFeature,
    int nQualityThreshold, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    if(m_pSocketManage->m_mapDBCountInfo.size() == 0)
    {
        char pRecvMsg[1024 * 10] = {0};
        unsigned int nRecvLen = 1024 * 10;
        GetDetail(pRecvMsg, &nRecvLen, nType);
        nRecvLen = 1024 * 10;
        GetAllDBInfo(pRecvMsg, &nRecvLen, nType);
    }

    int nRet = 0;
    
    HttpProtocol HttpInfo;
    HttpInfo.SetAddr(m_pSTAddr);
    HttpInfo.SetURL(string("/verify/face/synAdd"));
    HttpInfo.setRequestMethod(HL_HTTP_POST);
    HttpInfo.setRequestProperty("Accept", "image/jpeg, application/x-ms-application, image/gif, application/xaml+xml, image/pjpeg, application/x-ms-xbap, */*");
    HttpInfo.setRequestProperty("Referer", "XSCollectionServer");
    HttpInfo.setRequestProperty("Accept-Language", "en-US,en;q=0.8,zh-Hans-CN;q=0.5,zh-Hans;q=0.3");
    HttpInfo.setRequestProperty("Accept-Encoding", "gzip, deflate");
    char pContentType[128] = { 0 };
    string sBoundary = GetBoundary();
    sprintf_s(pContentType, sizeof(pContentType), "multipart/form-data; boundary=---------------------------%s", sBoundary.c_str());
    HttpInfo.setRequestProperty("Content-Type", pContentType);
    HttpInfo.setRequestProperty("User-Agent", "XSCollectionServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("DNT", "1");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");
    HttpInfo.setRequestProperty("Cache-Control", "no-cache");

    ZeroMemory(m_pHttpBody, HTTPSENDMSGSIZE);
    m_nHttpBodyLen = 0;

    sprintf_s(m_pHttpBody, HTTPSENDMSGSIZE,
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"dbName\"\r\n\r\n"
        "%s\r\n"
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"getFeature\"\r\n\r\n"
        "%d\r\n"
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"imageDatas\"; filename=\"%s.jpg\"\r\n"
        "Content-Type: image/pjpeg\r\n\r\n"
        , sBoundary.c_str(), pDBName, sBoundary.c_str(), nGetFeature, sBoundary.c_str(), sBoundary.c_str(), sBoundary.c_str());
    m_nHttpBodyLen += strlen(m_pHttpBody);

    memcpy(m_pHttpBody + m_nHttpBodyLen, pImage, nLen);
    m_nHttpBodyLen += nLen;

    char pTail[128] = { 0 };
    sprintf_s(pTail, sizeof(pTail), "\r\n-----------------------------%s--\r\n", sBoundary.c_str());
    memcpy(m_pHttpBody + m_nHttpBodyLen, pTail, strlen(pTail));
    m_nHttpBodyLen += strlen(pTail);

    char pBodyLen[12] = { 0 };
    sprintf_s(pBodyLen, sizeof(pBodyLen), "%d", m_nHttpBodyLen);
    HttpInfo.setRequestProperty("Content-Length", pBodyLen);

    string sHttp = HttpInfo.GetHttpHead();

    m_nHttpMsgLen = 0;
    if(sHttp.size() + m_nHttpBodyLen > HTTPSENDMSGSIZE)
    {
        delete []m_pHttpMsg;
        m_pHttpMsg = new char[sHttp.size() + m_nHttpBodyLen];
    }
    strcpy_s(m_pHttpMsg, HTTPSENDMSGSIZE, sHttp.c_str());
    m_nHttpMsgLen = sHttp.size();

    memcpy(m_pHttpMsg + m_nHttpMsgLen, m_pHttpBody, m_nHttpBodyLen);
    m_nHttpMsgLen += m_nHttpBodyLen;

    bool bRet = m_pSocketManage->SendMsg(m_pHttpMsg, m_nHttpMsgLen, nType, SYNADDIMAGE);
    if (!bRet)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 发送消息失败!\r\n%s", sHttp.c_str());
        nRet = STSDK_SENDMSGFAILED;
    }
    else
    {
        nRet = m_pSocketManage->RecvMsg(pResponseMsg, (unsigned int*)nMsgLen);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 接收HTTP回应消息失败");
        }
    }

    return nRet;
}
int CSTSDKClient::SynAddMultipleImage(const char * pDBName, STIMAGEINFO STImageInfo[], int nNum, int nGetFeature,
    int nQualityThreshold, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    if (m_pSocketManage->m_mapDBCountInfo.size() == 0)
    {
        char pRecvMsg[1024 * 10] = { 0 };
        unsigned int nRecvLen = 1024 * 10;
        GetDetail(pRecvMsg, &nRecvLen, nType);
        nRecvLen = 1024 * 10;
        GetAllDBInfo(pRecvMsg, &nRecvLen, nType);
    }

    //判断m_pHttpBody的长度是否足以装载所有的图片
    int nTotalLen = 0;
    for (int i = 0; i < nNum; i++)
    {
        nTotalLen += STImageInfo[i].nImageLen;
    }
    if (nTotalLen + 1024 * 10 > HTTPSENDMSGSIZE)
    {
        delete[]m_pHttpBody;
        m_pHttpBody = new char[nTotalLen + 1024 * 10];
    }

    int nRet = 0;

    HttpProtocol HttpInfo;
    HttpInfo.SetAddr(m_pSTAddr);
    HttpInfo.SetURL(string("/verify/face/synAdd"));
    HttpInfo.setRequestMethod(HL_HTTP_POST);
    HttpInfo.setRequestProperty("Accept", "image/jpeg, application/x-ms-application, image/gif, application/xaml+xml, image/pjpeg, application/x-ms-xbap, */*");
    HttpInfo.setRequestProperty("Referer", "XSCollectionServer");
    HttpInfo.setRequestProperty("Accept-Language", "en-US,en;q=0.8,zh-Hans-CN;q=0.5,zh-Hans;q=0.3");
    HttpInfo.setRequestProperty("Accept-Encoding", "gzip, deflate");
    char pContentType[128] = { 0 };
    string sBoundary = GetBoundary();
    sprintf_s(pContentType, sizeof(pContentType), "multipart/form-data; boundary=---------------------------%s", sBoundary.c_str());
    HttpInfo.setRequestProperty("Content-Type", pContentType);
    HttpInfo.setRequestProperty("User-Agent", "XSCollectionServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("DNT", "1");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");
    HttpInfo.setRequestProperty("Cache-Control", "no-cache");

    ZeroMemory(m_pHttpBody, HTTPSENDMSGSIZE);
    m_nHttpBodyLen = 0;

    sprintf_s(m_pHttpBody, HTTPSENDMSGSIZE,
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"dbName\"\r\n\r\n"
        "%s\r\n"
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"getFeature\"\r\n\r\n"
        "%d",
        sBoundary.c_str(), pDBName, sBoundary.c_str(), nGetFeature);
    m_nHttpBodyLen += strlen(m_pHttpBody);

    for (int i = 0; i < nNum; i++)
    {
        char pTemp[512] = { 0 };
        sprintf_s(pTemp, sizeof(pTemp),
            "\r\n-----------------------------%s\r\n"
            "Content-Disposition: form-data; name=\"imageDatas\"; filename=\"%s\"\r\n"
            "Content-Type: image/pjpeg\r\n\r\n",
            sBoundary.c_str(), STImageInfo[i].pName);
        memcpy(m_pHttpBody + m_nHttpBodyLen, pTemp, strlen(pTemp));
        m_nHttpBodyLen += strlen(pTemp);

        memcpy(m_pHttpBody + m_nHttpBodyLen, STImageInfo[i].pImageBuf, STImageInfo[i].nImageLen);
        m_nHttpBodyLen += STImageInfo[i].nImageLen;
    }

    char pTail[128] = { 0 };
    sprintf_s(pTail, sizeof(pTail), "\r\n-----------------------------%s--\r\n", sBoundary.c_str());
    memcpy(m_pHttpBody + m_nHttpBodyLen, pTail, strlen(pTail));
    m_nHttpBodyLen += strlen(pTail);

    char pBodyLen[12] = { 0 };
    sprintf_s(pBodyLen, sizeof(pBodyLen), "%d", m_nHttpBodyLen);
    HttpInfo.setRequestProperty("Content-Length", pBodyLen);

    string sHttp = HttpInfo.GetHttpHead();

    m_nHttpMsgLen = 0;
    if (sHttp.size() + m_nHttpBodyLen > HTTPSENDMSGSIZE)
    {
        delete[]m_pHttpMsg;
        m_pHttpMsg = new char[sHttp.size() + m_nHttpBodyLen];
    }
    strcpy_s(m_pHttpMsg, HTTPSENDMSGSIZE, sHttp.c_str());
    m_nHttpMsgLen = sHttp.size();

    memcpy(m_pHttpMsg + m_nHttpMsgLen, m_pHttpBody, m_nHttpBodyLen);
    m_nHttpMsgLen += m_nHttpBodyLen;

    bool bRet = m_pSocketManage->SendMsg(m_pHttpMsg, m_nHttpMsgLen, nType, SYNADDMULTIPLEIMAGE);
    if (!bRet)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 发送消息失败!\r\n%s", sHttp.c_str());
        nRet = STSDK_SENDMSGFAILED;
    }
    else
    {
        nRet = m_pSocketManage->RecvMsg(pResponseMsg, (unsigned int*)nMsgLen);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 接收HTTP回应消息失败");
        }
    }

    return nRet;
}
int CSTSDKClient::BatchGetFeature(STIMAGEINFO STImageInfo[], int nNum, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    //判断m_pHttpBody的长度是否足以装载所有的图片
    int nTotalLen = 0;
    for (int i = 0; i < nNum; i++)
    {
        nTotalLen += STImageInfo[i].nImageLen;
    }
    if (nTotalLen + 1024 * 10 > HTTPSENDMSGSIZE)
    {
        delete[]m_pHttpBody;
        m_pHttpBody = new char[nTotalLen + 1024 * 10];
    }

    int nRet = 0;

    HttpProtocol HttpInfo;
    HttpInfo.SetAddr(m_pSTAddr);
    HttpInfo.SetURL(string("/verify/feature/batchGet"));
    HttpInfo.setRequestMethod(HL_HTTP_POST);
    HttpInfo.setRequestProperty("Accept", "image/jpeg, application/x-ms-application, image/gif, application/xaml+xml, image/pjpeg, application/x-ms-xbap, */*");
    HttpInfo.setRequestProperty("Referer", "XSCollectionServer");
    HttpInfo.setRequestProperty("Accept-Language", "en-US,en;q=0.8,zh-Hans-CN;q=0.5,zh-Hans;q=0.3");
    HttpInfo.setRequestProperty("Accept-Encoding", "gzip, deflate");
    char pContentType[128] = { 0 };
    string sBoundary = GetBoundary();
    sprintf_s(pContentType, sizeof(pContentType), "multipart/form-data; boundary=---------------------------%s", sBoundary.c_str());
    HttpInfo.setRequestProperty("Content-Type", pContentType);
    HttpInfo.setRequestProperty("User-Agent", "XSCollectionServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("DNT", "1");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");
    HttpInfo.setRequestProperty("Cache-Control", "no-cache");

    ZeroMemory(m_pHttpBody, HTTPSENDMSGSIZE);
    m_nHttpBodyLen = 0;

    for (int i = 0; i < nNum; i++)
    {
        char pTemp[512] = { 0 };
        sprintf_s(pTemp, sizeof(pTemp),
            "-----------------------------%s\r\n"
            "Content-Disposition: form-data; name=\"imageDatas\"; filename=\"%s\"\r\n"
            "Content-Type: image/pjpeg\r\n\r\n",
            sBoundary.c_str(), STImageInfo[i].pName);
        memcpy(m_pHttpBody + m_nHttpBodyLen, pTemp, strlen(pTemp));
        m_nHttpBodyLen += strlen(pTemp);

        memcpy(m_pHttpBody + m_nHttpBodyLen, STImageInfo[i].pImageBuf, STImageInfo[i].nImageLen);
        m_nHttpBodyLen += STImageInfo[i].nImageLen;

        memcpy(m_pHttpBody + m_nHttpBodyLen, "\r\n", 2);
        m_nHttpBodyLen += 2;
    }

    char pTail[128] = { 0 };
    sprintf_s(pTail, sizeof(pTail), "-----------------------------%s--\r\n", sBoundary.c_str());
    memcpy(m_pHttpBody + m_nHttpBodyLen, pTail, strlen(pTail));
    m_nHttpBodyLen += strlen(pTail);

    char pBodyLen[12] = { 0 };
    sprintf_s(pBodyLen, sizeof(pBodyLen), "%d", m_nHttpBodyLen);
    HttpInfo.setRequestProperty("Content-Length", pBodyLen);

    string sHttp = HttpInfo.GetHttpHead();

    m_nHttpMsgLen = 0;
    if (sHttp.size() + m_nHttpBodyLen > HTTPSENDMSGSIZE)
    {
        delete[]m_pHttpMsg;
        m_pHttpMsg = new char[sHttp.size() + m_nHttpBodyLen];
    }
    strcpy_s(m_pHttpMsg, HTTPSENDMSGSIZE, sHttp.c_str());
    m_nHttpMsgLen = sHttp.size();

    memcpy(m_pHttpMsg + m_nHttpMsgLen, m_pHttpBody, m_nHttpBodyLen);
    m_nHttpMsgLen += m_nHttpBodyLen;

    bool bRet = m_pSocketManage->SendMsg(m_pHttpMsg, m_nHttpMsgLen, nType, SYNADDMULTIPLEIMAGE);
    if (!bRet)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 发送消息失败!\r\n%s", sHttp.c_str());
        nRet = STSDK_SENDMSGFAILED;
    }
    else
    {
        nRet = m_pSocketManage->RecvMsg(pResponseMsg, (unsigned int*)nMsgLen);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 接收HTTP回应消息失败");
        }
    }

    return nRet;
}
int CSTSDKClient::SynAddFeature(const char * pDBName, const char * pFeature, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    if(m_pSocketManage->m_mapDBCountInfo.size() == 0)
    {
        char pRecvMsg[1024 * 10] = {0};
        unsigned int nRecvLen = 1024 * 10;
        GetDetail(pRecvMsg, &nRecvLen, nType);
        nRecvLen = 1024 * 10;
        GetAllDBInfo(pRecvMsg, &nRecvLen, nType);
    }

    int nRet = 0;
    
    HttpProtocol HttpInfo;
    HttpInfo.SetAddr(m_pSTAddr);
    HttpInfo.SetURL(string("/verify/feature/synAdd"));
    HttpInfo.setRequestMethod(HL_HTTP_POST);
    HttpInfo.setRequestProperty("Accept", "image/jpeg, application/x-ms-application, image/gif, application/xaml+xml, image/pjpeg, application/x-ms-xbap, */*");
    HttpInfo.setRequestProperty("Referer", "XSCollectionServer");
    HttpInfo.setRequestProperty("Accept-Language", "en-US,en;q=0.8,zh-Hans-CN;q=0.5,zh-Hans;q=0.3");
    HttpInfo.setRequestProperty("Accept-Encoding", "gzip, deflate");
    char pContentType[128] = { 0 };
    string sBoundary = GetBoundary();
    sprintf_s(pContentType, sizeof(pContentType), "multipart/form-data; boundary=---------------------------%s", sBoundary.c_str());
    HttpInfo.setRequestProperty("Content-Type", pContentType);
    HttpInfo.setRequestProperty("User-Agent", "XSCollectionServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("DNT", "1");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");
    HttpInfo.setRequestProperty("Cache-Control", "no-cache");

    ZeroMemory(m_pHttpBody, HTTPSENDMSGSIZE);
    m_nHttpBodyLen = 0;

    sprintf_s(m_pHttpBody, HTTPSENDMSGSIZE,
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"dbName\"\r\n\r\n"
        "%s\r\n"
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"feature\"\r\n\r\n"
        "%s\r\n"
        "-----------------------------%s--\r\n"
        , sBoundary.c_str(), pDBName, sBoundary.c_str(), pFeature, sBoundary.c_str());
    m_nHttpBodyLen += strlen(m_pHttpBody);

    char pBodyLen[12] = { 0 };
    sprintf_s(pBodyLen, sizeof(pBodyLen), "%d", m_nHttpBodyLen);
    HttpInfo.setRequestProperty("Content-Length", pBodyLen);

    string sHttp = HttpInfo.GetHttpHead();

    m_nHttpMsgLen = 0;
    if(sHttp.size() + m_nHttpBodyLen > HTTPSENDMSGSIZE)
    {
        delete []m_pHttpMsg;
        m_pHttpMsg = new char[sHttp.size() + m_nHttpBodyLen];
    }
    strcpy_s(m_pHttpMsg, HTTPSENDMSGSIZE, sHttp.c_str());
    m_nHttpMsgLen = sHttp.size();

    memcpy(m_pHttpMsg + m_nHttpMsgLen, m_pHttpBody, m_nHttpBodyLen);
    m_nHttpMsgLen += m_nHttpBodyLen;

    bool bRet = m_pSocketManage->SendMsg(m_pHttpMsg, m_nHttpMsgLen, nType, SYNADDFEATURE);
    if (!bRet)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 发送消息失败!\r\n%s", sHttp.c_str());
        nRet = STSDK_SENDMSGFAILED;
    }
    else
    {
        nRet = m_pSocketManage->RecvMsg(pResponseMsg, (unsigned int*)nMsgLen);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 接收HTTP回应消息失败");
        }
    }

    return nRet;
}
int CSTSDKClient::GetImageByID(const char * pImageID, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    
    HttpProtocol HttpInfo;
    HttpInfo.SetAddr(m_pSTAddr);
    char pURL[1024] = { 0 };
    sprintf_s(pURL, sizeof(pURL), "/verify/face/gets?imageId=%s", pImageID);
    HttpInfo.SetURL(pURL);
    HttpInfo.setRequestMethod(HL_HTTP_GET);
    HttpInfo.setRequestProperty("Accept", "*/*");
    HttpInfo.setRequestProperty("Referer", "XSCollectionServer");
    HttpInfo.setRequestProperty("Accept-Language", "en-US,en;q=0.8,zh-Hans-CN;q=0.5,zh-Hans;q=0.3");
    HttpInfo.setRequestProperty("Accept-Encoding", "gzip, deflate");
    HttpInfo.setRequestProperty("User-Agent", "XSCollectionServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("DNT", "1");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");
    HttpInfo.setRequestProperty("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");

    string sHttp = HttpInfo.GetHttpHead();

    m_nHttpMsgLen = 0;
    strcpy_s(m_pHttpMsg, HTTPSENDMSGSIZE, sHttp.c_str());
    m_nHttpMsgLen = sHttp.size();

    bool bRet = m_pSocketManage->SendMsg(m_pHttpMsg, m_nHttpMsgLen, nType, GETIMAGEBYID);
    if (!bRet)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 发送消息失败!\r\n%s", sHttp.c_str());
        nRet = STSDK_SENDMSGFAILED;
    }
    else
    {
        nRet = m_pSocketManage->RecvMsg(pResponseMsg, (unsigned int*)nMsgLen);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 接收HTTP回应消息失败");
        }
    }

    return nRet;
}
int CSTSDKClient::DelImageByID(const char * pDBName, const char * pImageID, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    
    HttpProtocol HttpInfo;
    HttpInfo.SetAddr(m_pSTAddr);
    HttpInfo.SetURL(string("/verify/face/deletes"));
    HttpInfo.setRequestMethod(HL_HTTP_POST);
    HttpInfo.setRequestProperty("Accept", "text/plain, */*; q=0.01");
    HttpInfo.setRequestProperty("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
    HttpInfo.setRequestProperty("X-Requested-With", "XMLHttpRequest");
    HttpInfo.setRequestProperty("Referer", "XSCollectionServer");
    HttpInfo.setRequestProperty("Accept-Language", "en-US,en;q=0.8,zh-Hans-CN;q=0.5,zh-Hans;q=0.3");
    HttpInfo.setRequestProperty("Accept-Encoding", "gzip, deflate");
    HttpInfo.setRequestProperty("User-Agent", "XSCollectionServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("DNT", "1");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");
    HttpInfo.setRequestProperty("Cache-Control", "no-cache");

    ZeroMemory(m_pHttpBody, HTTPSENDMSGSIZE);
    m_nHttpBodyLen = 0;

    sprintf_s(m_pHttpBody, HTTPSENDMSGSIZE, "dbName=%s&imageId=%s", pDBName, pImageID);
    m_nHttpBodyLen = strlen(m_pHttpBody);

    char pBodyLen[12] = { 0 };
    sprintf_s(pBodyLen, sizeof(pBodyLen), "%d", m_nHttpBodyLen);
    HttpInfo.setRequestProperty("Content-Length", pBodyLen);

    string sHttp = HttpInfo.GetHttpHead();

    m_nHttpMsgLen = 0;
    if(sHttp.size() + m_nHttpBodyLen > HTTPSENDMSGSIZE)
    {
        delete []m_pHttpMsg;
        m_pHttpMsg = new char[sHttp.size() + m_nHttpBodyLen];
    }
    strcpy_s(m_pHttpMsg, HTTPSENDMSGSIZE, sHttp.c_str());
    m_nHttpMsgLen = sHttp.size();

    memcpy(m_pHttpMsg + m_nHttpMsgLen, m_pHttpBody, m_nHttpBodyLen);
    m_nHttpMsgLen += m_nHttpBodyLen;

    bool bRet = m_pSocketManage->SendMsg(m_pHttpMsg, m_nHttpMsgLen, nType, DELIMAGEBYID);
    if (!bRet)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 发送消息失败!\r\n%s", sHttp.c_str());
        nRet = STSDK_SENDMSGFAILED;
    }
    else
    {
        nRet = m_pSocketManage->RecvMsg(pResponseMsg, (unsigned int*)nMsgLen);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 接收HTTP回应消息失败");
        }
    }

    return nRet;
}
int CSTSDKClient::SearchImage(const char * pDBName, const char * pImage, int nLen, int nTopNum,
    double dbScore, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    
    HttpProtocol HttpInfo;
    HttpInfo.SetAddr(m_pSTAddr);
    HttpInfo.SetURL(string("/verify/face/search"));
    HttpInfo.setRequestMethod(HL_HTTP_POST);
    HttpInfo.setRequestProperty("Accept", "image/jpeg, application/x-ms-application, image/gif, application/xaml+xml, image/pjpeg, application/x-ms-xbap, */*");
    HttpInfo.setRequestProperty("Referer", "XSCollectionServer");
    HttpInfo.setRequestProperty("Accept-Language", "en-US,en;q=0.8,zh-Hans-CN;q=0.5,zh-Hans;q=0.3");
    HttpInfo.setRequestProperty("Accept-Encoding", "gzip, deflate");
    char pContentType[128] = { 0 };
    string sBoundary = GetBoundary();
    sprintf_s(pContentType, sizeof(pContentType), "multipart/form-data; boundary=---------------------------%s", sBoundary.c_str());
    HttpInfo.setRequestProperty("Content-Type", pContentType);
    HttpInfo.setRequestProperty("User-Agent", "XSCollectionServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("DNT", "1");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");
    HttpInfo.setRequestProperty("Cache-Control", "no-cache");

    ZeroMemory(m_pHttpBody, HTTPSENDMSGSIZE);
    m_nHttpBodyLen = 0;

    sprintf_s(m_pHttpBody, HTTPSENDMSGSIZE,
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"dbName\"\r\n\r\n"
        "%s\r\n"
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"topNum\"\r\n\r\n"
        "%d\r\n"
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"score\"\r\n\r\n"
        "%lf\r\n"
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"imageData\"; filename=\"%s.jpg\"\r\n"
        "Content-Type: image/pjpeg\r\n\r\n"
        , sBoundary.c_str(), pDBName, sBoundary.c_str(), nTopNum, sBoundary.c_str(), dbScore, sBoundary.c_str(), sBoundary.c_str());
    m_nHttpBodyLen += strlen(m_pHttpBody);

    memcpy(m_pHttpBody + m_nHttpBodyLen, pImage, nLen);
    m_nHttpBodyLen += nLen;

    char pTail[128] = { 0 };
    sprintf_s(pTail, sizeof(pTail), "\r\n-----------------------------%s--\r\n", sBoundary.c_str());
    memcpy(m_pHttpBody + m_nHttpBodyLen, pTail, strlen(pTail));
    m_nHttpBodyLen += strlen(pTail);

    char pBodyLen[12] = { 0 };
    sprintf_s(pBodyLen, sizeof(pBodyLen), "%d", m_nHttpBodyLen);
    HttpInfo.setRequestProperty("Content-Length", pBodyLen);

    string sHttp = HttpInfo.GetHttpHead();

    m_nHttpMsgLen = 0;
    if(sHttp.size() + m_nHttpBodyLen > HTTPSENDMSGSIZE)
    {
        delete []m_pHttpMsg;
        m_pHttpMsg = new char[sHttp.size() + m_nHttpBodyLen];
    }
    strcpy_s(m_pHttpMsg, HTTPSENDMSGSIZE, sHttp.c_str());
    m_nHttpMsgLen = sHttp.size();

    memcpy(m_pHttpMsg + m_nHttpMsgLen, m_pHttpBody, m_nHttpBodyLen);
    m_nHttpMsgLen += m_nHttpBodyLen;

    bool bRet = m_pSocketManage->SendMsg(m_pHttpMsg, m_nHttpMsgLen, nType, SEARCHIMAGE);
    if (!bRet)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 发送消息失败!\r\n%s", sHttp.c_str());
        nRet = STSDK_SENDMSGFAILED;
    }
    else
    {
        nRet = m_pSocketManage->RecvMsg(pResponseMsg, (unsigned int*)nMsgLen);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 接收HTTP回应消息失败");
        }
    }

    return nRet;
}
int CSTSDKClient::Verification(const char * pImage1, int nLen1, const char * pImage2, int nLen2, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    
    HttpProtocol HttpInfo;
    HttpInfo.SetAddr(m_pSTAddr);
    HttpInfo.SetURL(string("/verify/face/verification"));
    HttpInfo.setRequestMethod(HL_HTTP_POST);
    HttpInfo.setRequestProperty("Accept", "image/jpeg, application/x-ms-application, image/gif, application/xaml+xml, image/pjpeg, application/x-ms-xbap, */*");
    HttpInfo.setRequestProperty("Referer", "XSCollectionServer");
    HttpInfo.setRequestProperty("Accept-Language", "en-US,en;q=0.8,zh-Hans-CN;q=0.5,zh-Hans;q=0.3");
    HttpInfo.setRequestProperty("Accept-Encoding", "gzip, deflate");
    char pContentType[128] = { 0 };
    string sBoundary = GetBoundary();
    sprintf_s(pContentType, sizeof(pContentType), "multipart/form-data; boundary=---------------------------%s", sBoundary.c_str());
    HttpInfo.setRequestProperty("Content-Type", pContentType);
    HttpInfo.setRequestProperty("User-Agent", "XSCollectionServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("DNT", "1");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");
    HttpInfo.setRequestProperty("Cache-Control", "no-cache");

    ZeroMemory(m_pHttpBody, HTTPSENDMSGSIZE);
    m_nHttpBodyLen = 0;

    sprintf_s(m_pHttpBody, HTTPSENDMSGSIZE,
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"imageOne\"; filename=\"%s.jpg\"\r\n"
        "Content-Type: image/pjpeg\r\n\r\n"
        , sBoundary.c_str(), sBoundary.c_str());
    m_nHttpBodyLen += strlen(m_pHttpBody);

    memcpy(m_pHttpBody + m_nHttpBodyLen, pImage1, nLen1);
    m_nHttpBodyLen += nLen1;

    char pImage2Info[1024 * 2];
    sprintf_s(pImage2Info, sizeof(pImage2Info),
        "\r\n-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"imageTwo\"; filename=\"%s.jpg\"\r\n"
        "Content-Type: image/pjpeg\r\n\r\n"
        , sBoundary.c_str(), GetBoundary().c_str());
    memcpy(m_pHttpBody + m_nHttpBodyLen, pImage2Info, strlen(pImage2Info));
    m_nHttpBodyLen += strlen(pImage2Info);

    memcpy(m_pHttpBody + m_nHttpBodyLen, pImage2, nLen2);
    m_nHttpBodyLen += nLen2;

    char pTail[128] = { 0 };
    sprintf_s(pTail, sizeof(pTail), "\r\n-----------------------------%s--\r\n", sBoundary.c_str());
    memcpy(m_pHttpBody + m_nHttpBodyLen, pTail, strlen(pTail));
    m_nHttpBodyLen += strlen(pTail);

    char pBodyLen[12] = { 0 };
    sprintf_s(pBodyLen, sizeof(pBodyLen), "%d", m_nHttpBodyLen);
    HttpInfo.setRequestProperty("Content-Length", pBodyLen);

    string sHttp = HttpInfo.GetHttpHead();

    m_nHttpMsgLen = 0;
    if(sHttp.size() + m_nHttpBodyLen > HTTPSENDMSGSIZE)
    {
        delete []m_pHttpMsg;
        m_pHttpMsg = new char[sHttp.size() + m_nHttpBodyLen];
    }
    strcpy_s(m_pHttpMsg, HTTPSENDMSGSIZE, sHttp.c_str());
    m_nHttpMsgLen = sHttp.size();

    memcpy(m_pHttpMsg + m_nHttpMsgLen, m_pHttpBody, m_nHttpBodyLen);
    m_nHttpMsgLen += m_nHttpBodyLen;

    bool bRet = m_pSocketManage->SendMsg(m_pHttpMsg, m_nHttpMsgLen, 3, VERIFICATION);
    if (!bRet)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 发送消息失败!\r\n%s", sHttp.c_str());
        nRet = STSDK_SENDMSGFAILED;
    }
    else
    {
        nRet = m_pSocketManage->RecvMsg(pResponseMsg, (unsigned int*)nMsgLen);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 接收HTTP回应消息失败");
        }
    }

    return nRet;
}
int CSTSDKClient::SearchImageByFeature(const char * pDBName, const char * pFeature, int nLen, int nTopNum,
    double dbScore, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    
    HttpProtocol HttpInfo;
    HttpInfo.SetAddr(m_pSTAddr);
    HttpInfo.SetURL(string("/verify/feature/search"));
    HttpInfo.setRequestMethod(HL_HTTP_POST);
    HttpInfo.setRequestProperty("Accept", "image/jpeg, application/x-ms-application, image/gif, application/xaml+xml, image/pjpeg, application/x-ms-xbap, */*");
    HttpInfo.setRequestProperty("Referer", "XSCollectionServer");
    HttpInfo.setRequestProperty("Accept-Language", "en-US,en;q=0.8,zh-Hans-CN;q=0.5,zh-Hans;q=0.3");
    HttpInfo.setRequestProperty("Accept-Encoding", "gzip, deflate");
    char pContentType[128] = { 0 };
    string sBoundary = GetBoundary();
    sprintf_s(pContentType, sizeof(pContentType), "multipart/form-data; boundary=---------------------------%s", sBoundary.c_str());
    HttpInfo.setRequestProperty("Content-Type", pContentType);
    HttpInfo.setRequestProperty("User-Agent", "XSCollectionServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("DNT", "1");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");
    HttpInfo.setRequestProperty("Cache-Control", "no-cache");

    ZeroMemory(m_pHttpBody, HTTPSENDMSGSIZE);
    m_nHttpBodyLen = 0;

    sprintf_s(m_pHttpBody, HTTPSENDMSGSIZE,
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"dbName\"\r\n\r\n"
        "%s\r\n"
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"topNum\"\r\n\r\n"
        "%d\r\n"
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"score\"\r\n\r\n"
        "%lf\r\n"
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"feature\"\r\n\r\n"
        "%s\r\n"
        "-----------------------------%s--\r\n"
        , sBoundary.c_str(), pDBName, sBoundary.c_str(), nTopNum, sBoundary.c_str(), dbScore, sBoundary.c_str(), pFeature, sBoundary.c_str());
    m_nHttpBodyLen += strlen(m_pHttpBody);

    char pBodyLen[12] = { 0 };
    sprintf_s(pBodyLen, sizeof(pBodyLen), "%d", m_nHttpBodyLen);
    HttpInfo.setRequestProperty("Content-Length", pBodyLen);

    string sHttp = HttpInfo.GetHttpHead();

    m_nHttpMsgLen = 0;
    if(sHttp.size() + m_nHttpBodyLen > HTTPSENDMSGSIZE)
    {
        delete []m_pHttpMsg;
        m_pHttpMsg = new char[sHttp.size() + m_nHttpBodyLen];
    }
    strcpy_s(m_pHttpMsg, HTTPSENDMSGSIZE, sHttp.c_str());
    m_nHttpMsgLen = sHttp.size();

    memcpy(m_pHttpMsg + m_nHttpMsgLen, m_pHttpBody, m_nHttpBodyLen);
    m_nHttpMsgLen += m_nHttpBodyLen;

    bool bRet = m_pSocketManage->SendMsg(m_pHttpMsg, m_nHttpMsgLen, nType, SEARCHIMAGEBYFEATURE);
    if (!bRet)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 发送消息失败!\r\n%s", sHttp.c_str());
        nRet = STSDK_SENDMSGFAILED;
    }
    else
    {
        nRet = m_pSocketManage->RecvMsg(pResponseMsg, (unsigned int*)nMsgLen);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 接收HTTP回应消息失败");
        }
    }

    return nRet;
}
int CSTSDKClient::GetAttribute(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    
    HttpProtocol HttpInfo;
    HttpInfo.SetAddr(m_pSTAddr);
    HttpInfo.SetURL(string("/verify/attribute/gets"));
    HttpInfo.setRequestMethod(HL_HTTP_POST);
    HttpInfo.setRequestProperty("Accept", "image/jpeg, application/x-ms-application, image/gif, application/xaml+xml, image/pjpeg, application/x-ms-xbap, */*");
    HttpInfo.setRequestProperty("Referer", "XSCollectionServer");
    HttpInfo.setRequestProperty("Accept-Language", "en-US,en;q=0.8,zh-Hans-CN;q=0.5,zh-Hans;q=0.3");
    HttpInfo.setRequestProperty("Accept-Encoding", "gzip, deflate");
    char pContentType[128] = { 0 };
    string sBoundary = GetBoundary();
    sprintf_s(pContentType, sizeof(pContentType), "multipart/form-data; boundary=---------------------------%s", sBoundary.c_str());
    HttpInfo.setRequestProperty("Content-Type", pContentType);
    HttpInfo.setRequestProperty("User-Agent", "XSCollectionServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("DNT", "1");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");
    HttpInfo.setRequestProperty("Cache-Control", "no-cache");

    ZeroMemory(m_pHttpBody, HTTPSENDMSGSIZE);
    m_nHttpBodyLen = 0;

    sprintf_s(m_pHttpBody, HTTPSENDMSGSIZE,
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"imageData\"; filename=\"%s.jpg\"\r\n"
        "Content-Type: image/pjpeg\r\n\r\n"
        , sBoundary.c_str(), sBoundary.c_str());
    m_nHttpBodyLen += strlen(m_pHttpBody);

    memcpy(m_pHttpBody + m_nHttpBodyLen, pImage, nLen);
    m_nHttpBodyLen += nLen;

    char pTail[128] = { 0 };
    sprintf_s(pTail, sizeof(pTail), "\r\n-----------------------------%s--\r\n", sBoundary.c_str());
    memcpy(m_pHttpBody + m_nHttpBodyLen, pTail, strlen(pTail));
    m_nHttpBodyLen += strlen(pTail);

    char pBodyLen[12] = { 0 };
    sprintf_s(pBodyLen, sizeof(pBodyLen), "%d", m_nHttpBodyLen);
    HttpInfo.setRequestProperty("Content-Length", pBodyLen);

    string sHttp = HttpInfo.GetHttpHead();

    m_nHttpMsgLen = 0;
    if(sHttp.size() + m_nHttpBodyLen > HTTPSENDMSGSIZE)
    {
        delete []m_pHttpMsg;
        m_pHttpMsg = new char[sHttp.size() + m_nHttpBodyLen];
    }
    strcpy_s(m_pHttpMsg, HTTPSENDMSGSIZE, sHttp.c_str());
    m_nHttpMsgLen = sHttp.size();

    memcpy(m_pHttpMsg + m_nHttpMsgLen, m_pHttpBody, m_nHttpBodyLen);
    m_nHttpMsgLen += m_nHttpBodyLen;

    bool bRet = m_pSocketManage->SendMsg(m_pHttpMsg, m_nHttpMsgLen, 3, GETATTRIBUTE);
    if (!bRet)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 发送消息失败!\r\n%s", sHttp.c_str());
        nRet = STSDK_SENDMSGFAILED;
    }
    else
    {
        nRet = m_pSocketManage->RecvMsg(pResponseMsg, (unsigned int*)nMsgLen);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 接收HTTP回应消息失败");
        }
    }
        
    return nRet;
}
int CSTSDKClient::GetFeature(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    
    HttpProtocol HttpInfo;
    HttpInfo.SetAddr(m_pSTAddr);
    HttpInfo.SetURL(string("/verify/feature/gets"));
    HttpInfo.setRequestMethod(HL_HTTP_POST);
    HttpInfo.setRequestProperty("Accept", "image/jpeg, application/x-ms-application, image/gif, application/xaml+xml, image/pjpeg, application/x-ms-xbap, */*");
    HttpInfo.setRequestProperty("Referer", "XSCollectionServer");
    HttpInfo.setRequestProperty("Accept-Language", "en-US,en;q=0.8,zh-Hans-CN;q=0.5,zh-Hans;q=0.3");
    HttpInfo.setRequestProperty("Accept-Encoding", "gzip, deflate");
    char pContentType[128] = { 0 };
    string sBoundary = GetBoundary();
    sprintf_s(pContentType, sizeof(pContentType), "multipart/form-data; boundary=---------------------------%s", sBoundary.c_str());
    HttpInfo.setRequestProperty("Content-Type", pContentType);
    HttpInfo.setRequestProperty("User-Agent", "XSCollectionServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("DNT", "1");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");
    HttpInfo.setRequestProperty("Cache-Control", "no-cache");

    ZeroMemory(m_pHttpBody, HTTPSENDMSGSIZE);
    m_nHttpBodyLen = 0;

    sprintf_s(m_pHttpBody, HTTPSENDMSGSIZE,
        "-----------------------------%s\r\n"
        "Content-Disposition: form-data; name=\"imageData\"; filename=\"%s.jpg\"\r\n"
        "Content-Type: image/pjpeg\r\n\r\n"
        , sBoundary.c_str(), sBoundary.c_str());
    m_nHttpBodyLen += strlen(m_pHttpBody);

    memcpy(m_pHttpBody + m_nHttpBodyLen, pImage, nLen);
    m_nHttpBodyLen += nLen;

    char pTail[128] = { 0 };
    sprintf_s(pTail, sizeof(pTail), "\r\n-----------------------------%s--\r\n", sBoundary.c_str());
    memcpy(m_pHttpBody + m_nHttpBodyLen, pTail, strlen(pTail));
    m_nHttpBodyLen += strlen(pTail);

    char pBodyLen[12] = { 0 };
    sprintf_s(pBodyLen, sizeof(pBodyLen), "%d", m_nHttpBodyLen);
    HttpInfo.setRequestProperty("Content-Length", pBodyLen);

    string sHttp = HttpInfo.GetHttpHead();

    m_nHttpMsgLen = 0;
    if(sHttp.size() + m_nHttpBodyLen > HTTPSENDMSGSIZE)
    {
        delete []m_pHttpMsg;
        m_pHttpMsg = new char[sHttp.size() + m_nHttpBodyLen];
    }
    strcpy_s(m_pHttpMsg, HTTPSENDMSGSIZE, sHttp.c_str());
    m_nHttpMsgLen = sHttp.size();

    memcpy(m_pHttpMsg + m_nHttpMsgLen, m_pHttpBody, m_nHttpBodyLen);
    m_nHttpMsgLen += m_nHttpBodyLen;

    bool bRet = m_pSocketManage->SendMsg(m_pHttpMsg, m_nHttpMsgLen, 3, GETFEATURE);
    if (!bRet)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 发送消息失败!\r\n%s", sHttp.c_str());
        nRet = STSDK_SENDMSGFAILED;
    }
    else
    {
        nRet = m_pSocketManage->RecvMsg(pResponseMsg, (unsigned int*)nMsgLen);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 接收HTTP回应消息失败");
        }
    }
        
    return nRet;
}

int CSTSDKClient::GetDetail(char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    int nRet = 0;
    
    HttpProtocol HttpInfo;
    HttpInfo.SetAddr(m_pSTAddr);
    HttpInfo.SetURL(string("/verify/detail"));
    HttpInfo.setRequestMethod(HL_HTTP_GET);
    HttpInfo.setRequestProperty("X-Requested-With", "XMLHttpRequest");
    HttpInfo.setRequestProperty("Accept", "text/plain, */*; q=0.01");
    HttpInfo.setRequestProperty("Referer", "XSCollectionServer");
    HttpInfo.setRequestProperty("Accept-Language", "en-US,en;q=0.8,zh-Hans-CN;q=0.5,zh-Hans;q=0.3");
    HttpInfo.setRequestProperty("Accept-Encoding", "gzip, deflate");
    HttpInfo.setRequestProperty("User-Agent", "XSCollectionServer");
    HttpInfo.setRequestProperty("Host", m_pSTAddr);
    HttpInfo.setRequestProperty("DNT", "1");
    HttpInfo.setRequestProperty("Connection", "Keep-Alive");

    string sHttp = HttpInfo.GetHttpHead();

    m_nHttpMsgLen = 0;
    strcpy_s(m_pHttpMsg, HTTPSENDMSGSIZE, sHttp.c_str());
    m_nHttpMsgLen = sHttp.size();

    bool bRet = m_pSocketManage->SendMsg(m_pHttpMsg, m_nHttpMsgLen, nType, GETDETAIL);
    if (!bRet)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 发送消息失败!\r\n%s", sHttp.c_str());
        nRet = STSDK_SENDMSGFAILED;
    }
    else
    {
        nRet = m_pSocketManage->RecvMsg(pResponseMsg, (unsigned int*)nMsgLen);
        if (nRet != 0)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 接收HTTP回应消息失败");
        }
    }

    return nRet;
}

string CSTSDKClient::GetBoundary()
{
    return "FFFFFFFFFF";
    char pBoundary[32] = {0};
    sprintf_s(pBoundary, sizeof(pBoundary), "%x%x", m_nBoundaryHigh++, m_nBoundaryLow++);
    if(m_nBoundaryHigh >= 4000000000)
    {
        m_nBoundaryLow = (rand() * rand()) % 40000 + 1000000;
        m_nBoundaryHigh = (rand() * rand()) % 40000 + 5000000;
    }
    return pBoundary;
}
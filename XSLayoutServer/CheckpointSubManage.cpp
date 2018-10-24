#include "StdAfx.h"
#include "CheckpointSubManage.h"


extern CLogRecorder g_LogRecorder;
CCheckpointSubManage::CCheckpointSubManage()
{
    ZeroMemory(m_pProxyServerIP, sizeof(m_pProxyServerIP));
    m_nProxyPort = 0;
    m_pZeromqManage = NULL;
}
CCheckpointSubManage::~CCheckpointSubManage()
{
}
bool CCheckpointSubManage::Init(char * pProxyServerIP, int nProxyServerPort, LPSubMessageCallback pCallback, void * pUser)
{
    m_pCallback = pCallback;
    m_pUser = pUser;
    strcpy_s(m_pProxyServerIP, sizeof(m_pProxyServerIP), pProxyServerIP);
    m_nProxyPort = nProxyServerPort;

    if (!InitZeromq())
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "InitZeromq[%s:%d] Failed!", m_pProxyServerIP, m_nProxyPort);
        return false;
    }

    return true;
}
void CCheckpointSubManage::UnInit()
{
    if (NULL == m_pZeromqManage)
    {
        m_pZeromqManage->UnInit();
        delete m_pZeromqManage;
        m_pZeromqManage = NULL;
    }
    return;
}
bool CCheckpointSubManage::InitZeromq()
{
    if (NULL == m_pZeromqManage)
    {
        m_pZeromqManage = new CZeromqManage;
    }
    if (!m_pZeromqManage->InitSub(NULL, 0, m_pProxyServerIP, m_nProxyPort, ZeromqSubMsg, this))
    {
        printf("****Error: InitSub[%s:%d]失败!", m_pProxyServerIP, m_nProxyPort);
        return false;
    }
    return true;
}
void CCheckpointSubManage::ZeromqSubMsg(LPSUBMESSAGE pSubMessage, void * pUser)
{
    CCheckpointSubManage * pThis = (CCheckpointSubManage *)pUser;
    pThis->ParseZeromqJson(pSubMessage);
}
bool CCheckpointSubManage::ParseZeromqJson(LPSUBMESSAGE pSubMessage)
{
    int nRet = INVALIDERROR;

    string sCommand(pSubMessage->pOperationType);
    rapidjson::Document document;
    if (string(pSubMessage->pSubJsonValue) != "")
    {
        document.Parse(pSubMessage->pSubJsonValue);
        if (document.HasParseError())
        {
            printf("***Warning: Parse Json Format Failed[%s].\n", pSubMessage->pSubJsonValue);
            return false;
        }
    }
    if (sCommand == COMMANDADD)        //增加图片
    {
            if (document.HasMember(JSONFACEUUID) && document[JSONFACEUUID].IsString() && strlen(document[JSONFACEUUID].GetString()) < MAXLEN &&
                document.HasMember(JSONFEATURE) && document[JSONFEATURE].IsString() && strlen(document[JSONFEATURE].GetString()) < FEATURELEN &&
                document.HasMember(JSONTIME) && document[JSONTIME].IsInt64() && strlen(document[JSONFEATURE].GetString()) > FEATUREMIXLEN &&
                document.HasMember(JSONDRIVE) && document[JSONDRIVE].IsString() && strlen(document[JSONDRIVE].GetString()) == 1 &&
                document.HasMember(JSONSERVERIP) && document[JSONSERVERIP].IsString() && strlen(document[JSONSERVERIP].GetString()) < MAXIPLEN &&
                document.HasMember(JSONFACERECT) && document[JSONFACERECT].IsString() && strlen(document[JSONFACERECT].GetString()) < MAXIPLEN)
            {
                //1. 保存FaceUUID
                strcpy(pSubMessage->pFaceUUID, document[JSONFACEUUID].GetString());
                //2. 保存特征值
                strcpy(pSubMessage->pFeature, document[JSONFEATURE].GetString());
                //3. 保存特征值时间
                int64_t nTime = document[JSONTIME].GetInt64();
                pSubMessage->nImageTime = int(nTime / 1000);
                //4. 保存磁盘信息
                strcpy(pSubMessage->pDisk, document[JSONDRIVE].GetString());
                //5. 保存图片服务器IP
                strcpy(pSubMessage->pImageIP, document[JSONSERVERIP].GetString());
                //6.  保存人脸坐标
                strcpy(pSubMessage->pFaceRect, document[JSONFACERECT].GetString());
                //7. 保存卡口ID
                strcpy(pSubMessage->pDeviceID, pSubMessage->pHead);
                //8. 保存图片URL
                if (document.HasMember(JSONFACEURL) && document[JSONFACEURL].IsString() && strlen(document[JSONFACEURL].GetString()) < 2048 &&
                    document.HasMember(JSONBKGURL) && document[JSONBKGURL].IsString() && strlen(document[JSONBKGURL].GetString()) < 2048)
                {
                    strcpy(pSubMessage->pFaceURL, document[JSONFACEURL].GetString());
                    strcpy(pSubMessage->pBkgURL, document[JSONBKGURL].GetString());
                }
                //回调抓拍图片
                m_pCallback(pSubMessage, m_pUser);
        }
    }
}
bool CCheckpointSubManage::AddSubCheckpoint(char * pCheckpointID)
{
    m_pZeromqManage->AddSubMessage(pCheckpointID);
    return true;
}
bool CCheckpointSubManage::DelSubCheckpoint(char * pCheckpointID)
{
    m_pZeromqManage->DelSubMessage(pCheckpointID);
    return true;
}

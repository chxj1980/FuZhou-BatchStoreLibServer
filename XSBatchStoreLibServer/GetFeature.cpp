#include "StdAfx.h"
#include "GetFeature.h"
#include <objbase.h>

extern CLogRecorder g_LogRecorder;
CGetFeature::CGetFeature()
{
    m_hStopEvent = CreateEvent(NULL, true, false, NULL);
    InitializeCriticalSection(&m_cs);
    m_nPushNum = 0;
}
CGetFeature::~CGetFeature()
{
    CloseHandle(m_hStopEvent);
    DeleteCriticalSection(&m_cs);
}
int CGetFeature::Init(pStoreImageCallback pCallback, void * pUser, STSERVERINFO STServerInfo[], int nQuantity)
{
    m_pImageCallback = pCallback;
    m_pUser = pUser;

    int nRet = STSDK_Init(STServerInfo, nQuantity);
    if (nRet != 0)
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "****Error: STSDK_Init��ʼ����������ʧ��!");
        return STSDKInitFailed;
    }

    for (int i = 0; i < THREADNUM; i++)
    {
        CreateThread(NULL, 0, GetImageFeatureThread, this, NULL, 0);   //���ػ�ȡ����ֵ�߳�
    }
    return 0;
}
void CGetFeature::UnInit()
{
    STSDK_UnInit();
    SetEvent(m_hStopEvent);
    Sleep(100);
}
int CGetFeature::AddFaceImageInfo(LPSTOREFACEINFO pStoreImageInfo)
{
    EnterCriticalSection(&m_cs);
    m_listStoreImageInfo.push_back(pStoreImageInfo);
    LeaveCriticalSection(&m_cs);
    return 0;
}
DWORD CGetFeature::GetImageFeatureThread(LPVOID lParam)
{
    CGetFeature * pThis = (CGetFeature*)lParam;
    pThis->GetImageFeatureAction();
    return 0;
}
void CGetFeature::GetImageFeatureAction()
{
    LPSTOREFACEINFO pStoreImageInfo = NULL;
    char * pSTResponseMsg = new char[STRESPONSELEN];
    unsigned int nSTRespLen = STRESPONSELEN;
    while (WAIT_TIMEOUT == WaitForSingleObject(m_hStopEvent, THREADWAITTIME))
    {
        do
        {
            EnterCriticalSection(&m_cs);
            if (m_listStoreImageInfo.size() > 0)
            {
                pStoreImageInfo = m_listStoreImageInfo.front();
                m_listStoreImageInfo.pop_front();
            }
            else
            {
                pStoreImageInfo = NULL;
            }
            LeaveCriticalSection(&m_cs);

            if (pStoreImageInfo != NULL)
            {
                STAddStoreImage(pStoreImageInfo, pSTResponseMsg, nSTRespLen);
                m_pImageCallback(pStoreImageInfo, m_pUser);
            }
        } while (NULL != pStoreImageInfo);
    }
    return;
}
int CGetFeature::STAddStoreImage(LPSTOREFACEINFO pStoreImageInfo, char * pSTResponseMsg, unsigned int nSTRespLen)
{
    int nRet = 0;
    LPSTIMAGEINFO STImageInfo = new STIMAGEINFO[pStoreImageInfo->listPushSTImageInfo.size()];

    std::string sFaceBuf = "";
    int nFaceLen = 0;

    int i = 0;
    for (LISTPUSHSTIMAGEINFO::iterator itImage = pStoreImageInfo->listPushSTImageInfo.begin();
        itImage != pStoreImageInfo->listPushSTImageInfo.end(); itImage++)
    {
        if ((*itImage)->bUsed)
        {
            nFaceLen = 0;
            //���ͻ��˴����ͼƬ, ��Base64����תΪAscII��
            sFaceBuf = ZBase64::Decode((*itImage)->pImageBuf, (*itImage)->nImageLen, nFaceLen);
            //��ͼƬ�����ݱ��浽pStoreImageInfo
            if ((*itImage)->nImageBufMaxLen < nFaceLen)
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__,
                    "***Warning: ����ͼƬZBase64���泤��[%d] < �����ͼƬ����[%d]", (*itImage)->nImageBufMaxLen, nFaceLen);

                delete (*itImage)->pImageBuf;
                (*itImage)->pImageBuf = new char[nFaceLen + 1];
                (*itImage)->nImageBufMaxLen = nFaceLen + 1;
            }
            memcpy((*itImage)->pImageBuf, sFaceBuf.c_str(), nFaceLen);
            (*itImage)->nImageLen = nFaceLen;
            strcpy_s((*itImage)->pFaceUUID, sizeof((*itImage)->pFaceUUID), GetUUID().c_str());
            (*itImage)->nFaceQuality = STRepFileNameNotExist;

            STImageInfo[i].pImageBuf = (*itImage)->pImageBuf;
            STImageInfo[i].nImageLen = (*itImage)->nImageLen;
            strcpy_s(STImageInfo[i].pName, sizeof(STImageInfo[i].pName), (*itImage)->pImageID);
            i++;

            g_LogRecorder.WriteInfoLogEx(__FUNCTION__, "Name[%s], FaceUUUID[%s].", (*itImage)->pImageID, (*itImage)->pFaceUUID);
        }
        else
        {
            break;
        }
    }

    nSTRespLen = STRESPONSELEN;
    nRet = STSDK_SynAddMultipleImage(STTESTLIB, STImageInfo, i, 0, 0, pSTResponseMsg, &nSTRespLen, 2);
    if (nRet != 0)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***warning: STSDK_SynAddMultipleImage Failed, Return[%d].", nRet);
        nRet = -1;
    }
    else
    {
        nRet = GetSTImageID(pSTResponseMsg, nSTRespLen, pStoreImageInfo);
        if (nRet == 0)
        {
            g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "������ֵ�����ȡ�ص��[%d]����ֵ�ɹ�.", pStoreImageInfo->nStoreLibID);
            pStoreImageInfo->nEvent = 0;
            m_nPushNum += i;
        }
        else
        {
            if (string(pSTResponseMsg, nSTRespLen).find("DB_NOT_EXISTS") != string::npos || string(pSTResponseMsg, nSTRespLen).find("dbName is none") != string::npos)
            {
                if (!CreateStoreLib(STTESTLIB, pSTResponseMsg, nSTRespLen))
                {
                    g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: �����ص��[%s]ʧ��.", STTESTLIB);
                    g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***warning: STSDK_SynAddImage Failed, Return[%d].", nRet);
                    pStoreImageInfo->nEvent = GetSTFeatureFailed;
                }
                else
                {
                    g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "**Info: �����ص��[%s]�ɹ�.", STTESTLIB);
                    nSTRespLen = STRESPONSELEN;
                    nRet = STSDK_SynAddMultipleImage(STTESTLIB, STImageInfo, i, 0, 0, pSTResponseMsg, &nSTRespLen, 2);
                    if (nRet != 0)
                    {
                        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***warning: STSDK_SynAddMultipleImage Failed, Return[%d].", nRet);
                        pStoreImageInfo->nEvent = GetSTFeatureFailed;
                    }
                    else
                    {
                        nRet = GetSTImageID(pSTResponseMsg, nSTRespLen, pStoreImageInfo);
                        if (nRet == 0)
                        {
                            m_nPushNum += i;
                            g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "������ֵ�����ȡ�ص��[%d]����ֵ�ɹ�.", pStoreImageInfo->nStoreLibID);
                        }
                        pStoreImageInfo->nEvent = nRet;
                    }
                }
            }
            else
            {
                pStoreImageInfo->nEvent = nRet;
            }
        }
    }

    if (m_nPushNum > 10000) //�����ʱ������ > 10000, ���
    {
        nSTRespLen = STRESPONSELEN;
        STSDK_OperationDB(STTESTLIB, 3, pSTResponseMsg, &nSTRespLen, 2);
        m_nPushNum = 0;
    }
    return nRet;
}

bool CGetFeature::CreateStoreLib(char * pSTFeature, char * pSTResponseMsg, unsigned int nSTRespLen)
{
    bool bRet = false;
    nSTRespLen = STRESPONSELEN;
    int nRet = STSDK_OperationDB(pSTFeature, 1, pSTResponseMsg, &nSTRespLen, 2);
    if (nRet != 0)
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***warning: STSDK_OperationDB Add StoreLib Failed, Return[%d].", nRet);
    }
    else
    {
        if (!GetSTResponseResult(pSTResponseMsg, nSTRespLen))
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: ����ST�ص��[%s]ʧ��.", pSTFeature);
        }
        else
        {
            g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "����ST�ص��[%s]�ɹ�.", pSTFeature);
            bRet = true;
        }
    }

    return bRet;
}
bool CGetFeature::GetSTResponseResult(char * pMessage, int nLen)
{
    bool bRet = false;
    rapidjson::Document document;
    document.Parse(pMessage);
    if (document.HasParseError())
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "����Json��ʧ��[%s]", pMessage);
    }
    else
    {
        if (document.HasMember("result") && document["result"].IsString())
        {
            string sResult = document["result"].GetString();
            if (sResult != "success")
            {
                if (document.HasMember("errorMessage") && document["errorMessage"].IsString())
                {
                    string sErrorMsg = document["errorMessage"].GetString();
                    if (sErrorMsg != "FAILED_TO_CREATE_DB_EXISTS")   //����ST��ʱ, ���Ӧ�˴���, ��ʾ�⼺����, ���Ƿ��سɹ�
                    {
                        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: ST��Ӧ[error], ErrorMsg[%s].", sErrorMsg.c_str());
                    }
                    else
                    {
                        bRet = true;
                    }
                }
            }
            else
            {
                bRet = true;
            }
        }
    }
    return bRet;
}
int CGetFeature::GetSTImageID(char * pMessage, int nLen, LPSTOREFACEINFO pStoreImageInfo)
{
    int nRet = 0;
    rapidjson::Document document;
    document.Parse(pMessage);
    if (document.HasParseError())
    {
        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "����Json��ʧ��[%s]", pMessage);
        nRet = STJsonFormatFailed;
    }
    else
    {
        if (document.HasMember("errorMessage") && document["errorMessage"].IsString())
        {
            string sErrorMsg = document["errorMessage"].GetString();
            g_LogRecorder.WriteErrorLogEx(__FUNCTION__, "***Warning: ���������������ص�������Ϣ[%s]", sErrorMsg.c_str());
            pStoreImageInfo->nEvent = AddImageToSTFailed;
            nRet = AddImageToSTFailed;
        }
        else
        {
            //�������ʧ�ܵ�ͼƬ������Ϣ
            if (document.HasMember("fail") && document["fail"].IsArray() && document["fail"].Size() > 0)
            {
                for (int i = 0; i < document["fail"].Size(); i++)
                {
                    if (document["fail"][i].HasMember("name") && document["fail"][i].HasMember("errReason") &&
                        document["fail"][i]["name"].IsString() && document["fail"][i]["errReason"].IsString())
                    {
                        string sName = document["fail"][i]["name"].GetString();
                        string sReason = document["fail"][i]["errReason"].GetString();

                        LISTPUSHSTIMAGEINFO::iterator itImage = pStoreImageInfo->listPushSTImageInfo.begin();
                        for (; itImage != pStoreImageInfo->listPushSTImageInfo.end(); itImage++)
                        {
                            if (string((*itImage)->pImageID) == sName)
                            {
                                (*itImage)->nFaceQuality = STRepGetFeatureFailed;
                                break;
                            }
                        }
                        if (itImage == pStoreImageInfo->listPushSTImageInfo.end())
                        {
                            (*itImage)->nFaceQuality = STRepFileNameNotExist;
                            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: ����������Ϣ�ļ���[%s]������", sName.c_str());
                        }
                    }
                    else
                    {
                        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "***Waring: �������������Ϣ��ʽ����, fail����name����reason�ֶ�");
                    }
                }
            }

            //�����ɹ�����ͼƬ������Ϣ
            if (document.HasMember("success") && document["success"].IsArray() && document["success"].Size() > 0)
            {
                for (int i = 0; i < document["success"].Size(); i++)
                {
                    if (document["success"][i].HasMember("imageId") && document["success"][i]["imageId"].IsString() &&
                        document["success"][i].HasMember("feature") && document["success"][i]["feature"].IsString() &&
                        document["success"][i].HasMember("name")    && document["success"][i]["name"].IsString())
                        
                    {
                        string sSTImageID = document["success"][i]["imageId"].GetString();
                        string sSTImageFeature = document["success"][i]["feature"].GetString();
                        string sName = document["success"][i]["name"].GetString();
                        double dbQuality = 0;
                        if (document["success"][i].HasMember("qualityScore") && document["success"][i]["qualityScore"].IsDouble())
                        {
                            dbQuality = document["success"][i]["qualityScore"].GetDouble();
                        }


                        LISTPUSHSTIMAGEINFO::iterator itImage = pStoreImageInfo->listPushSTImageInfo.begin();
                        for (; itImage != pStoreImageInfo->listPushSTImageInfo.end(); itImage++)
                        {
                            if (string((*itImage)->pImageID) == sName)
                            {
                                strcpy_s((*itImage)->pSTImageID, sizeof((*itImage)->pSTImageID), sSTImageID.c_str());
                                strcpy_s((*itImage)->pFeature, sizeof((*itImage)->pFeature), sSTImageFeature.c_str());
                                (*itImage)->nFaceQuality = dbQuality;
                                g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "ST: Name[%s], ImageID[%s], Quality[%d]", 
                                    sName.c_str(), sSTImageID.c_str(), (*itImage)->nFaceQuality);
                                break;
                            }
                        }
                        if (itImage == pStoreImageInfo->listPushSTImageInfo.end())
                        {
                            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: ����������Ϣ�ļ���[%s]������", sName.c_str());
                        }
                    }
                    else
                    {
                        g_LogRecorder.WriteDebugLogEx(__FUNCTION__,
                            "***Waring: �������������Ϣ��ʽ����, success����imageId����feature����name����qualityScore�ֶ�");
                    }
                }
            }

        }
    }

    return nRet;
}
string CGetFeature::GetUUID()
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

int CGetFeature::DetectPicFace(const char * pPicBuff, int nLen, char * pRep, int nRepLen)
{


    return 0;
}
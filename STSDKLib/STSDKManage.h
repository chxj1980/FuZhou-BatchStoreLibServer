#pragma once
#include "STSDKInterface.h"
#include "LogRecorder.h"
#include "STSDKClient.h"
class CSTSDKManage
{
public:
    CSTSDKManage(void);
    ~CSTSDKManage(void);
public:
    int Init(STSERVERINFO STServerInfo[], int nQuantity, int nThread);
    int UnInit();
    int OperationDB(const char * pDBName, int nOperationType, char * pResponseMsg, unsigned int * nMsgLen, int nType);
    int GetAllDBInfo(char * pResponseMsg, unsigned int * nMsgLen, int nType);
    int FaceQuality(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType);
    int SynAddImage(const char * pDBName, const char * pImage, int nLen, int nGetFeature,
        int nQualityThreshold, char * pResponseMsg, unsigned int * nMsgLen, int nType);
    int SynAddMultipleImage(const char * pDBName, STIMAGEINFO STImageInfo[], int nNum, int nGetFeature,
        int nQualityThreshold, char * pResponseMsg, unsigned int * nMsgLen, int nType);
    int BatchGetFeature(STIMAGEINFO STImageInfo[], int nNum, char * pResponseMsg, unsigned int * nMsgLen, int nType);
    int SynAddFeature(const char * pDBName, const char * pFeature, char * pResponseMsg, unsigned int * nMsgLen, int nType);
    int GetImageByID(const char * pImageID, char * pResponseMsg, unsigned int * nMsgLen, int nType);
    int DelImageByID(const char * pDBName, const char * pImageID, char * pResponseMsg, unsigned int * nMsgLen, int nType);
    int SearchImage(const char * pDBName, const char * pImage, int nLen, int nTopNum, double dbScore, char * pResponseMsg, unsigned int * nMsgLen, int nType);
    int Verification(const char * pImage1, int nLen1, const char * pImage2, int nLen2, char * pResponseMsg, unsigned int * nMsgLen, int nType);
    int SearchImageByFeature(const char * pDBName, const char * pFeature, int nLen, int nTopNum,
        double dbScore, char * pResponseMsg, unsigned int * nMsgLen, int nType);
    int GetAttribute(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType);
    int GetFeature(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType);
    int GetDetail(char * pResponseMsg, unsigned int * nMsgLen, int nType);
private:
    CSTSDKClient * GetSTClient();
    void PushClient(CSTSDKClient * pSTClient);
private:
    list<CSTSDKClient *> m_listSTClient;

    CRITICAL_SECTION m_cs;
};


#pragma once

#include "HttpProtocol.h"
#include "LogRecorder.h"
#include "STSDKInterface.h"
#include "SocketManage.h"

class CSTSDKClient
{
public:
    CSTSDKClient(void);
public:
    ~CSTSDKClient(void);
public:
    int Init(STSERVERINFO STServerInfo[], int nQuantity);
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

    string GetBoundary();
public:
    CSocketManage * m_pSocketManage;
private:
    char m_pSTIP[20];
    int m_nSTPort;
    char m_pSTAddr[32];

    static unsigned int m_nBoundaryLow;
    static unsigned int m_nBoundaryHigh;

    char * m_pHttpBody;                     //Http Body
    int m_nHttpBodyLen;                     //Body³¤¶È
    char * m_pHttpMsg;
    int m_nHttpMsgLen;
};

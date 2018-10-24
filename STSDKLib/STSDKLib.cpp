// STSDKLib.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

CSTSDKManage g_STManage;
int STSDK_Init(STSERVERINFO STServerInfo[], int nQuantity, int nThread)
{
    return g_STManage.Init(STServerInfo, nQuantity, nThread);
}

int STSDK_UnInit()
{
    return g_STManage.UnInit();
}

int STSDK_OperationDB(const char * pDBName, int nOperationType, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    return g_STManage.OperationDB(pDBName, nOperationType, pResponseMsg, nMsgLen, nType);
}

int STSDK_GetAllDBInfo(char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    return g_STManage.GetAllDBInfo(pResponseMsg, nMsgLen, nType);
}
int STSDK_FaceQuality(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    return g_STManage.FaceQuality(pImage, nLen, pResponseMsg, nMsgLen, nType);
}
int STSDK_SynAddImage(const char * pDBName, const char * pImage, int nLen, int nGetFeature,
    int nQualityThreshold, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    return g_STManage.SynAddImage(pDBName, pImage, nLen, nGetFeature, nQualityThreshold, pResponseMsg, nMsgLen, nType);
}
int STSDK_SynAddMultipleImage(const char * pDBName, STIMAGEINFO STImageInfo[], int nNum, int nGetFeature,
    int nQualityThreshold, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    return g_STManage.SynAddMultipleImage(pDBName, STImageInfo, nNum, nGetFeature, nQualityThreshold, pResponseMsg, nMsgLen, nType);
}
int STSDK_BatchGetFeature(STIMAGEINFO STImageInfo[], int nNum, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    return g_STManage.BatchGetFeature(STImageInfo, nNum, pResponseMsg, nMsgLen, nType);
}
int STSDK_SynAddFeature(const char * pDBName, const char * pFeature, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    return g_STManage.SynAddFeature(pDBName, pFeature, pResponseMsg, nMsgLen, nType);
}

int STSDK_GetImageByID(const char * pImageID, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    return g_STManage.GetImageByID(pImageID, pResponseMsg, nMsgLen, nType);
}
int STSDK_DelImageByID(const char * pDBName, const char * pImageID, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    return g_STManage.DelImageByID(pDBName, pImageID, pResponseMsg, nMsgLen, nType);
}
int STSDK_SearchImage(const char * pDBName, const char * pImage, int nLen, int nTopNum, double dbScore, 
    char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    return g_STManage.SearchImage(pDBName, pImage, nLen, nTopNum, dbScore, pResponseMsg, nMsgLen, nType);
}
int STSDK_Verification(const char * pImage1, int nLen1, const char * pImage2, int nLen2, 
    char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    return g_STManage.Verification(pImage1, nLen1, pImage2, nLen2, pResponseMsg, nMsgLen, nType);
}
int STSDK_SearchImageByFeature(const char * pDBName, const char * pFeature, int nLen, int nTopNum,
    double dbScore, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    return g_STManage.SearchImageByFeature(pDBName, pFeature, nLen, nTopNum, dbScore, pResponseMsg, nMsgLen, nType);
}
int STSDK_GetAttribute(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    return g_STManage.GetAttribute(pImage, nLen, pResponseMsg, nMsgLen, nType);
}
int STSDK_GetFeature(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    return g_STManage.GetFeature(pImage, nLen, pResponseMsg, nMsgLen, nType);
}
int STSDK_GetDetail(char * pResponseMsg, unsigned int * nMsgLen, int nType)
{
    return g_STManage.GetDetail(pResponseMsg, nMsgLen, nType);
}
#pragma once

#include "DataDefine.h"
#include "ZBase64.h"
#include "STSDKInterface.h"
#define STRESPONSELEN 1024 * 1024 * 1
class CGetFeature
{
public:
    CGetFeature();
    ~CGetFeature();
public:
    int Init(pStoreImageCallback pCallback, void * pUser, STSERVERINFO STServerInfo[], int nQuantity);
    void UnInit();
    //添加入库图片信息到链表等待处理
    int AddFaceImageInfo(LPSTOREFACEINFO pStoreImageInfo);

    int DetectPicFace(const char * pPicBuff, int nLen, char * pRep, int nRepLen);
protected:
    static DWORD WINAPI GetImageFeatureThread(LPVOID lParam);
    void GetImageFeatureAction();
    //创建重点库
    bool CreateStoreLib(char * pSTFeature, char * pSTResponseMsg, unsigned int nSTRespLen);
    //将人脸数据添加到商汤库
    int STAddStoreImage(LPSTOREFACEINFO pStoreImageInfo, char * pSTResponseMsg, unsigned int nSTRespLen);
    //解析ST回应消息内容
    bool GetSTResponseResult(char * pMessage, int nLen);
    //解析回应信息, 获取人脸质量值
    int GetSTImageID(char * pMessage, int nLen, LPSTOREFACEINFO pStoreImageInfo);

    //获取人脸图片唯一UUID
    virtual std::string GetUUID();
protected:
    CRITICAL_SECTION m_cs;  //临界区
    HANDLE m_hStopEvent;    //停止事件

    pStoreImageCallback m_pImageCallback;   //获取特征值完成后回调结果函数
    void * m_pUser;                         //初始化传入数据, 回调时返回

    LISTSTOREFACEINFO m_listStoreImageInfo; //等待获取特征值任务链表

    unsigned int m_nPushNum;    //己入库ST临时库容量, >10000时清空
};


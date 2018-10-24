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
    //������ͼƬ��Ϣ������ȴ�����
    int AddFaceImageInfo(LPSTOREFACEINFO pStoreImageInfo);

    int DetectPicFace(const char * pPicBuff, int nLen, char * pRep, int nRepLen);
protected:
    static DWORD WINAPI GetImageFeatureThread(LPVOID lParam);
    void GetImageFeatureAction();
    //�����ص��
    bool CreateStoreLib(char * pSTFeature, char * pSTResponseMsg, unsigned int nSTRespLen);
    //������������ӵ�������
    int STAddStoreImage(LPSTOREFACEINFO pStoreImageInfo, char * pSTResponseMsg, unsigned int nSTRespLen);
    //����ST��Ӧ��Ϣ����
    bool GetSTResponseResult(char * pMessage, int nLen);
    //������Ӧ��Ϣ, ��ȡ��������ֵ
    int GetSTImageID(char * pMessage, int nLen, LPSTOREFACEINFO pStoreImageInfo);

    //��ȡ����ͼƬΨһUUID
    virtual std::string GetUUID();
protected:
    CRITICAL_SECTION m_cs;  //�ٽ���
    HANDLE m_hStopEvent;    //ֹͣ�¼�

    pStoreImageCallback m_pImageCallback;   //��ȡ����ֵ��ɺ�ص��������
    void * m_pUser;                         //��ʼ����������, �ص�ʱ����

    LISTSTOREFACEINFO m_listStoreImageInfo; //�ȴ���ȡ����ֵ��������

    unsigned int m_nPushNum;    //�����ST��ʱ������, >10000ʱ���
};


#pragma once

#include "ConfigRead.h"
#include "mysql_acl.h"
#include "HttpServerAction.h"   //���տͻ��������ص���Ϣ
#include "ZBase64.h"
#include "AnalyseStore.h"
#include "GetFeature.h"
#include <iostream>
using namespace std;


class CBatchStoreLibServer
{
public:
    CBatchStoreLibServer(void);
public:
    ~CBatchStoreLibServer(void);
public:
    bool StartBatchStoreLibServer();
    bool StopBatchStoreLibServer();

private:
    bool Init();

    //��ʼ������DB
    bool InitDB();
    //�������ݿ��ѯ�ص��������Ϣ
    bool GetStoreLibInfo();
    //��ʼ��HTTP����
    bool InitHttpServer(string sRedisIP, int nRedisPort);
    //��ʼ��ST����
    bool InitGetFeature();
    //��ʼ��������������
    bool InitAnalyse();

    //HTTP�ͻ�������ص�
    static void CALLBACK HttpClientRequestCallback(LPHTTPREQUEST pHttpRequest, void * pUser);
    //����Http�����߳�
    static DWORD WINAPI HTTPRequestParseThread(LPVOID lParam);
    void HTTPRequestParseAction();

    //HTTP��������������ST��
    bool AddStoreLibFace(LPHTTPREQUEST pHttpRequest);
    //HTTP�����ST��ɾ������
    bool DelStoreLibFace(LPHTTPREQUEST pHttpRequest);
    //HTTP�����ST��ɾ���ص��
    bool DelStoreLib(LPHTTPREQUEST pHttpRequest);
    //����ͼƬ�����ش���
    bool SaveFileToDisk(unsigned int nStoreLibID, char * pImageBuf, int nLen, char * pFilePath);
    //�������ӵ�������Ϣ������map
    bool AddStoreFaceToMap(std::string sFaceUUID, std::string sSTImageID, unsigned int nLayoutLibID);
    //�ӱ���mapɾ��������Ϣ
    bool DelStoreFaceToMap(std::string sFaceUUID, unsigned int nLayoutLibID);
    //�ӱ���mapɾ���ص��, bDelLib: true: ɾ���ص��, false: ��ɾ��
    bool DelStoreLibToMap(unsigned int nLayoutLibID, bool bDelLib = true);
    //�ӱ��ش���ɾ�������ص��ͼƬ, bDelDir: true: ɾ���ļ���, false: ��ɾ��
    bool DelLibFaceFromDisk(unsigned int nStoreLibID, bool bDelDir = true);
    //��ȡ������
    int GetErrorMsg(int nError, char * pMsg);
    //��Ӧ�ͻ���
    void SendResponseMsg(SOCKET ClientSocket, int nEvent);
    LPSTOREFACEINFO GetFreeFaceInfo();
    void RecoverFaceInfo(LPSTOREFACEINFO pStoreImageInfo);

    //ST����ص������
    static void CALLBACK ImageInfoCallback(LPSTOREFACEINFO pStoreImageInfo, void * pUser);

private:
     CRITICAL_SECTION m_Httpcs;
     CRITICAL_SECTION m_cs;
     HANDLE m_hStopEvent;           //����ֹͣ�¼�
     CConfigRead * m_pConfigRead;   //�����ļ���ȡ

     char m_pServerIP[20];          //���ط���IP
     int m_nServerPort;             //���ط���Port

     CMysql_acl m_mysqltool;        //MySQL���ݿ����

     MAPSTSERVERINFO m_mapSTServerInfo; //��ȡ����ֵ��������Ϣ
     CGetFeature * m_pGetFeature;   //��ȡ����ֵ��������
     CAnalyseStore * m_pAnalyseStore;//��������ֵ����������

     CHttpServerAction * m_pHttpServer; //HTTP�����������
     LISTHTTPREQUEST m_listHttpRequest; //HTTP������Ϣ����

     MAPSTORELIBINFO m_mapStoreLibInfo;//�ص�⼰����������ͼƬ��Ϣ

     LISTSTOREFACEINFO m_listStoreImageInfo; //�����յ���HTTP��Ϣ��Դ��

     char m_pProxyServerIP[20];
     int m_nProxyPort;
     LISTANALYSESERVERINFO m_listAnalyseServerInfo;

     SYSTEMTIME m_sysTime;
     char m_pSavePath[128];         //ͼƬ����·��(�������ļ���ȡ)
};

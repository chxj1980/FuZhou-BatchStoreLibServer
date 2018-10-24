#pragma once

#include "ConfigRead.h"
#include "mysql_acl.h"
#include "HttpServerAction.h"   //���տͻ������Ͳ�����Ϣ
#include "ZeromqManage.h"
#include "CheckpointSubManage.h"
#include <set>
using namespace std;


typedef struct _AnalyseFeatureLibInfo
{
    set<int> setFeatureInfo;
}ANALYSEFEATURELIBINFO, *LPANALYSEFEATURELIBINFO;
typedef map<string, LPANALYSEFEATURELIBINFO> MAPANALYSEFEATURELIBINFO;

class CLayoutServer
{
public:
    CLayoutServer(void);
public:
    ~CLayoutServer(void);
public:
    bool Init();
    bool StartLayoutServer();
    bool StopLayoutServer();

private:
    bool InitDB();
    //�������ݿ��ѯ���ؿ�����Ϣ
    bool GetLayoutCameraInfo();
    //��ȡ�ӿڷ��񲼿�����
    bool GetLayoutTaskInfo();
    //��ʼ��Http����
    bool InitHttpServer(string sLocalIP, int nLocalPort);
    //��ʼ��Redis����
    bool InitRedisManage(string sRedisIP, int nRedisPort);

    //���Ͳ���ͼƬ��Ϣ�����ղ��ؽ��
    bool InitAnalyseSearch();
    //�������񲼿ؽ���ص� 
    static void CALLBACK AnalyseLayoutResultCallback(LPSUBMESSAGE pSubMessage, void * pUser);

    //��ʼ�����Ŀ���ͼƬ
    bool InitCheckpointSubManage();
    //Zeromq��������ͼƬ��Ϣ�ص�
    static void CALLBACK ImageSubCallback(LPSUBMESSAGE pSubMessage, void * pUser);

    //���ķ������ط�������ֵ���ָ����Ϣ
    bool InitZeroMq();
    //Zeromq�ص�����ֵ�����Ϣ
    static void CALLBACK AnalyseLibInfoCallback(LPSUBMESSAGE pSubMessage, void * pUser);

    //���濨�ڶ��������߳�
    static DWORD WINAPI SubCheckpointThread(LPVOID lParam);
    void SubCheckpointAction();

    //���Ӳ��ؿ���(��ʼ��ʱʹ��) nTaskType: 0: ϵͳ����, 1: �ӿڷ��񲼿���������, 2: �ӿڷ��񲼿��������
    bool AddSubCheckpointInfo(int nLibID, char * pBeginTime, char * pEndtime, char * pCheckpointID, int nScore, int nTaskType = 0);
    //ɾ�����ؿ���
    bool DelLayoutCheckpoint(int nLibID, char * pCheckpointID);
    //���Ӳ�������ͼƬ
    bool AddLayoutImage(int nLibID, char * pBeginTime, char * pEndtime, char * pFaceUUID);
    //ɾ����������ͼƬ
    bool DelLayoutImage(int nLibID, char * pFaceUUID);
    //ֹͣ���ؿ�
    bool StopLayoutLib(int nLibID);
    //����|�޸Ĳ�������
    int AddLayoutTaskInfo(LPLAYOUTTASKINFO pTaskInfo);
    //ɾ����������
    int StopLayoutTask(string sTaskID);

    //HTTP�ͻ�������ص�
    static void CALLBACK HttpClientRequestCallback(LPHTTPREQUEST pHttpRequest, void * pUser);
    //����Http�����߳�
    static DWORD WINAPI HTTPRequestParseThread(LPVOID lParam);
    void HTTPRequestParseAction();
    //��Ӧ�ͻ���
    void SendResponseMsg(SOCKET ClientSocket, int nEvent, string sTaskID = "");
    int GetErrorMsg(int nError, char * pMsg);

    unsigned long long ChangeTimeToSecond(string sTime);
    string ChangeSecondToTime(unsigned long long nSecond);
    string GetUUID();

    bool UpdateTaskInfoToDB(string sTaskID, bool bAdd = true, string sTaskInfo = "", string sPublishURL = "");
private:
    CRITICAL_SECTION m_cs;
    CRITICAL_SECTION m_csHttp;
    CRITICAL_SECTION m_csFeature;
    CRITICAL_SECTION m_csTask;
    HANDLE m_hStopEvent;                //����ֹͣ�¼�
    CConfigRead * m_pConfigRead;        //�����ļ���ȡ

    char m_pServerIP[20];               //���ط���IP
    int m_nServerPort;                  //���ط���Port

    char m_pProxyServerIP[20];          //�����������IP
    int m_nProxyPort;                   //�����������Port
    CZeromqManage * m_pZeromqManage;    //���ķ������������ֵ�����Ϣ
    CCheckpointSubManage * m_pCheckSubManage;   //���Ŀ���ץ��ͼƬ����

    CMysql_acl m_mysqltool;             //MySQL���ݿ����

    char m_pRedisIP[20];                //Redis IP
    int m_nRedisPort;                   //Redis Port
    CRedisManage * m_pRedisManage;      //Redis��������

    CAnalyseSearch * m_pAnalyseSearch;   //�������ط����������

    CHttpServerAction * m_pHttpServer;  //Web HTTP����
    LISTHTTPREQUEST m_listHttpRequest;  //HTTP������Ϣ����

    int m_nImageNum;                    //��ץ�Ļص���ͼƬ����
    int m_nAnalyseSearchNum;            //ƥ���е�ͼƬ����
    SYSTEMTIME m_sysTime;               //ʱ��

    MAPANALYSEFEATURELIBINFO m_mapAnalyseFeatureLibInfo;    //������������ֵ�����Ϣ
    MAPLAYOUTCHECKPOINTINFO m_mapLayoutCheckpointInfo;      //���ؿ��ڼ��������ؿ���Ϣ

    MAPLAYOUTTASKINFO m_mapLayoutTaskInfo;                  //����������Ϣ
    map<string, int> m_mapLayoutFaceUUIDInfo;           //���ؿ��ӦFaceUUID

    bool m_bPrintSnap;      //�Ƿ��ӡץ��ͼƬ��Ϣ
};

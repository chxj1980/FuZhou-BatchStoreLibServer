#pragma once

#include <map>
#include <string>
#include <list>
using namespace std;

#define STANALYSELIB extern "C" _declspec(dllexport)

//�����붨��
enum ANALYSELIB_ErrorCode
{
    ANALYSELIB_INVALIDERROR = 0,
    ANALYSELIB_COMMANDNOTSUPPORT = 1001,   //���֧��
    ANALYSELIB_SOCKETINITFAILED,           //����Socket��ʼ��ʧ��
    ANALYSELIB_OPERATIONNOTFOUND,          //���ݿ�������Ͳ�����
    ANALYSELIB_DISCONNECTSERVER,           //δ���ӷ���, �޷���������
    ANALYSELIB_STRESPOVERTIME,             //�����Ӧ��ʱ
    ANALYSELIB_RESPONSEMSGLENFAILED,       //���ջ�Ӧ��Ϣ�ַ���̫��
    ANALYSELIB_HTTPMSGFORMATWRONG,         //��ӦHTTP��Ϣ��ʽ����
    ANALYSELIB_SOCKETINVALID,              //SOCKETδ��ʼ��
    ANALYSELIB_SOCKETCLOSE,                //Զ������ǿ�ƹر�����
    ANALYSELIB_GETSOCKETFAILED,            //δ��ȡ������Socket
    ANALYSELIB_SENDMSGFAILED,              //������Ϣʧ��
    ANALYSELIB_RECVMSGZERO,                //������Ϣ��Ϊ��
    ANALYSELIB_STJSONFORMATERROR,          //��������Json��ʽ������
    ANALYSELIB_PARAMILLEGAL,               //��������
    ANALYSELIB_NOTINIT,                    //��δ��ʼ��
    ANALYSELIB_SERVERIDNOTEXIST            //��������ID������
};


typedef struct _STAnalyseServerInfo
{
    char pAnalyseServerID[21];  //�������������ID
    char pAnalyseServerIP[20];  //���������������IP
    int nAnalyseServerPort;     //���������������Port
    int nType;                  //�������������������, 1: ץ�ķ�����, 2: �ص�������(��ץ��)
    map<string, int> mapCheckpointInfo; //�����������������Ϣ, nType = 1ʱʹ��
}STANALYSESERVERINFO, *LPSTANALYSESERVERINFO;

typedef struct _STImageFeatureInfo
{
    char * pFaceUUID;       //ͼƬFaceUUID
    char * pImageFeature;   //ͼƬFeature
    int nImageLen;          //ͼƬFeature����
    char * pTime;           //ͼƬץ��ʱ��
    int nScore;             //ͼƬ������������
}STIMAGEFEATUREINFO, *LPSTIMAGEFEATUREINFO;

//������, ʱ����������ֵ��Ϣ
typedef struct _CheckpointSearchInfo
{
    list<string> listCheckpoint;    //�����б�
    char * pFeature;          //����ֵ
    int nFtLen;                     //����ֵ����
    int nTopNum;                    //��󷵻����������
    double dbScore;                 //ƥ����ͷ���
    char pBeginTime[20];            //������ʼʱ��
    char pEndTime[20];              //��������ʱ��
}CHECKPOINTSEARCHINFO, *LPCHECKPOINTSEARCHINFO;

/*************��ʼ��************/
/*����: 
    nThread: ͬ��ִ���߳���
����ֵ: 0�ɹ�, <0ʧ�� */
STANALYSELIB int STAnalyse_Init(int nThread = 8);

/*************�޸ķ���������Ϣ************/
/*����: 
    pSTServerInfo:   ����������Ϣ
    nEditType: 1: ����, 2: ɾ��
����ֵ: 0�ɹ�, <0ʧ�� */
STANALYSELIB int STAnalyse_Edit(LPSTANALYSESERVERINFO pServerInfo, int nEditType = 1);

/*************����ʼ��************/
/*����: ��
����ֵ: 0�ɹ�, <0ʧ�� */
STANALYSELIB int STAnalyse_UnInit();

/*************���ݿ����*************/
/*����:
    pDBName: ������
    nOperationType: ��������, 1: ����, 2: ɾ��, 3: ����Ŀ���
    pResponseMsg: ���������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
    nType: ������������, 1: ץ�ķ�����, 2: �ص�������(��ץ��)
����ֵ: 0�ɹ�, <0ʧ��*/
STANALYSELIB int STAnalyse_OperationDB(const char * pDBName, int nOperationType, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************��ȡ����Ŀ�����Ϣ*************/
/*����:
    pResponseMsg: ���������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STANALYSELIB int STAnalyse_GetAllDBInfo(char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************��������ͼƬ����ֵ���*************/
/*����:
    pDBName: ����
    STImageFeatureInfo: �������ͼƬ����ֵ����
    nNum: �����������ֵ����
    pResponseMsg: ���������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STANALYSELIB int STAnalyse_SynAddMultipleFeature(const char * pDBName, STIMAGEFEATUREINFO STImageFeatureInfo[], int nNum, 
                                                char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************��ָ�����ݿ��и���IDɾ��ͼƬ*************/
/*����:
    pDBName: ����
    FaceUUID: ͼƬFaceUUID
    pResponseMsg: ���������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STANALYSELIB int STAnalyse_DelImageByID(const char * pDBName, const char * pFaceUUID, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************������������, ����ָ������ֵ��ָ����Ŀ����м���Ŀ��*************/
/*����:
    pDBName: ����
    pFeature: ͼƬ����ֵ
    nLen: ͼƬ���ݳ���
    nTopNum: �����������������������������500��
    nScore: �����������ͷ���
    pResponseMsg: ���������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STANALYSELIB int STAnalyse_SearchImageByFeature(const char * pDBName, const char * pFeature, int nLen, int nTopNum, 
    double dbScore, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************���ӹ�������*************/
/*����:
    pServerID:��������ID
    pDeviceID: ����ID
    nPort: ���ڶ�Ӧ�˿�
    pResponseMsg: ���������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STANALYSELIB int STAnalyse_AddCheckpoint(const char * pServerID, const char * pDeviceID, int &nPort, char * pResponseMsg, unsigned int * nMsgLen);

/*************ɾ����������*************/
/*����:
    pServerID:��������ID
    pDeviceID: ����ID
    pResponseMsg: ���������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STANALYSELIB int STAnalyse_DelCheckpoint(const char * pServerID, const char * pDeviceID, char * pResponseMsg, unsigned int * nMsgLen);

/*************�򿨿���������ֵ*************/
/*����:
    pDeviceID: ����ID
    pFeature: ����ֵ��Ϣ
    pResponseMsg: ���������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STANALYSELIB int STAnalyse_CheckpointAddFeature(const char * pDeviceID, LPSTIMAGEFEATUREINFO pFeatureInfo, char * pResponseMsg, unsigned int * nMsgLen);

/*************�ӿ���ɾ������ֵ*************/
/*����:
    pDeviceID: ����ID
    pFaceUUID: ����ֵFaceUUID
    pResponseMsg: ���������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STANALYSELIB int STAnalyse_CheckpointDelFeature(const char * pDeviceID, const char * pFaceUUID, char * pResponseMsg, unsigned int * nMsgLen);

/*************�ӿ�����������ֵ*************/
/*����:
    pCheckpointSearchInfo: ��������ֵ��Ϣ
    pResponseMsg: ���������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STANALYSELIB int STAnalyse_CheckpointSearch(LPCHECKPOINTSEARCHINFO pCheckpointSearchInfo, char * pResponseMsg, unsigned int * nMsgLen);


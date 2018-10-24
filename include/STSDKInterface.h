#pragma once
#define STSDKLIB extern "C" _declspec(dllexport)


//�����붨��
enum STSDK_ErrorCode
{
    STSDK_INVALIDERROR = 0,         //�޴���
    STSDK_SOCKETINITFAILED = 1001,  //����Socket��ʼ��ʧ��
    STSDK_OPERATIONNOTFOUND,        //���ݿ�������Ͳ�����
    STSDK_DISCONNECTSERVER,         //δ���ӷ���, �޷���������
    STSDK_STRESPOVERTIME,           //�����Ӧ��ʱ
    STSDK_RESPONSEMSGLENFAILED,     //���ջ�Ӧ��Ϣ�ַ���̫��
    STSDK_HTTPMSGFORMATWRONG,       //��ӦHTTP��Ϣ��ʽ����
    STSDK_SOCKETINVALID,            //SOCKETδ��ʼ��
    STSDK_SOCKETCLOSE,              //Զ������ǿ�ƹر�����
    STSDK_GETSOCKETFAILED,          //δ��ȡ������Socket
    STSDK_SENDMSGFAILED,            //������Ϣʧ��
    STSDK_RECVMSGZERO,              //������Ϣ��Ϊ��
    STSDK_STJSONFORMATERROR         //��������Json��ʽ������
};


typedef struct _STServerInfo
{
    char pSTServerIP[20];   //����������IP
    int nSTServerPort;      //����������Port
    int nType;              //��������������, 1: ץ�ķ�����, 2: �ص�������(��ץ��)
}STSERVERINFO, *LPSTSERVERINFO;

typedef struct _STImageInfo
{
    char * pImageBuf;   //ͼƬBuf
    int nImageLen;      //ͼƬBuf����
    char pName[1024];     //ͼƬ��
}STIMAGEINFO, *LPSTIMAGEINFO;

/*************��ʼ��************/
/*����: 
    pSTIP:   ����IP
    nSTPort: ����Port
    nThread: ͬ��ִ���߳���
����ֵ: 0�ɹ�, <0ʧ�� */
STSDKLIB int STSDK_Init(STSERVERINFO STServerInfo[], int nQuantity, int nThread = 8);

/*************����ʼ��************/
/*����: ��
����ֵ: 0�ɹ�, <0ʧ�� */
STSDKLIB int STSDK_UnInit();

/*************���ݿ����*************/
/*����:
    pDBName: ���ݿ�����
    nOperationType: ��������, 1: ����, 2: ɾ��, 3: ����Ŀ���
    pResponseMsg: ������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
    nType: ��������������, 1: ץ�ķ�����, 2: �ص�������(��ץ��), 3:��ʱʹ��, �����
����ֵ: 0�ɹ�, <0ʧ��*/
STSDKLIB int STSDK_OperationDB(const char * pDBName, int nOperationType, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************��ȡ����Ŀ�����Ϣ*************/
/*����:
    pResponseMsg: ������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STSDKLIB int STSDK_GetAllDBInfo(char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************����ͼƬ�����������������*************/
/*����:
    pImage: ͼƬ����
    nLen: ͼƬ���ݳ���
    pResponseMsg: ������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
    ����ֵ: 0�ɹ�, <0ʧ��*/
STSDKLIB int STSDK_FaceQuality(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************ͬ������ͼƬ���*************/
/*����:
    pDBName: ���ݿ���
    pImage: ͼƬ����
    nLen: ͼƬ���ݳ���
    nGetFeature: �Ƿ񴫵���������,0Ϊ����������1Ϊ����������
    nQualityThreshold: ������ȡ�����������������
    pResponseMsg: ������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STSDKLIB int STSDK_SynAddImage(const char * pDBName, const char * pImage, int nLen, int nGetFeature,
    int nQualityThreshold, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************��������ͼƬ���*************/
/*����:
    pDBName: ���ݿ���
    STImageInfo: �������ͼƬ����
    nNum: �������ͼƬ����
    nGetFeature: �Ƿ񴫵���������,0Ϊ����������1Ϊ����������
    nQualityThreshold: ������ȡ�����������������
    pResponseMsg: ������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STSDKLIB int STSDK_SynAddMultipleImage(const char * pDBName, STIMAGEINFO STImageInfo[], int nNum, int nGetFeature,
    int nQualityThreshold, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************������ȡ����ͼƬ����ֵ(�����, �޷���ȡ������������)*************/
/*����:
STImageInfo: �������ͼƬ����
nNum: �������ͼƬ����
pResponseMsg: ������Ӧ��Ϣ
nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STSDKLIB int STSDK_BatchGetFeature(STIMAGEINFO STImageInfo[], int nNum, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************ͬ������ͼƬ����ֵ���*************/
/*����:
    pDBName: ���ݿ���
    pImage: ͼƬ����
    nLen: ͼƬ���ݳ���
    nGetFeature: �Ƿ񴫵���������,0Ϊ����������1Ϊ����������
    nQualityThreshold: ������ȡ�����������������
    pResponseMsg: ������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STSDKLIB int STSDK_SynAddFeature(const char * pDBName, const char * pFeature, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************����ID��ȡ����ͼƬ*************/
/*����:
    pImageID: ͼƬID(�������ݿⷵ��ID)
    pResponseMsg: ������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STSDKLIB int STSDK_GetImageByID(const char * pImageID, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************��ָ�����ݿ��и���IDɾ��ͼƬ*************/
/*����:
    pDBName: ���ݿ���
    pImageID: ͼƬID(�������ݿⷵ��ID)
    pResponseMsg: ������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STSDKLIB int STSDK_DelImageByID(const char * pDBName, const char * pImageID, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************����ͼƬ����, ��ָ�����ݿ�����������Ŀ��*************/
/*����:
    pDBName: ���ݿ���
    pImage: ͼƬ����
    nLen: ͼƬ���ݳ���
    nTopNum: �����������������������������500��
    nScore: �����������ͷ���
    pResponseMsg: ������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STSDKLIB int STSDK_SearchImage(const char * pDBName, const char * pImage, int nLen, int nTopNum, 
    double dbScore, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************1:1������֤, ����������ͼƬ���жԱȣ�������ߵ����ưٷֱ�*************/
/*����:
    pImage1: ͼƬ1����
    nLen1: ͼƬ1���ݳ���
    pImage2: ͼƬ2����
    nLen1: ͼƬ2���ݳ���
    pResponseMsg: ������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STSDKLIB int STSDK_Verification(const char * pImage1, int nLen1, const char * pImage2, int nLen2,
    char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************������������, ����ָ������ֵ��ָ����Ŀ����м���Ŀ��*************/
/*����:
    pDBName: ���ݿ���
    pFeature: ͼƬ����ֵ
    nLen: ͼƬ���ݳ���
    nTopNum: �����������������������������500��
    nScore: �����������ͷ���
    pResponseMsg: ������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STSDKLIB int STSDK_SearchImageByFeature(const char * pDBName, const char * pFeature, int nLen, int nTopNum, 
    double dbScore, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************��ȡָ������ͼƬ������*************/
/*����:
    pImage: ͼƬ����
    nLen: ͼƬ���ݳ���
    pResponseMsg: ������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STSDKLIB int STSDK_GetAttribute(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************��ȡ����ͼƬ����ֵFeature*************/
/*����:
    pImage: ͼƬ����
    nLen: ͼƬ���ݳ���
    pResponseMsg: ������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STSDKLIB int STSDK_GetFeature(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************��ȡĿ������ͼƬ������*************/
/*����: 
    pResponseMsg: ������Ӧ��Ϣ
    nMsgLen: ��Ӧ��Ϣ����
����ֵ: 0�ɹ�, <0ʧ��*/
STSDKLIB int STSDK_GetDetail(char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);



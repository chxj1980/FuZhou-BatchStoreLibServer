#pragma once
#include "acl_cpp/lib_acl.hpp"
#include "lib_acl.h"
#include "STSDKInterface.h"
#include "STAnalyseInterface.h"
#include <set>
using namespace std;

#define THREADWAITTIME      10                  //�̵߳ȴ�ʱ��(ms)
#define FACEIMAGELEN        1024 * 1024 * 2     //faceͼƬ���Buf����
#define MAXFACEIMAGEINFO    150                 //ԭʼ��Դ�������
#define HEARTBEATMSG        "heartbeat"         //����

//zeromq����
#define COMMANDADDLIB       "addlib"        //�����ص��
#define COMMANDDELLIB       "dellib"        //ɾ���ص��
#define COMMANDADD          "add"           //��������ֵ����
#define COMMANDDEL          "del"           //ɾ������ֵ����
#define COMMANDCLEAR        "clear"         //�������ֵ������
#define COMMANDSEARCH       "search"        //��������ֵ����
#define COMMANDLAYOUTSEARCH "layoutsearch"  //���ط��񲼿�����
#define LAYOUTLIBADDRESSINFO "layoutlibinfo"//���ط�����������ֵ���ָ����Ϣ����
//Json���ֶζ���
#define JSONLIBINFO     "libinfo"       //�������񷢲�����ֵ�����Ϣ���ؿ���
#define JSONLIBINDEX    "index"         //�ص������ֵ��Ƭ����

#define JSONLAYOUTDATA  "data"          //���ؽ������
#define JSONLAYOUTFACEUUID "layoutfaceuuid" //����ͼƬFaceUUID
#define JSONLIBID      "libid"         //�ص��ID
#define JSONCHECKPOINT  "checkpoint"    //����ID
#define JSONFACEUUID    "faceuuid"      //ץ��ͼƬ����ֵFaceUUID
#define JSONSCORE       "score"         //�ȶԷ���
#define JSONFEATURE     "feature"       //����ֵ
#define JSONFACERECT    "facerect"      //����ͼƬ����
#define JSONFACEURL     "face_url"      //����url
#define JSONBKGURL      "bkg_url"       //����ͼurl
#define JSONTIME        "imagetime"     //ץ��ʱ��
#define JSONDRIVE       "imagedisk"     //ͼƬ�����ڲɼ������ϵ������̷�
#define JSONSERVERIP    "imageip"       //�ɼ�����IP

//Redis Hash���ֶζ���
#define HASHPICTURE     "Picture"
#define HASHFACE        "Face"
#define HASHTIME        "Time"
#define HASHSCORE       "Score"
#define HASHFEATURE     "Feature"

#define SUBSCRIBEPATTERN    "Checkpoints."          //Redis����
#define SQLMAXLEN           1024 * 4                //SQL�����󳤶�

#define MAXLEN      128
#define MAXIPLEN        20          //IP, FaceRect��󳤶�
#define FEATURELEN  4096
#define FEATUREMIXLEN   500         //Feature��̳���, С�ڴ˳��ȶ�Ϊ���Ϸ�
#define THREADNUM  8           //��ȡ����ֵ�����߳���
#define STTESTLIB   "CollectTemp"   //���������������������ʱ����

//���ݿ����
#define LAYOUTCHECKPOINTTABLE   "layoutcheckpoint"  //���ؿ��ڱ�
#define STORELIBTABLE           "storelib"          //�ص���
#define STOREFACEINFOTABLE      "storefaceinfo"     //�ص��ͼƬ��
#define LAYOUTRESULTTABLE       "layoutresult"      //���رȶԽ����
#define SERVERTYPETABLE         "servertype"        //�������ͱ�
#define SERVERINFOTABLE         "serverinfo"        //������Ϣ��

#define FEATUREMAXLEN   1024 * 4

enum ErrorCode      //�����붨��
{
    INVALIDERROR = 0,       //�޴���
    ServerNotInit = 12001,   //������δ��ʼ�����
    DBAleadyExist,          //�⼺����
    DBNotExist,             //�ⲻ����
    FaceUUIDAleadyExist,    //FaceUUID������
    FaceUUIDNotExist,       //FaceUUID������
    ParamIllegal,           //�����Ƿ�
    NewFailed,              //new�����ڴ�ʧ��
    JsonFormatError,        //json����ʽ����
    CommandNotFound,        //��֧�ֵ�����
    HttpMsgUpperLimit,      //http��Ϣ������������������
    PthreadMutexInitFailed, //�ٽ�����ʼ��ʧ��
    FeatureNumOverMax,      //������������ֵ��������
    JsonParamIllegal,       //Json����ֵ�Ƿ�
    MysqlQueryFailed,       //Mysql����ִ��ʧ��.
    VerifyFailed,           //�ȶ�ʧ��
    PushFeatureToAnalyseFailed, //����ֵ���͸���������ʧ��
    STSDKInitFailed,        //ST���ʼ��ʧ��
    STJsonFormatFailed,     //ST���ؽ��Json������ʧ��
    GetSTFeatureFailed,      //����ST�ص��ʧ��
    AddImageToSTFailed,     //����ͼƬ��ST��������ȡ����ֵ, ����errorʧ��
    STRepGetFeatureFailed,  //ST��������ȡ����ͼƬ����ֵʧ��
    STRepFileNameNotExist,  //���ST������, �����ļ���������
    GetLocalPathFailed,     //��ȡ���񱾵�·��ʧ��
    LoadLibFailed,          //���ؿ�ʧ��
    GetProcAddressFailed,   //��ȡ������ַʧ��
    FRSEngineInitFailed,    //��������ֵ��ȡ���ʼ��ʧ��
    ConvertBMPFailed,       //ͼƬתBMPʧ��
    GetLocalFeatureFailed,  //���ػ�ȡͼƬ����ֵʧ��
    NotEnoughResource,      //�޿�����Դ, �Ժ��ϴ�
    GetPictureFailed,       //����ͼƬurl��ַ�޷���ȡ��ͼƬ
    TaskIDNotExist          //����ID������
};

typedef std::map<std::string, LPSTSERVERINFO> MAPSTSERVERINFO;
typedef std::map<std::string, LPSTANALYSESERVERINFO> MAPANALYSESERVERINFO;

//��������
enum TASKTYPE
{
    INVALIDTASK,
    CHECKPOINTADDFEATURE,   //�򿨿���������ֵ
    CHECKPOINTDELFEATURE,   //�ӿ���ɾ������ֵ
    CHECKPOINTCLEARFEATURE, //�ӿ���ɾ������ֵ
    CHECKPOINTSEARCH,       //��ʱ��, ������������ֵ
};

typedef struct _SubMessage
{
    char pHead[MAXLEN];                 //������Ϣͷ
    char pOperationType[MAXLEN];        //������Ϣ��������
    char pSource[MAXLEN];               //������ϢԴ
    char pSubJsonValue[FEATURELEN * 4]; //������ϢJson��
    string sPubJsonValue;     //������ϢJson��
         
    char pDeviceID[MAXLEN];     //����ID
    char pFaceUUID[MAXLEN];     //����ֵFaceUUID
    int  nImageTime;            //ͼƬץ��ʱ��
    char pFeature[FEATURELEN];  //����ֵ
    char pDisk[2];              //ͼƬ�������
    char pImageIP[MAXIPLEN];    //ͼƬ���������IP
    char pFaceRect[MAXIPLEN];   //��������
    char pFaceURL[2048];        //ͼƬ����URL
    char pBkgURL[2048];         //ͼƬ����URL
    _SubMessage()
    {
        memset(pHead, 0, sizeof(pHead));
        memset(pOperationType, 0, sizeof(pOperationType));
        memset(pSource, 0, sizeof(pSource));
        memset(pSubJsonValue, 0, sizeof(pSubJsonValue));
        sPubJsonValue = "";

        memset(pDeviceID, 0, sizeof(pDeviceID));
        memset(pFaceUUID, 0, sizeof(pFaceUUID));
        nImageTime = 0;
        memset(pFeature, 0, sizeof(pFeature));
        memset(pDisk, 0, sizeof(pDisk));
        memset(pImageIP, 0, sizeof(pImageIP));
        memset(pFaceRect, 0, sizeof(pFaceRect));
        memset(pFaceURL, 0, sizeof(pFaceURL));
        memset(pBkgURL, 0, sizeof(pBkgURL));
    }
    void Init()
    {
        memset(pHead, 0, sizeof(pHead));
        memset(pOperationType, 0, sizeof(pOperationType));
        memset(pSource, 0, sizeof(pSource));
        memset(pSubJsonValue, 0, sizeof(pSubJsonValue));
        sPubJsonValue = "";

        memset(pDeviceID, 0, sizeof(pDeviceID));
        memset(pFaceUUID, 0, sizeof(pFaceUUID));
        nImageTime = 0;
        memset(pFeature, 0, sizeof(pFeature));
        memset(pDisk, 0, sizeof(pDisk));
        memset(pImageIP, 0, sizeof(pImageIP));
        memset(pFaceRect, 0, sizeof(pFaceRect));
        memset(pFaceURL, 0, sizeof(pFaceURL));
        memset(pBkgURL, 0, sizeof(pBkgURL));
    }
}SUBMESSAGE, *LPSUBMESSAGE;
typedef std::list<LPSUBMESSAGE> LISTSUBMESSAGE;
typedef void(*LPSubMessageCallback)(LPSUBMESSAGE pSubMessage, void * pUser);

#ifdef LAYOUTSERVER //���ط���
//���ؿ�����Ϣ
typedef std::map<std::string, unsigned int> MAPLAYOUTCHECKPOINT;
typedef struct _LayoutInfo
{
    int nLayoutLibID;       //���ؿ�ID
    char pBeginTime[32];    //��ʼʱ��
    char pEndTime[32];      //����ʱ��
    std::list<std::string> listFaceUUID;  //��Ҫ���ػ�ɾ�����ص�ͼƬFaceUUID
    MAPLAYOUTCHECKPOINT mapCheckpoint;    //���ؿ��ڱ������, һ����Ϣ����Я��������ڱ��
    _LayoutInfo()
    {
        nLayoutLibID = 0;
        ZeroMemory(pBeginTime, sizeof(pBeginTime));
        ZeroMemory(pEndTime, sizeof(pEndTime));
    }
    ~_LayoutInfo()
    {
        listFaceUUID.clear();
        mapCheckpoint.clear();
    }
}LAYOUTINFO, *LPLAYOUTINFO;

//Ϊ��������ID����
typedef struct _TaskCheckpoint
{
    int nLayoutLibID;
    map<string, int> mapTaskCheckpoint;
}TASKCHECKPOINT, *LPTASKCHECKPOINT;

typedef struct _LayoutTaskInfo
{
    char pTaskID[MAXLEN];
    char pPublishURL[1024];
    char pBeginTime[20];
    char pEndTime[20];
    map<int, LPTASKCHECKPOINT> mapTaskLib;
    _LayoutTaskInfo()
    {
        ZeroMemory(pTaskID, sizeof(pTaskID));
        ZeroMemory(pPublishURL, sizeof(pPublishURL));
        ZeroMemory(pBeginTime, sizeof(pBeginTime));
        ZeroMemory(pEndTime, sizeof(pEndTime));
    }
    ~_LayoutTaskInfo()
    {
        while (mapTaskLib.size() > 0)
        {
            delete mapTaskLib.begin()->second;
            mapTaskLib.erase(mapTaskLib.begin());
        }
    }
}LAYOUTTASKINFO, *LPLAYOUTTASKINFO;
typedef std::map<string, LPLAYOUTTASKINFO> MAPLAYOUTTASKINFO;
#endif

#ifdef BATCHSTORELIBSERVER     //�ص������������

//����������Ϣ
typedef struct _AnalyseServerInfo
{
    char pServerID[MAXLEN];
    int nServerType;    //������������, 2: �˲�, 3: ����
}ANALYSESERVERINFO, *LPANALYSESERVERINFO;
typedef list<LPANALYSESERVERINFO> LISTANALYSESERVERINFO;

typedef std::map<std::string, std::string> MAPZBASE64FACE;  //����ͼƬ����(*.jpg), ����ͼƬ��Ϣ(ZBase64�����ʽ)
typedef struct _StoreInfo
{
    int nStoreLibID;                        //�ص��ID
    char pStoreLibName[1024];               //�ص������
    int nLibType;                           //�ص������, 2: �˲�, 3: ����
    std::list<std::string> listFaceUUID;    //ɾ��FaceUUID
    MAPZBASE64FACE mapZBase64FaceInfo;      //����ͼƬ��Ϣ(ZBase64�����ʽ)���ͻ��˴�������nameʵ����ֻ�Ǹ���ţ� ��ʵname�ֶ�ΪImageName��
    MAPZBASE64FACE mapZBaseImageName;       //����ͼƬ���Ʊ���
    _StoreInfo()
    {
        nStoreLibID = 0;
        ZeroMemory(pStoreLibName, sizeof(pStoreLibName));
    }
    ~_StoreInfo()
    {
        listFaceUUID.clear();
        mapZBase64FaceInfo.clear();
    }
}STOREINFO, *LPSTOREINFO;
#endif

typedef struct _HTTPRequest
{
    std::string sHttpHead;  //�ͻ�������Http Head
    std::string sHttpBody;  //�ͻ�������Http Body
    SOCKET ClientSocket;    //�ͻ���Socekt
    int nOperatorType;      //��������, 1: ��������ͼƬ, 2: ɾ������ͼƬ, 3: ���Ӳ��ؿ���, 4: ɾ�����ؿ���, 5: ɾ�����ؿ�, 6: ���߼��ӿ�, �������
#ifdef BATCHSTORELIBSERVER
    LPSTOREINFO pStoreInfo;
    string sDetectPicURL;
#endif
#ifdef LAYOUTSERVER
    LPLAYOUTINFO pLayoutInfo;
    LPLAYOUTTASKINFO pTaskInfo;
    std::string sTaskID;
#endif
    _HTTPRequest()
    {
        sHttpHead = "";
        sHttpBody = "";
        ClientSocket = 0;
#ifdef BATCHSTORELIBSERVER
        pStoreInfo = NULL;
        sDetectPicURL = "";
#endif
#ifdef LAYOUTSERVER
        pLayoutInfo = NULL;
        pTaskInfo = NULL;
        sTaskID = "";
#endif
    }
    ~_HTTPRequest()
    {
#ifdef BATCHSTORELIBSERVER
        if (NULL != pStoreInfo)
        {
            delete pStoreInfo;
            pStoreInfo = NULL;
        }
#endif
#ifdef LAYOUTSERVER
        if (NULL != pLayoutInfo)
        {
            delete pLayoutInfo;
            pLayoutInfo = NULL;
        }
        if (NULL != pTaskInfo)
        {
            delete pTaskInfo;
            pTaskInfo = NULL;
        }
#endif
    }
}HTTPREQUEST, *LPHTTPREQUEST;
typedef std::list<LPHTTPREQUEST> LISTHTTPREQUEST;

#ifdef LAYOUTSERVER

typedef struct _LayoutLibInfo
{
    unsigned int nBeginTime;    //���ؿ�ʼʱ��
    unsigned int nEndTime;      //���ؽ���ʱ��
    unsigned int nSystemScore;  //ϵͳ������ֵ
    unsigned int nTaskScore;    //�ӿڷ��񲼿�������ֵ
    unsigned int nScore;        //���͸��������ط������ֵ
    _LayoutLibInfo()
    {
        nBeginTime = 0;
        nEndTime = 0;
        nScore = 100;
        nSystemScore = 100;
        nTaskScore = 100;
    }
}LAYOUTLIBINFO, *LPLAYOUTLIBINFO;
typedef std::map<unsigned int, LPLAYOUTLIBINFO> MAPLAYOUTLIBINFO;

typedef struct _LayoutCheckpointInfo
{
    bool bLayoutStatus;
    MAPLAYOUTLIBINFO mapLayoutLibInfo;
    _LayoutCheckpointInfo()
    {
        bLayoutStatus = false;
    }
}LAYOUTCHECKPOINTINFO, *LPLAYOUTCHECKPOINTINFO;
typedef std::map<string, LPLAYOUTCHECKPOINTINFO> MAPLAYOUTCHECKPOINTINFO;




#endif

#ifdef BATCHSTORELIBSERVER
typedef struct _StoreLibInfo
{
    unsigned int nStoreLibID;               //�ص��ID
    int nLibType;                           //�ص������, 2: �˲�, 3: ����
    set<string> setStoreFace;              //�����ص��������Ӧ��ϵ(FaceUUID, ImageID)
    _StoreLibInfo()
    {
        nStoreLibID = 0;
        setStoreFace.clear();
    }
    ~_StoreLibInfo()
    {
        setStoreFace.clear();
    }
}STORELIBINFO, *LPSTORELIBINFO;
typedef std::map<unsigned int, LPSTORELIBINFO> MAPSTORELIBINFO;

typedef struct _PushSTImageInfo
{
    char pImageID[1024];    //ͼƬID
    char pImageName[1024];  //ͼƬ��
    char pFaceUUID[64];     //����ͼƬΨһUUID
    char * pImageBuf;       //ͼƬ��Ϣbuf
    int nImageBufMaxLen;    //ͼƬ��ϢBuf��󳤶�
    int nImageLen;          //ͼƬ��С
    char pSTImageID[64];    //��������ⷵ�ص�ͼƬID
    char pFeature[FEATUREMAXLEN]; //����������ⷵ�ص���������
    int nFaceQuality;       //�������񷵻ص���������, ���ʧ��ʱ���������
    bool bUsed;             //��ǰ�Ƿ񼺱�ʹ��
    char pFilePath[1024];    //ͼƬ�������λ��
    char pFaceURL[1024];    //ͼƬURL��ַ
    char pErrorMsg[64];
    _PushSTImageInfo()
    {
        ZeroMemory(pImageID, sizeof(pImageID));
        ZeroMemory(pImageName, sizeof(pImageName));
        ZeroMemory(pFaceUUID, sizeof(pFaceUUID));
        ZeroMemory(pSTImageID, sizeof(pSTImageID));
        ZeroMemory(pFeature, sizeof(pFeature));
        ZeroMemory(pFilePath, sizeof(pFilePath));
        ZeroMemory(pFaceURL, sizeof(pFaceURL));
        ZeroMemory(pErrorMsg, sizeof(pErrorMsg));

        pImageBuf = new char[FACEIMAGELEN];
        nImageBufMaxLen = FACEIMAGELEN;
        nImageLen = 0;
        nFaceQuality = 0;
        bUsed = false;
    }
    void Init()
    {
        ZeroMemory(pImageID, sizeof(pImageID));
        ZeroMemory(pImageName, sizeof(pImageName));
        ZeroMemory(pFaceUUID, sizeof(pFaceUUID));
        ZeroMemory(pSTImageID, sizeof(pSTImageID));
        ZeroMemory(pFeature, sizeof(pFeature));
        ZeroMemory(pFilePath, sizeof(pFilePath));
        ZeroMemory(pFaceURL, sizeof(pFaceURL));
        ZeroMemory(pErrorMsg, sizeof(pErrorMsg));
        nImageLen = 0;
        nFaceQuality = 0;
        bUsed = false;
    }
    ~_PushSTImageInfo()
    {
        delete []pImageBuf;
    }
}PUSHSTIMAGEINFO, *LPPUSHSTIMAGEINFO;
typedef std::list<LPPUSHSTIMAGEINFO> LISTPUSHSTIMAGEINFO;

typedef struct _StoreFaceInfo
{
    unsigned int nStoreLibID;   //�ص��ID
    char pStoreLibName[1024];   //�ص������
    int nLibType;               //�ص������, 2: �˲�, 3: ����
    SOCKET ClientSocket;        //�ͻ���Socekt
    int nEvent;                 //�����������ص����, 0�ɹ�, != 0 ʧ��
    unsigned int nSuccessNum;   //�ɹ��������
    LISTPUSHSTIMAGEINFO listPushSTImageInfo;
    _StoreFaceInfo()
    {
        ClientSocket = INVALID_SOCKET;
        nSuccessNum = 0;
        nStoreLibID = 0;
        ZeroMemory(pStoreLibName, sizeof(pStoreLibName));
        nEvent = 0;
    }
    void Init()
    {
        ClientSocket = INVALID_SOCKET;
        nSuccessNum = 0;
        nStoreLibID = 0;
        ZeroMemory(pStoreLibName, sizeof(pStoreLibName));
        nEvent = 0;
        for(LISTPUSHSTIMAGEINFO::iterator it = listPushSTImageInfo.begin();
            it != listPushSTImageInfo.end(); it ++)
        {
            (*it)->Init();
        }
    }
    ~_StoreFaceInfo()
    {
        while(listPushSTImageInfo.size() > 0)
        {
            delete listPushSTImageInfo.front();
            listPushSTImageInfo.pop_front();
        }
    }
}STOREFACEINFO, *LPSTOREFACEINFO;
typedef std::list<LPSTOREFACEINFO> LISTSTOREFACEINFO;
typedef void (CALLBACK *pStoreImageCallback)(LPSTOREFACEINFO pStoreImageInfo, void * pUser);

#endif

#pragma once
#include "acl_cpp/lib_acl.hpp"
#include "lib_acl.h"
#include "STSDKInterface.h"
#include "STAnalyseInterface.h"
#include <set>
using namespace std;

#define THREADWAITTIME      10                  //线程等待时间(ms)
#define FACEIMAGELEN        1024 * 1024 * 2     //face图片最大Buf长度
#define MAXFACEIMAGEINFO    150                 //原始资源池最大数
#define HEARTBEATMSG        "heartbeat"         //心跳

//zeromq命令
#define COMMANDADDLIB       "addlib"        //增加重点库
#define COMMANDDELLIB       "dellib"        //删除重点库
#define COMMANDADD          "add"           //增加特征值命令
#define COMMANDDEL          "del"           //删除特征值命令
#define COMMANDCLEAR        "clear"         //清空特征值库命令
#define COMMANDSEARCH       "search"        //搜索特征值命令
#define COMMANDLAYOUTSEARCH "layoutsearch"  //布控服务布控命令
#define LAYOUTLIBADDRESSINFO "layoutlibinfo"//布控分析服务特征值结点指针信息订阅
//Json串字段定义
#define JSONLIBINFO     "libinfo"       //分析服务发布特征值结点信息布控库名
#define JSONLIBINDEX    "index"         //重点库特征值分片索引

#define JSONLAYOUTDATA  "data"          //布控结果数组
#define JSONLAYOUTFACEUUID "layoutfaceuuid" //布控图片FaceUUID
#define JSONLIBID      "libid"         //重点库ID
#define JSONCHECKPOINT  "checkpoint"    //卡口ID
#define JSONFACEUUID    "faceuuid"      //抓拍图片特征值FaceUUID
#define JSONSCORE       "score"         //比对分数
#define JSONFEATURE     "feature"       //特征值
#define JSONFACERECT    "facerect"      //人脸图片坐标
#define JSONFACEURL     "face_url"      //人脸url
#define JSONBKGURL      "bkg_url"       //背景图url
#define JSONTIME        "imagetime"     //抓拍时间
#define JSONDRIVE       "imagedisk"     //图片保存在采集服务上的驱动盘符
#define JSONSERVERIP    "imageip"       //采集服务IP

//Redis Hash表字段定义
#define HASHPICTURE     "Picture"
#define HASHFACE        "Face"
#define HASHTIME        "Time"
#define HASHSCORE       "Score"
#define HASHFEATURE     "Feature"

#define SUBSCRIBEPATTERN    "Checkpoints."          //Redis订阅
#define SQLMAXLEN           1024 * 4                //SQL语句最大长度

#define MAXLEN      128
#define MAXIPLEN        20          //IP, FaceRect最大长度
#define FEATURELEN  4096
#define FEATUREMIXLEN   500         //Feature最短长度, 小于此长度定为不合法
#define THREADNUM  8           //获取特征值处理线程数
#define STTESTLIB   "CollectTemp"   //批量入库服务入分析服务临时库名

//数据库表名
#define LAYOUTCHECKPOINTTABLE   "layoutcheckpoint"  //布控卡口表
#define STORELIBTABLE           "storelib"          //重点库表
#define STOREFACEINFOTABLE      "storefaceinfo"     //重点库图片表
#define LAYOUTRESULTTABLE       "layoutresult"      //布控比对结果表
#define SERVERTYPETABLE         "servertype"        //服务类型表
#define SERVERINFOTABLE         "serverinfo"        //服务信息表

#define FEATUREMAXLEN   1024 * 4

enum ErrorCode      //错误码定义
{
    INVALIDERROR = 0,       //无错误
    ServerNotInit = 12001,   //服务尚未初始化完成
    DBAleadyExist,          //库己存在
    DBNotExist,             //库不存在
    FaceUUIDAleadyExist,    //FaceUUID己存在
    FaceUUIDNotExist,       //FaceUUID不存在
    ParamIllegal,           //参数非法
    NewFailed,              //new申请内存失败
    JsonFormatError,        //json串格式错误
    CommandNotFound,        //不支持的命令
    HttpMsgUpperLimit,      //http消息待处理数量己达上限
    PthreadMutexInitFailed, //临界区初始化失败
    FeatureNumOverMax,      //批量增加特征值数量超标
    JsonParamIllegal,       //Json串有值非法
    MysqlQueryFailed,       //Mysql操作执行失败.
    VerifyFailed,           //比对失败
    PushFeatureToAnalyseFailed, //特征值推送给分析服务失败
    STSDKInitFailed,        //ST库初始化失败
    STJsonFormatFailed,     //ST返回结果Json串解析失败
    GetSTFeatureFailed,      //创建ST重点库失败
    AddImageToSTFailed,     //推送图片给ST服务器获取特征值, 返回error失败
    STRepGetFeatureFailed,  //ST服务器获取单张图片特征值失败
    STRepFileNameNotExist,  //入库ST服务器, 返回文件名不存在
    GetLocalPathFailed,     //获取服务本地路径失败
    LoadLibFailed,          //加载库失败
    GetProcAddressFailed,   //获取函数地址失败
    FRSEngineInitFailed,    //本地特征值获取库初始化失败
    ConvertBMPFailed,       //图片转BMP失败
    GetLocalFeatureFailed,  //本地获取图片特征值失败
    NotEnoughResource,      //无可用资源, 稍候上传
    GetPictureFailed,       //给定图片url地址无法获取到图片
    TaskIDNotExist          //任务ID不存在
};

typedef std::map<std::string, LPSTSERVERINFO> MAPSTSERVERINFO;
typedef std::map<std::string, LPSTANALYSESERVERINFO> MAPANALYSESERVERINFO;

//任务类型
enum TASKTYPE
{
    INVALIDTASK,
    CHECKPOINTADDFEATURE,   //向卡口增加特征值
    CHECKPOINTDELFEATURE,   //从卡口删除特征值
    CHECKPOINTCLEARFEATURE, //从卡口删除特征值
    CHECKPOINTSEARCH,       //按时间, 卡口搜索特征值
};

typedef struct _SubMessage
{
    char pHead[MAXLEN];                 //订阅消息头
    char pOperationType[MAXLEN];        //订阅消息操作类型
    char pSource[MAXLEN];               //订阅消息源
    char pSubJsonValue[FEATURELEN * 4]; //订阅消息Json串
    string sPubJsonValue;     //发布消息Json串
         
    char pDeviceID[MAXLEN];     //卡口ID
    char pFaceUUID[MAXLEN];     //特征值FaceUUID
    int  nImageTime;            //图片抓拍时间
    char pFeature[FEATURELEN];  //特征值
    char pDisk[2];              //图片保存磁盘
    char pImageIP[MAXIPLEN];    //图片保存服务器IP
    char pFaceRect[MAXIPLEN];   //人脸坐标
    char pFaceURL[2048];        //图片人脸URL
    char pBkgURL[2048];         //图片背景URL
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

#ifdef LAYOUTSERVER //布控服务
//布控卡口信息
typedef std::map<std::string, unsigned int> MAPLAYOUTCHECKPOINT;
typedef struct _LayoutInfo
{
    int nLayoutLibID;       //布控库ID
    char pBeginTime[32];    //开始时间
    char pEndTime[32];      //结束时间
    std::list<std::string> listFaceUUID;  //需要布控或删除布控的图片FaceUUID
    MAPLAYOUTCHECKPOINT mapCheckpoint;    //布控卡口编号链表, 一次消息可能携带多个卡口编号
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

//为布控任务ID增加
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

#ifdef BATCHSTORELIBSERVER     //重点库批量入库服务

//分析服务信息
typedef struct _AnalyseServerInfo
{
    char pServerID[MAXLEN];
    int nServerType;    //分析服务类型, 2: 核查, 3: 布控
}ANALYSESERVERINFO, *LPANALYSESERVERINFO;
typedef list<LPANALYSESERVERINFO> LISTANALYSESERVERINFO;

typedef std::map<std::string, std::string> MAPZBASE64FACE;  //人脸图片名称(*.jpg), 人脸图片信息(ZBase64编码格式)
typedef struct _StoreInfo
{
    int nStoreLibID;                        //重点库ID
    char pStoreLibName[1024];               //重点库名称
    int nLibType;                           //重点库类型, 2: 核查, 3: 布控
    std::list<std::string> listFaceUUID;    //删除FaceUUID
    MAPZBASE64FACE mapZBase64FaceInfo;      //人脸图片信息(ZBase64编码格式)（客户端传过来的name实际上只是个编号， 真实name字段为ImageName）
    MAPZBASE64FACE mapZBaseImageName;       //人脸图片名称保存
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
    std::string sHttpHead;  //客户端请求Http Head
    std::string sHttpBody;  //客户端请求Http Body
    SOCKET ClientSocket;    //客户端Socekt
    int nOperatorType;      //请求类型, 1: 增加人脸图片, 2: 删除人脸图片, 3: 增加布控卡口, 4: 删除布控卡口, 5: 删除布控库, 6: 工具集接口, 检测人脸
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
    unsigned int nBeginTime;    //布控开始时间
    unsigned int nEndTime;      //布控结束时间
    unsigned int nSystemScore;  //系统布控阈值
    unsigned int nTaskScore;    //接口服务布控任务阈值
    unsigned int nScore;        //发送给分析布控服务的阈值
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
    unsigned int nStoreLibID;               //重点库ID
    int nLibType;                           //重点库类型, 2: 核查, 3: 布控
    set<string> setStoreFace;              //己入重点库人脸对应关系(FaceUUID, ImageID)
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
    char pImageID[1024];    //图片ID
    char pImageName[1024];  //图片名
    char pFaceUUID[64];     //人脸图片唯一UUID
    char * pImageBuf;       //图片信息buf
    int nImageBufMaxLen;    //图片信息Buf最大长度
    int nImageLen;          //图片大小
    char pSTImageID[64];    //分析服务库返回的图片ID
    char pFeature[FEATUREMAXLEN]; //分析服务入库返回的人脸特征
    int nFaceQuality;       //分析服务返回的人脸分数, 入库失败时保存错误码
    bool bUsed;             //当前是否己被使用
    char pFilePath[1024];    //图片保存磁盘位置
    char pFaceURL[1024];    //图片URL地址
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
    unsigned int nStoreLibID;   //重点库ID
    char pStoreLibName[1024];   //重点库名称
    int nLibType;               //重点库类型, 2: 核查, 3: 布控
    SOCKET ClientSocket;        //客户端Socekt
    int nEvent;                 //分析服务入重点库结果, 0成功, != 0 失败
    unsigned int nSuccessNum;   //成功入库数量
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

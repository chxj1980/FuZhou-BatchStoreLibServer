#pragma once

#include <map>
#include <string>
#include <list>
using namespace std;

#define STANALYSELIB extern "C" _declspec(dllexport)

//错误码定义
enum ANALYSELIB_ErrorCode
{
    ANALYSELIB_INVALIDERROR = 0,
    ANALYSELIB_COMMANDNOTSUPPORT = 1001,   //命令不支持
    ANALYSELIB_SOCKETINITFAILED,           //本地Socket初始化失败
    ANALYSELIB_OPERATIONNOTFOUND,          //数据库操作类型不存在
    ANALYSELIB_DISCONNECTSERVER,           //未连接服务, 无法发送数据
    ANALYSELIB_STRESPOVERTIME,             //服务回应超时
    ANALYSELIB_RESPONSEMSGLENFAILED,       //接收回应消息字符串太短
    ANALYSELIB_HTTPMSGFORMATWRONG,         //回应HTTP消息格式错误
    ANALYSELIB_SOCKETINVALID,              //SOCKET未初始化
    ANALYSELIB_SOCKETCLOSE,                //远程主机强制关闭连接
    ANALYSELIB_GETSOCKETFAILED,            //未获取到可用Socket
    ANALYSELIB_SENDMSGFAILED,              //发送消息失败
    ANALYSELIB_RECVMSGZERO,                //接收消息串为空
    ANALYSELIB_STJSONFORMATERROR,          //商汤返回Json格式串错误
    ANALYSELIB_PARAMILLEGAL,               //参数错误
    ANALYSELIB_NOTINIT,                    //尚未初始化
    ANALYSELIB_SERVERIDNOTEXIST            //分析服务ID不存在
};


typedef struct _STAnalyseServerInfo
{
    char pAnalyseServerID[21];  //商汤库分析服务ID
    char pAnalyseServerIP[20];  //商汤库分析服务器IP
    int nAnalyseServerPort;     //商汤库分析服务器Port
    int nType;                  //商汤库分析服务器类型, 1: 抓拍服务器, 2: 重点库服务器(非抓拍)
    map<string, int> mapCheckpointInfo; //分析服务关联卡口信息, nType = 1时使用
}STANALYSESERVERINFO, *LPSTANALYSESERVERINFO;

typedef struct _STImageFeatureInfo
{
    char * pFaceUUID;       //图片FaceUUID
    char * pImageFeature;   //图片Feature
    int nImageLen;          //图片Feature长度
    char * pTime;           //图片抓拍时间
    int nScore;             //图片人脸质量分数
}STIMAGEFEATUREINFO, *LPSTIMAGEFEATUREINFO;

//按卡口, 时间搜索特征值信息
typedef struct _CheckpointSearchInfo
{
    list<string> listCheckpoint;    //卡口列表
    char * pFeature;          //特征值
    int nFtLen;                     //特征值长度
    int nTopNum;                    //最大返回搜索结果数
    double dbScore;                 //匹配最低分数
    char pBeginTime[20];            //搜索开始时间
    char pEndTime[20];              //搜索结束时间
}CHECKPOINTSEARCHINFO, *LPCHECKPOINTSEARCHINFO;

/*************初始化************/
/*参数: 
    nThread: 同步执行线程数
返回值: 0成功, <0失败 */
STANALYSELIB int STAnalyse_Init(int nThread = 8);

/*************修改分析服务信息************/
/*参数: 
    pSTServerInfo:   分析服务信息
    nEditType: 1: 增加, 2: 删除
返回值: 0成功, <0失败 */
STANALYSELIB int STAnalyse_Edit(LPSTANALYSESERVERINFO pServerInfo, int nEditType = 1);

/*************反初始化************/
/*参数: 无
返回值: 0成功, <0失败 */
STANALYSELIB int STAnalyse_UnInit();

/*************数据库操作*************/
/*参数:
    pDBName: 库名称
    nOperationType: 操作类型, 1: 增加, 2: 删除, 3: 清理目标库
    pResponseMsg: 分析服务回应消息
    nMsgLen: 回应消息长度
    nType: 分析服务类型, 1: 抓拍服务器, 2: 重点库服务器(非抓拍)
返回值: 0成功, <0失败*/
STANALYSELIB int STAnalyse_OperationDB(const char * pDBName, int nOperationType, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************获取所有目标库信息*************/
/*参数:
    pResponseMsg: 分析服务回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STANALYSELIB int STAnalyse_GetAllDBInfo(char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************批量人脸图片特征值入库*************/
/*参数:
    pDBName: 库名
    STImageFeatureInfo: 批量入库图片特征值数据
    nNum: 批量入库特征值数量
    pResponseMsg: 分析服务回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STANALYSELIB int STAnalyse_SynAddMultipleFeature(const char * pDBName, STIMAGEFEATUREINFO STImageFeatureInfo[], int nNum, 
                                                char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************在指定数据库中根据ID删除图片*************/
/*参数:
    pDBName: 库名
    FaceUUID: 图片FaceUUID
    pResponseMsg: 分析服务回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STANALYSELIB int STAnalyse_DelImageByID(const char * pDBName, const char * pFaceUUID, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************人脸特征搜索, 根据指定特征值在指定的目标库中检索目标*************/
/*参数:
    pDBName: 库名
    pFeature: 图片特征值
    nLen: 图片数据长度
    nTopNum: 返回搜索结果的最大数量（最大上限500）
    nScore: 搜索结果的最低分数
    pResponseMsg: 分析服务回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STANALYSELIB int STAnalyse_SearchImageByFeature(const char * pDBName, const char * pFeature, int nLen, int nTopNum, 
    double dbScore, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************增加关联卡口*************/
/*参数:
    pServerID:分析服务ID
    pDeviceID: 卡口ID
    nPort: 卡口对应端口
    pResponseMsg: 分析服务回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STANALYSELIB int STAnalyse_AddCheckpoint(const char * pServerID, const char * pDeviceID, int &nPort, char * pResponseMsg, unsigned int * nMsgLen);

/*************删除关联卡口*************/
/*参数:
    pServerID:分析服务ID
    pDeviceID: 卡口ID
    pResponseMsg: 分析服务回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STANALYSELIB int STAnalyse_DelCheckpoint(const char * pServerID, const char * pDeviceID, char * pResponseMsg, unsigned int * nMsgLen);

/*************向卡口增加特征值*************/
/*参数:
    pDeviceID: 卡口ID
    pFeature: 特征值信息
    pResponseMsg: 分析服务回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STANALYSELIB int STAnalyse_CheckpointAddFeature(const char * pDeviceID, LPSTIMAGEFEATUREINFO pFeatureInfo, char * pResponseMsg, unsigned int * nMsgLen);

/*************从卡口删除特征值*************/
/*参数:
    pDeviceID: 卡口ID
    pFaceUUID: 特征值FaceUUID
    pResponseMsg: 分析服务回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STANALYSELIB int STAnalyse_CheckpointDelFeature(const char * pDeviceID, const char * pFaceUUID, char * pResponseMsg, unsigned int * nMsgLen);

/*************从卡口搜索特征值*************/
/*参数:
    pCheckpointSearchInfo: 搜索特征值信息
    pResponseMsg: 分析服务回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STANALYSELIB int STAnalyse_CheckpointSearch(LPCHECKPOINTSEARCHINFO pCheckpointSearchInfo, char * pResponseMsg, unsigned int * nMsgLen);


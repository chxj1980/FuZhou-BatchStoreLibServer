#pragma once
#define STSDKLIB extern "C" _declspec(dllexport)


//错误码定义
enum STSDK_ErrorCode
{
    STSDK_INVALIDERROR = 0,         //无错误
    STSDK_SOCKETINITFAILED = 1001,  //本地Socket初始化失败
    STSDK_OPERATIONNOTFOUND,        //数据库操作类型不存在
    STSDK_DISCONNECTSERVER,         //未连接服务, 无法发送数据
    STSDK_STRESPOVERTIME,           //服务回应超时
    STSDK_RESPONSEMSGLENFAILED,     //接收回应消息字符串太短
    STSDK_HTTPMSGFORMATWRONG,       //回应HTTP消息格式错误
    STSDK_SOCKETINVALID,            //SOCKET未初始化
    STSDK_SOCKETCLOSE,              //远程主机强制关闭连接
    STSDK_GETSOCKETFAILED,          //未获取到可用Socket
    STSDK_SENDMSGFAILED,            //发送消息失败
    STSDK_RECVMSGZERO,              //接收消息串为空
    STSDK_STJSONFORMATERROR         //商汤返回Json格式串错误
};


typedef struct _STServerInfo
{
    char pSTServerIP[20];   //商汤服务器IP
    int nSTServerPort;      //商汤服务器Port
    int nType;              //商汤服务器类型, 1: 抓拍服务器, 2: 重点库服务器(非抓拍)
}STSERVERINFO, *LPSTSERVERINFO;

typedef struct _STImageInfo
{
    char * pImageBuf;   //图片Buf
    int nImageLen;      //图片Buf长度
    char pName[1024];     //图片名
}STIMAGEINFO, *LPSTIMAGEINFO;

/*************初始化************/
/*参数: 
    pSTIP:   商汤IP
    nSTPort: 商汤Port
    nThread: 同步执行线程数
返回值: 0成功, <0失败 */
STSDKLIB int STSDK_Init(STSERVERINFO STServerInfo[], int nQuantity, int nThread = 8);

/*************反初始化************/
/*参数: 无
返回值: 0成功, <0失败 */
STSDKLIB int STSDK_UnInit();

/*************数据库操作*************/
/*参数:
    pDBName: 数据库名称
    nOperationType: 操作类型, 1: 增加, 2: 删除, 3: 清理目标库
    pResponseMsg: 商汤回应消息
    nMsgLen: 回应消息长度
    nType: 商汤服务器类型, 1: 抓拍服务器, 2: 重点库服务器(非抓拍), 3:临时使用, 不入库
返回值: 0成功, <0失败*/
STSDKLIB int STSDK_OperationDB(const char * pDBName, int nOperationType, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************获取所有目标库信息*************/
/*参数:
    pResponseMsg: 商汤回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STSDKLIB int STSDK_GetAllDBInfo(char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************人脸图片检测人脸与质量评分*************/
/*参数:
    pImage: 图片数据
    nLen: 图片数据长度
    pResponseMsg: 商汤回应消息
    nMsgLen: 回应消息长度
    返回值: 0成功, <0失败*/
STSDKLIB int STSDK_FaceQuality(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************同步人脸图片入库*************/
/*参数:
    pDBName: 数据库名
    pImage: 图片数据
    nLen: 图片数据长度
    nGetFeature: 是否传递特征开关,0为传递特征，1为不传递特征
    nQualityThreshold: 设置提取人脸的最低质量分数
    pResponseMsg: 商汤回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STSDKLIB int STSDK_SynAddImage(const char * pDBName, const char * pImage, int nLen, int nGetFeature,
    int nQualityThreshold, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************批量人脸图片入库*************/
/*参数:
    pDBName: 数据库名
    STImageInfo: 批量入库图片数据
    nNum: 批量入库图片数量
    nGetFeature: 是否传递特征开关,0为传递特征，1为不传递特征
    nQualityThreshold: 设置提取人脸的最低质量分数
    pResponseMsg: 商汤回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STSDKLIB int STSDK_SynAddMultipleImage(const char * pDBName, STIMAGEINFO STImageInfo[], int nNum, int nGetFeature,
    int nQualityThreshold, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************批量获取人脸图片特征值(不入库, 无法获取到脸质量分数)*************/
/*参数:
STImageInfo: 批量入库图片数据
nNum: 批量入库图片数量
pResponseMsg: 商汤回应消息
nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STSDKLIB int STSDK_BatchGetFeature(STIMAGEINFO STImageInfo[], int nNum, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************同步人脸图片特征值入库*************/
/*参数:
    pDBName: 数据库名
    pImage: 图片数据
    nLen: 图片数据长度
    nGetFeature: 是否传递特征开关,0为传递特征，1为不传递特征
    nQualityThreshold: 设置提取人脸的最低质量分数
    pResponseMsg: 商汤回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STSDKLIB int STSDK_SynAddFeature(const char * pDBName, const char * pFeature, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************根据ID获取人脸图片*************/
/*参数:
    pImageID: 图片ID(商汤数据库返回ID)
    pResponseMsg: 商汤回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STSDKLIB int STSDK_GetImageByID(const char * pImageID, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************在指定数据库中根据ID删除图片*************/
/*参数:
    pDBName: 数据库名
    pImageID: 图片ID(商汤数据库返回ID)
    pResponseMsg: 商汤回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STSDKLIB int STSDK_DelImageByID(const char * pDBName, const char * pImageID, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************人脸图片搜索, 在指定数据库中搜索相似目标*************/
/*参数:
    pDBName: 数据库名
    pImage: 图片数据
    nLen: 图片数据长度
    nTopNum: 返回搜索结果的最大数量（最大上限500）
    nScore: 搜索结果的最低分数
    pResponseMsg: 商汤回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STSDKLIB int STSDK_SearchImage(const char * pDBName, const char * pImage, int nLen, int nTopNum, 
    double dbScore, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************1:1人脸验证, 将两张人脸图片进行对比，输出两者的相似百分比*************/
/*参数:
    pImage1: 图片1数据
    nLen1: 图片1数据长度
    pImage2: 图片2数据
    nLen1: 图片2数据长度
    pResponseMsg: 商汤回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STSDKLIB int STSDK_Verification(const char * pImage1, int nLen1, const char * pImage2, int nLen2,
    char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************人脸特征搜索, 根据指定特征值在指定的目标库中检索目标*************/
/*参数:
    pDBName: 数据库名
    pFeature: 图片特征值
    nLen: 图片数据长度
    nTopNum: 返回搜索结果的最大数量（最大上限500）
    nScore: 搜索结果的最低分数
    pResponseMsg: 商汤回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STSDKLIB int STSDK_SearchImageByFeature(const char * pDBName, const char * pFeature, int nLen, int nTopNum, 
    double dbScore, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************提取指定人脸图片的属性*************/
/*参数:
    pImage: 图片数据
    nLen: 图片数据长度
    pResponseMsg: 商汤回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STSDKLIB int STSDK_GetAttribute(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************提取人脸图片特征值Feature*************/
/*参数:
    pImage: 图片数据
    nLen: 图片数据长度
    pResponseMsg: 商汤回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STSDKLIB int STSDK_GetFeature(const char * pImage, int nLen, char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);

/*************获取目标库最大图片总容量*************/
/*参数: 
    pResponseMsg: 商汤回应消息
    nMsgLen: 回应消息长度
返回值: 0成功, <0失败*/
STSDKLIB int STSDK_GetDetail(char * pResponseMsg, unsigned int * nMsgLen, int nType = 1);



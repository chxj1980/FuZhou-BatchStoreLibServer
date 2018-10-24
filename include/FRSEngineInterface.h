#ifndef FRSEngineInterface_h__
#define FRSEngineInterface_h__


#ifndef FRSENGINE_IMPORTS
#define FRSENGINE extern "C" __declspec(dllexport)
#else
#define FRSENGINE extern "C" __declspec(dllimport)
#endif

#define MAXFEATURESIZE 2596


enum FRSResultCode
{
	FRSResult_Ok = 0,                    //成功
	FRSResult_GDIPlusInitFail = -10,     //GDI+初始化失败
	FRSResult_FRSLibInitFail,            //FRS库初始化失败
	FRSResult_FeatureBufTooSmall,        //存放特征值数组大小小于2596
	FRSResult_NotBmpFormat,              //非Bmp格式图片
	FRSResult_ConvertImageFail,          //转换图片格式失败
	FRSResult_FRSGetFeatureFail,         //FRS获取特征值失败
	FRSResult_FRSGetQualityFail,         //FRS获取质量失败
	FRSResult_FRSMatchFeatureFail        //FRS特征值比对失败
};

/*************************************************
    Method : FRSInitialize
	@Others: 初始化
*************************************************/
FRSENGINE int FRSInitialize();

/*************************************************
    Method : FRSInitialize
	@Others: 释放资源
*************************************************/
FRSENGINE void FRSUnitialize();

/*************************************************
    Method : FRSGetVersion
	@Others: 获取此接口版本
*************************************************/
FRSENGINE const char* FRSGetVersion();

/*************************************************
    Method   : FRSGetTargetFeature
	@Input
	[bImage]          : 图片数据
	[nImageSize]      : 图片大小
	[bFeature]        : 特征值数据
	[nMaxFeatureSize] : 存放特征值数据字节的大小

	@Others : 特征值定长2596个字节, 若无法正确返回可能原因为图片不合格
*************************************************/
FRSENGINE int FRSGetTargetFeature(BYTE* bImage, DWORD nImageSize, BYTE* bFeature, DWORD& nMaxFeatureSize);

/*************************************************
    Method   : FRSGetImageQualityAndFaceRect
	@Input
	[bImage]          : 图片数据
	[nImageSize]      : 图片大小
	[fScore]          : 质量分数
	[rectFace]        : 存放人脸位置区域, windows下结构体

	@Others : 特征值定长2596个字节, 若无法正确返回可能原因为图片不合格
*************************************************/
FRSENGINE int FRSGetImageQualityAndFaceRect(BYTE* bImage, DWORD nImageSize, float& fScore, RECT& rectFace);

/*************************************************
    Method   : FRSFeatureMatch
	@Input
	[bSrcFeature] : 比对特征值1
	[bDstFeature] : 比对特征值2
	[fScore]      : 相似度
	@Others       : 
*************************************************/
FRSENGINE int FRSFeatureMatch(BYTE* bSrcFeature, BYTE* bDstFeature, float& fScore);

/*************************************************
    Method   : FRSImageConvertBMP
	@Input
	[outBuf]          : 转换后的图片数据
	[dwMaxOutBufSize] : 
	[inBuf]           : 原图片
	[dwInBufSize]     :
	@Others           : 
*************************************************/
FRSENGINE bool FRSImageConvertBMP(BYTE* outBuf, DWORD& dwMaxOutBufSize, BYTE* inBuf, DWORD dwInBufSize);

#endif // FRSEngineInterface_h__

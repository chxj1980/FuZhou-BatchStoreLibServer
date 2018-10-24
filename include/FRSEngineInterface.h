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
	FRSResult_Ok = 0,                    //�ɹ�
	FRSResult_GDIPlusInitFail = -10,     //GDI+��ʼ��ʧ��
	FRSResult_FRSLibInitFail,            //FRS���ʼ��ʧ��
	FRSResult_FeatureBufTooSmall,        //�������ֵ�����СС��2596
	FRSResult_NotBmpFormat,              //��Bmp��ʽͼƬ
	FRSResult_ConvertImageFail,          //ת��ͼƬ��ʽʧ��
	FRSResult_FRSGetFeatureFail,         //FRS��ȡ����ֵʧ��
	FRSResult_FRSGetQualityFail,         //FRS��ȡ����ʧ��
	FRSResult_FRSMatchFeatureFail        //FRS����ֵ�ȶ�ʧ��
};

/*************************************************
    Method : FRSInitialize
	@Others: ��ʼ��
*************************************************/
FRSENGINE int FRSInitialize();

/*************************************************
    Method : FRSInitialize
	@Others: �ͷ���Դ
*************************************************/
FRSENGINE void FRSUnitialize();

/*************************************************
    Method : FRSGetVersion
	@Others: ��ȡ�˽ӿڰ汾
*************************************************/
FRSENGINE const char* FRSGetVersion();

/*************************************************
    Method   : FRSGetTargetFeature
	@Input
	[bImage]          : ͼƬ����
	[nImageSize]      : ͼƬ��С
	[bFeature]        : ����ֵ����
	[nMaxFeatureSize] : �������ֵ�����ֽڵĴ�С

	@Others : ����ֵ����2596���ֽ�, ���޷���ȷ���ؿ���ԭ��ΪͼƬ���ϸ�
*************************************************/
FRSENGINE int FRSGetTargetFeature(BYTE* bImage, DWORD nImageSize, BYTE* bFeature, DWORD& nMaxFeatureSize);

/*************************************************
    Method   : FRSGetImageQualityAndFaceRect
	@Input
	[bImage]          : ͼƬ����
	[nImageSize]      : ͼƬ��С
	[fScore]          : ��������
	[rectFace]        : �������λ������, windows�½ṹ��

	@Others : ����ֵ����2596���ֽ�, ���޷���ȷ���ؿ���ԭ��ΪͼƬ���ϸ�
*************************************************/
FRSENGINE int FRSGetImageQualityAndFaceRect(BYTE* bImage, DWORD nImageSize, float& fScore, RECT& rectFace);

/*************************************************
    Method   : FRSFeatureMatch
	@Input
	[bSrcFeature] : �ȶ�����ֵ1
	[bDstFeature] : �ȶ�����ֵ2
	[fScore]      : ���ƶ�
	@Others       : 
*************************************************/
FRSENGINE int FRSFeatureMatch(BYTE* bSrcFeature, BYTE* bDstFeature, float& fScore);

/*************************************************
    Method   : FRSImageConvertBMP
	@Input
	[outBuf]          : ת�����ͼƬ����
	[dwMaxOutBufSize] : 
	[inBuf]           : ԭͼƬ
	[dwInBufSize]     :
	@Others           : 
*************************************************/
FRSENGINE bool FRSImageConvertBMP(BYTE* outBuf, DWORD& dwMaxOutBufSize, BYTE* inBuf, DWORD dwInBufSize);

#endif // FRSEngineInterface_h__

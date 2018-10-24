#pragma once

#ifdef WIN32
#ifdef XSIGHT_DLL_EXPORTS
#define XS_ENCRYPT_Dll __declspec(dllexport)
#else
#define XS_ENCRYPT_Dll __declspec(dllimport)
#endif
#endif


extern "C"
{
	//�ṩһ��Ĭ�Ϲ��õ���Կ���мӽ���

	//pOutPublicKey:�����Կ
	//pOutPrivateKey:���˽Կ
	//bDefault:�Ƿ���һ��Ĭ�Ϲ��õ���Կ
	//Key���Ȳ�����2*1024
	XS_ENCRYPT_Dll bool XSGetKey(char *pOutPublicKey,char *pOutPrivateKey,bool bDefault);

	//����
	//pInSource:����
	//pOutSource:������ܺ�Ĵ�
	//pPublicKey:��Կ,�����ʱΪĬ����Կ
	XS_ENCRYPT_Dll bool XSEncrypt(const char *pInSource, char *pOutSource, const char *pPublicKey);

	//����
	//pInSource:���ܵĴ�
	//pOutSource:������ܺ�Ĵ�
	//pPrivateKey:��Կ,�����ʱΪĬ����Կ
	XS_ENCRYPT_Dll bool XSDecrypt(const char *pInSource, char *pOutSource, const char *pPrivateKey);
}

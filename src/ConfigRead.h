#pragma once
#include <string>

#include <property_tree/ptree.hpp>
#include <property_tree/xml_parser.hpp>
#include <filesystem/path.hpp>
#include <filesystem/operations.hpp>
#include <typeof/typeof.hpp>

using namespace boost::property_tree;
using namespace std;


class CConfigRead
{
public:
	CConfigRead(void);
public:
    ~CConfigRead(void);
public:
    string GetCurrentPath();
    bool ReadConfig();
public:
    string m_sCurrentPath;  //����ǰ·��
    string m_sConfigFile;	//����·��

    string m_sDBType;       //���ݿ�����
    string m_sDBIP;		    //���ݿ�IP
    int m_nDBPort;          //���ݿ�˿�
    string m_sDBName;		//���ݿ���
    string m_sDBUid;		//�û���
    string m_sDBPwd;		//�û�����

    string m_sServerCode;    //����Code
    string m_sSavePath;     //ͼƬ�������
    int m_nQuality;      //���ڴ������������ص��ͼƬ�����
};

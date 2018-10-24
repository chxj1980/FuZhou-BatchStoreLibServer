#include "StdAfx.h"
#include "ConfigRead.h"
#include "XSEncryptSDK.h"


CConfigRead::CConfigRead(void)
{
    
}

CConfigRead::~CConfigRead(void)
{
}

string CConfigRead::GetCurrentPath()
{
    DWORD nBufferLenth = MAX_PATH;
    char szBuffer[MAX_PATH] = { 0 };
    DWORD dwRet = GetModuleFileNameA(NULL, szBuffer, nBufferLenth);
    char *sPath = strrchr(szBuffer, '\\');
    memset(sPath, 0, strlen(sPath));
    m_sCurrentPath = szBuffer;
    return m_sCurrentPath;
}


bool CConfigRead::ReadConfig()
{
    m_sConfigFile = m_sCurrentPath + "/config/Config.xml";
#ifdef _DEBUG
    m_sConfigFile = "./config/Config.xml";
#endif
    try
    {
        ptree pt;
        read_xml(m_sConfigFile, pt, boost::property_tree::xml_parser::trim_whitespace);

        char pTemp[2048] = { 0 };

        m_sDBType = pt.get<string>("Config.DataBase.Drive");
        m_sDBIP = pt.get<string>("Config.DataBase.IP");
        if (m_sDBIP.size() < 36)      //Œ¥º”√‹◊¥Ã¨
        {
            XSEncrypt(m_sDBIP.c_str(), pTemp, NULL);
            pt.put<string>("Config.DataBase.IP", pTemp);
        }
        else                        //º”√‹◊¥Ã¨, ‘ÚΩ‚√‹
        {
            XSDecrypt(m_sDBIP.c_str(), pTemp, NULL);
            m_sDBIP = pTemp;
        }

        string sDBPort = pt.get<string>("Config.DataBase.Port");
        if (sDBPort.size() < 36)      //Œ¥º”√‹◊¥Ã¨
        {
            XSEncrypt(sDBPort.c_str(), pTemp, NULL);
            m_nDBPort = atoi(sDBPort.c_str());
            pt.put<string>("Config.DataBase.Port", pTemp);
        }
        else                        //º”√‹◊¥Ã¨
        {
            XSDecrypt(sDBPort.c_str(), pTemp, NULL);
            m_nDBPort = atoi(pTemp);
        }

        m_sDBName = pt.get<string>("Config.DataBase.Name");
        if (m_sDBName.size() < 36)      //Œ¥º”√‹◊¥Ã¨
        {
            XSEncrypt(m_sDBName.c_str(), pTemp, NULL);
            pt.put<string>("Config.DataBase.Name", pTemp);
        }
        else                        //º”√‹◊¥Ã¨
        {
            XSDecrypt(m_sDBName.c_str(), pTemp, NULL);
            m_sDBName = pTemp;
        }

        m_sDBUid = pt.get<string>("Config.DataBase.UserID");
        if (m_sDBUid.size() < 36)
        {
            XSEncrypt(m_sDBUid.c_str(), pTemp, NULL);
            pt.put<string>("Config.DataBase.UserID", pTemp);
        }
        else
        {
            XSDecrypt(m_sDBUid.c_str(), pTemp, NULL);
            m_sDBUid = pTemp;
        }

        m_sDBPwd = pt.get<string>("Config.DataBase.Password");
        if (m_sDBPwd.size() < 36)
        {
            XSEncrypt(m_sDBPwd.c_str(), pTemp, NULL);
            pt.put<string>("Config.DataBase.Password", pTemp);
        }
        else
        {
            XSDecrypt(m_sDBPwd.c_str(), pTemp, NULL);
            m_sDBPwd = pTemp;
        }

        boost::property_tree::xml_writer_settings<char> settings('\t', 1);
        boost::property_tree::write_xml(m_sConfigFile, pt, std::locale(), settings);     
#ifdef LAYOUTSERVER
        m_sServerCode = pt.get<string>("Config.LayoutServer.Servercode");
#endif
#ifdef BATCHSTORELIBSERVER
        m_sServerCode = pt.get<string>("Config.BatchStoreLibServer.Servercode");
        m_sSavePath = pt.get<string>("Config.BatchStoreLibServer.SavePath");
        if ("" == m_sSavePath)
        {
            m_sSavePath = "D:/XS-Face";
        }
        m_nQuality = pt.get<int>("Config.BatchStoreLibServer.Quality");
#endif
    }
    catch (boost::property_tree::ptree_error e)
    {
        printf("****Error: ∂¡»°≈‰÷√Œƒº˛≥ˆ¥Ì, %s!", e.what());
        return false;
    }

    if (m_sDBIP == "")
    {
        return false;
    }

    return true;
}

// TestSTSDKLib.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <Windows.h>
#include "STSDKInterface.h"
#include <iostream>
#include <map>
using namespace std;

#define MAXRECVLEN  1024 * 1024 * 2

int _tmain(int argc, _TCHAR* argv[])
{

    char pIn[2048] = { 0 };
    char pPath[2048] = { 0 };

    char * pResponsMsg = new char[MAXRECVLEN];
    unsigned int nResLen = MAXRECVLEN;
    //int nStatus = STSDK_Init("172.16.2.100", 9001);
    STSERVERINFO STServerInfo[2];
    strcpy(STServerInfo[0].pSTServerIP, "35.24.22.192");
    STServerInfo[0].nSTServerPort = 9001;
    STServerInfo[0].nType = 1;

    strcpy(STServerInfo[1].pSTServerIP, "172.16.2.101");
    STServerInfo[1].nSTServerPort = 9001;
    STServerInfo[1].nType = 1;
    int nStatus = STSDK_Init(STServerInfo, 1);
    if(nStatus < 0)
    {
        printf("****Error: ��������������ʧ��!\n");
        getchar();
    }
    else
    {
        Sleep(1000);
        //printf("Success: ���������������ɹ�!\n");
        while(true)
        {
            ZeroMemory(pResponsMsg, MAXRECVLEN);
            nResLen = MAXRECVLEN;
            printf("1: ����ָ��DB\n"
                "2: ɾ��ָ��DB\n"
                "3: ����ָ��DBȫ��ͼƬ����\n"
                "4: ȡ��ȫ�����ݿ���Ϣ\n"
                "5: ����ͼƬ�����������������\n"
                "6: ͬ����Ƭ��DB\n"
                "7: ����ͼƬID��ȡͼƬ\n"
                "8: ��ָ�����ݿ���ɾ��ͼƬ\n"
                "9: ����ͼƬ����\n"
                "a: 1:1������֤\n"
                "b: ��ȡָ������ͼƬ������\n"
                "c: ��ȡĿ������ͼƬ������\n"
                "d: ��������ֵ����ͼƬ����\n"
                "e: �������ֵ\n"
                "f: ��ȡͼƬ����ֵ\n"
                "h: ������ȡͼƬ����ֵ\n"
                "g: �������ͼƬ\n"
                "r: �˳�\n"
                "������ѡ��: ");
            char p = 0;
            cin >> p;
            switch (p)
            {
            case '1':
                cout << "�������ݿ�: " << endl;
                cin >> pIn;
                nStatus = STSDK_OperationDB(pIn, 1, pResponsMsg, &nResLen);
                if(nStatus < 0)
                {
                    printf("STSDK_OperationDBʧ��, ErrorCode[%d]!", nStatus);
                }
                else
                {
                    printf("Recv: \n%s\n\n", pResponsMsg);
                    //printf("Success: STSDK_OperationDB�ɹ�!");
                }
                break;
            case '2':
                cout << "ɾ�����ݿ�: " << endl;
                cin >> pIn;
                nStatus = STSDK_OperationDB(pIn, 2, pResponsMsg, &nResLen);
                if(nStatus < 0)
                {
                    printf("STSDK_OperationDBʧ��, ErrorCode[%d]!", nStatus);
                }
                else
                {
                    printf("Recv: \n%s\n\n", pResponsMsg);
                    //printf("Success: STSDK_OperationDB�ɹ�!");
                }
                break;
            case '3':
                cout << "�������ݿ�����: " << endl;
                cin >> pIn;
                nStatus = STSDK_OperationDB(pIn, 3, pResponsMsg, &nResLen);
                if(nStatus < 0)
                {
                    printf("STSDK_OperationDBʧ��, ErrorCode[%d]!", nStatus);
                }
                else
                {
                    printf("Recv: \n%s\n\n", pResponsMsg);
                    //printf("Success: STSDK_OperationDB�ɹ�!");
                }
                break;
            case '4':
                nStatus = STSDK_GetAllDBInfo(pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);
                break;
            case '5':
                {
                    cout << "�鿴ͼƬ����·��: " << endl;
                    cin >> pPath;
                    HANDLE m_hFileHandle = CreateFile(pPath,
                        GENERIC_READ | GENERIC_WRITE, 
                        FILE_SHARE_READ | FILE_SHARE_WRITE, 
                        NULL, 
                        OPEN_EXISTING, 
                        FILE_ATTRIBUTE_NORMAL, 
                        NULL);
                    if(INVALID_HANDLE_VALUE == m_hFileHandle)
                    {
                        printf("��ͼƬʧ��, ���ͼƬ·��...");
                        break;
                    }
                    int m_nFileSize = GetFileSize(m_hFileHandle, NULL);
                    char * pImageInfo = new char[m_nFileSize + 1];
                    DWORD * pRealRead = new DWORD;
                    ReadFile(m_hFileHandle, pImageInfo, m_nFileSize, pRealRead, NULL);

                    nStatus = STSDK_FaceQuality(pImageInfo, m_nFileSize, pResponsMsg, &nResLen);
                    printf("Recv: \n%s\n\n", pResponsMsg);
            }
                
                break;
            case '6':
            {
                cout << "ͬ��ͼƬ·��: " << endl;
                cin >> pPath;
                cout << "ͬ�����ݿ�: " << endl;
                cin >> pIn;
                HANDLE m_hFileHandle = CreateFile(pPath,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
                if (INVALID_HANDLE_VALUE == m_hFileHandle)
                {
                    printf("��ͼƬʧ��, ���ͼƬ·��...");
                    break;
                }
                int m_nFileSize = GetFileSize(m_hFileHandle, NULL);
                char * pImageInfo = new char[m_nFileSize + 1];
                DWORD * pRealRead = new DWORD;
                ReadFile(m_hFileHandle, pImageInfo, m_nFileSize, pRealRead, NULL);

                nStatus = STSDK_SynAddImage(pIn, pImageInfo, m_nFileSize, 0, 0, pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);
                break;
            }
            case 'g':
            {
                int nNum = 0;
                STIMAGEINFO STImageInfo[50];
                cout << "����ͬ��ͼƬ����: " << endl;
                cin >> nNum;
                cout << "����ͬ��ͼƬ·��: " << endl;
                cin >> pPath;
                for(int i = 0; i < nNum; i ++)
                {
                    char pName[32] = {0};
                    sprintf_s(pName, sizeof(pName), "%d.jpg", i + 1);

                    string sFileName = string(pPath);
                    sFileName += "/";
                    sFileName += string(pName);
                    HANDLE m_hFileHandle = CreateFile(sFileName.c_str(),
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
                    if (INVALID_HANDLE_VALUE == m_hFileHandle)
                    {
                        printf("��ͼƬʧ��, ���ͼƬ·��...");
                        break;
                    }
                    int m_nFileSize = GetFileSize(m_hFileHandle, NULL);
                    char * pImageInfo = new char[m_nFileSize + 1];
                    DWORD * pRealRead = new DWORD;
                    ReadFile(m_hFileHandle, pImageInfo, m_nFileSize, pRealRead, NULL);

                    STImageInfo[i].pImageBuf = pImageInfo;
                    STImageInfo[i].nImageLen = m_nFileSize;
                    strcpy_s(STImageInfo[i].pName, sizeof(STImageInfo[i].pName), pName);
                }
                
                cout << "ͬ�����ݿ�: " << endl;
                cin >> pIn;
                nStatus = STSDK_SynAddMultipleImage(pIn, STImageInfo, nNum, 0, 0, pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);

                break;
            }
            case '7':
            {
                cout << "��ȡͼƬID: " << endl;
                cin >> pPath;
                nStatus = STSDK_GetImageByID(pPath, pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);
                break;
            }
            case '8':
            {
                cout << "ɾ��ͼƬID: " << endl;
                cin >> pPath;
                cout << "���ݿ�: " << endl;
                cin >> pIn;
                nStatus = STSDK_DelImageByID(pIn, pPath, pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);
                break;
            }
            case '9':
            {
                cout << "�ȶ�ͼƬ·��: " << endl;
                cin >> pPath;
                cout << "���ݿ�: " << endl;
                cin >> pIn;
                HANDLE m_hFileHandle = CreateFile(pPath,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
                if (INVALID_HANDLE_VALUE == m_hFileHandle)
                {
                    printf("��ͼƬʧ��, ���ͼƬ·��...");
                    break;
                }
                int m_nFileSize = GetFileSize(m_hFileHandle, NULL);
                char * pImageInfo = new char[m_nFileSize + 1];
                DWORD * pRealRead = new DWORD;
                ReadFile(m_hFileHandle, pImageInfo, m_nFileSize, pRealRead, NULL);
                nStatus = STSDK_SearchImage(pIn, pImageInfo, m_nFileSize, 10, 0.5, pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);
                break;
            }
            case 'a':
            {

                cout << "�ȶ�ͼƬ·��1: " << endl;
                cin >> pPath;
                cout << "�ȶ�ͼƬ·��2: " << endl;
                cin >> pIn;
                HANDLE m_hFileHandle = CreateFile(pPath,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
                if (INVALID_HANDLE_VALUE == m_hFileHandle)
                {
                    printf("��ͼƬʧ��, ���ͼƬ·��...");
                    break;
                }
                int m_nFileSize = GetFileSize(m_hFileHandle, NULL);
                char * pImageInfo = new char[m_nFileSize + 1];
                DWORD * pRealRead = new DWORD;
                ReadFile(m_hFileHandle, pImageInfo, m_nFileSize, pRealRead, NULL);

                HANDLE m_hFileHandle2 = CreateFile(pIn,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
                if (INVALID_HANDLE_VALUE == m_hFileHandle2)
                {
                    printf("��ͼƬʧ��, ���ͼƬ·��...");
                    break;
                }
                int m_nFileSize2 = GetFileSize(m_hFileHandle2, NULL);
                char * pImageInfo2 = new char[m_nFileSize2 + 1];
                DWORD * pRealRead2 = new DWORD;
                ReadFile(m_hFileHandle2, pImageInfo2, m_nFileSize2, pRealRead2, NULL);
                nStatus = STSDK_Verification(pImageInfo, m_nFileSize, pImageInfo2, m_nFileSize2, pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);
                break;
            }
            case 'b':
            {
                cout << "��ȡָ������ͼƬ������·��: " << endl;
                cin >> pPath;
                HANDLE m_hFileHandle = CreateFile(pPath,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
                if (INVALID_HANDLE_VALUE == m_hFileHandle)
                {
                    printf("��ͼƬʧ��, ���ͼƬ·��...");
                    break;
                }
                int m_nFileSize = GetFileSize(m_hFileHandle, NULL);
                char * pImageInfo = new char[m_nFileSize + 1];
                DWORD * pRealRead = new DWORD;
                ReadFile(m_hFileHandle, pImageInfo, m_nFileSize, pRealRead, NULL);

                nStatus = STSDK_GetAttribute(pImageInfo, m_nFileSize, pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);
                break;
            }
            case 'c':
                nStatus = STSDK_GetDetail(pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);
                break;
            case 'd':
            {
                cout << "������������ֵ: " << endl;
                cin >> pPath;
                cout << "���ݿ�: " << endl;
                cin >> pIn;
                nStatus = STSDK_SearchImageByFeature(pIn, pPath, strlen(pPath), 10, 0.5, pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);
                break;
            }
            case 'f':
            {
                cout << "��ȡ����ֵͼƬ·��: " << endl;
                cin >> pPath;
                HANDLE m_hFileHandle = CreateFile(pPath,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
                if (INVALID_HANDLE_VALUE == m_hFileHandle)
                {
                    printf("��ͼƬʧ��, ���ͼƬ·��...");
                    break;
                }
                int m_nFileSize = GetFileSize(m_hFileHandle, NULL);
                char * pImageInfo = new char[m_nFileSize + 1];
                DWORD * pRealRead = new DWORD;
                ReadFile(m_hFileHandle, pImageInfo, m_nFileSize, pRealRead, NULL);

                nStatus = STSDK_GetFeature(pImageInfo, m_nFileSize, pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);
                break;
            }
            case 'h':
                {
                    int nNum;
                    cout << "������ȡ����ֵͼƬ����: " << endl;
                    cin >> nNum;
                    cout << "��ȡ����ֵͼƬ·��: " << endl;
                    cin >> pPath;
                    for(int i = 0; i < nNum; i ++)
                    {
                        nResLen = MAXRECVLEN;
                        char pName[32] = {0};
                        sprintf_s(pName, sizeof(pName), "%d.jpg", i + 1);

                        string sFileName = string(pPath);
                        sFileName += "/";
                        sFileName += string(pName);

                        HANDLE m_hFileHandle = CreateFile(sFileName.c_str(),
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
                        if (INVALID_HANDLE_VALUE == m_hFileHandle)
                        {
                            printf("��ͼƬʧ��, ���ͼƬ·��...");
                            break;
                        }
                        int m_nFileSize = GetFileSize(m_hFileHandle, NULL);
                        char * pImageInfo = new char[m_nFileSize + 1];
                        DWORD * pRealRead = new DWORD;
                        ReadFile(m_hFileHandle, pImageInfo, m_nFileSize, pRealRead, NULL);

                        nStatus = STSDK_GetFeature(pImageInfo, m_nFileSize, pResponsMsg, &nResLen);
                        printf("Recv: \n%s\n\n", pResponsMsg);
                    }
                    break;
                }
            case 'e':
            {
                cout << "������������ֵ: " << endl;
                cin >> pPath;
                cout << "���ݿ�: " << endl;
                cin >> pIn;
                nStatus = STSDK_SynAddFeature(pIn, pPath, pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);
                break;
            }
            case 'r':
                return 0;
            default:
                break;
            }
            //Sleep(1000 * 2);
        }
    }
    return 0;
}


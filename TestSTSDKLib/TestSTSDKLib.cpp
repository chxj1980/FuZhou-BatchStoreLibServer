// TestSTSDKLib.cpp : 定义控制台应用程序的入口点。
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
        printf("****Error: 连接商汤服务器失败!\n");
        getchar();
    }
    else
    {
        Sleep(1000);
        //printf("Success: 连接商汤服务器成功!\n");
        while(true)
        {
            ZeroMemory(pResponsMsg, MAXRECVLEN);
            nResLen = MAXRECVLEN;
            printf("1: 增加指定DB\n"
                "2: 删除指定DB\n"
                "3: 清理指定DB全部图片数据\n"
                "4: 取得全部数据库信息\n"
                "5: 人脸图片检测人脸与质量评分\n"
                "6: 同步照片到DB\n"
                "7: 根据图片ID获取图片\n"
                "8: 在指定数据库中删除图片\n"
                "9: 人脸图片搜索\n"
                "a: 1:1人脸验证\n"
                "b: 提取指定人脸图片的属性\n"
                "c: 获取目标库最大图片总容量\n"
                "d: 根据特征值人脸图片搜索\n"
                "e: 入库特征值\n"
                "f: 获取图片特征值\n"
                "h: 批量获取图片特征值\n"
                "g: 批量入库图片\n"
                "r: 退出\n"
                "请输入选项: ");
            char p = 0;
            cin >> p;
            switch (p)
            {
            case '1':
                cout << "增加数据库: " << endl;
                cin >> pIn;
                nStatus = STSDK_OperationDB(pIn, 1, pResponsMsg, &nResLen);
                if(nStatus < 0)
                {
                    printf("STSDK_OperationDB失败, ErrorCode[%d]!", nStatus);
                }
                else
                {
                    printf("Recv: \n%s\n\n", pResponsMsg);
                    //printf("Success: STSDK_OperationDB成功!");
                }
                break;
            case '2':
                cout << "删除数据库: " << endl;
                cin >> pIn;
                nStatus = STSDK_OperationDB(pIn, 2, pResponsMsg, &nResLen);
                if(nStatus < 0)
                {
                    printf("STSDK_OperationDB失败, ErrorCode[%d]!", nStatus);
                }
                else
                {
                    printf("Recv: \n%s\n\n", pResponsMsg);
                    //printf("Success: STSDK_OperationDB成功!");
                }
                break;
            case '3':
                cout << "清理数据库数据: " << endl;
                cin >> pIn;
                nStatus = STSDK_OperationDB(pIn, 3, pResponsMsg, &nResLen);
                if(nStatus < 0)
                {
                    printf("STSDK_OperationDB失败, ErrorCode[%d]!", nStatus);
                }
                else
                {
                    printf("Recv: \n%s\n\n", pResponsMsg);
                    //printf("Success: STSDK_OperationDB成功!");
                }
                break;
            case '4':
                nStatus = STSDK_GetAllDBInfo(pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);
                break;
            case '5':
                {
                    cout << "查看图片质量路径: " << endl;
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
                        printf("打开图片失败, 检查图片路径...");
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
                cout << "同步图片路径: " << endl;
                cin >> pPath;
                cout << "同步数据库: " << endl;
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
                    printf("打开图片失败, 检查图片路径...");
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
                cout << "批量同步图片数量: " << endl;
                cin >> nNum;
                cout << "批量同步图片路径: " << endl;
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
                        printf("打开图片失败, 检查图片路径...");
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
                
                cout << "同步数据库: " << endl;
                cin >> pIn;
                nStatus = STSDK_SynAddMultipleImage(pIn, STImageInfo, nNum, 0, 0, pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);

                break;
            }
            case '7':
            {
                cout << "获取图片ID: " << endl;
                cin >> pPath;
                nStatus = STSDK_GetImageByID(pPath, pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);
                break;
            }
            case '8':
            {
                cout << "删除图片ID: " << endl;
                cin >> pPath;
                cout << "数据库: " << endl;
                cin >> pIn;
                nStatus = STSDK_DelImageByID(pIn, pPath, pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);
                break;
            }
            case '9':
            {
                cout << "比对图片路径: " << endl;
                cin >> pPath;
                cout << "数据库: " << endl;
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
                    printf("打开图片失败, 检查图片路径...");
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

                cout << "比对图片路径1: " << endl;
                cin >> pPath;
                cout << "比对图片路径2: " << endl;
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
                    printf("打开图片失败, 检查图片路径...");
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
                    printf("打开图片失败, 检查图片路径...");
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
                cout << "提取指定人脸图片的属性路径: " << endl;
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
                    printf("打开图片失败, 检查图片路径...");
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
                cout << "输入人脸特征值: " << endl;
                cin >> pPath;
                cout << "数据库: " << endl;
                cin >> pIn;
                nStatus = STSDK_SearchImageByFeature(pIn, pPath, strlen(pPath), 10, 0.5, pResponsMsg, &nResLen);
                printf("Recv: \n%s\n\n", pResponsMsg);
                break;
            }
            case 'f':
            {
                cout << "提取特征值图片路径: " << endl;
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
                    printf("打开图片失败, 检查图片路径...");
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
                    cout << "批量获取特征值图片数量: " << endl;
                    cin >> nNum;
                    cout << "提取特征值图片路径: " << endl;
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
                            printf("打开图片失败, 检查图片路径...");
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
                cout << "输入人脸特征值: " << endl;
                cin >> pPath;
                cout << "数据库: " << endl;
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


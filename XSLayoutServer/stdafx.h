// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <string>
#include <list>
#include <map>

#include "DataDefine.h"
#include "RedisManage.h"
#include "AnalyseSearch.h"
#include "LogRecorder.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"
using namespace std;

#pragma warning(disable:4996)
#pragma warning(disable:4244)
#pragma warning(disable:4267)
#pragma warning(disable:4018)
#pragma warning(disable:4800)
#pragma warning(disable:4267)
#pragma warning(disable:4003)
#pragma warning(disable:4099)




// TODO: 在此处引用程序需要的其他头文件

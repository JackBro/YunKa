// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"
#include "Resource.h"
#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�: 
#include <windows.h>

// C ����ʱͷ�ļ�
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <objbase.h>
#include "./ui_common/global_setting_define.h"


#include "UIlib.h"

using namespace DuiLib;



#define lengthof(x) (sizeof(x)/sizeof(*x))
#define MAX max
#define MIN min
#define CLAMP(x,a,b) (MIN(b,MAX(a,x)))




#ifdef _DEBUG
#   ifdef _UNICODE
#       pragma comment(lib, "DuiLib.lib")
#   else
#       pragma comment(lib, "..\\..\\DuiLib\\Lib\\DuiLib_d.lib")
#   endif
#else
#   ifdef _UNICODE
#       pragma comment(lib, "..\\..\\DuiLib\\Lib\\DuiLib_u.lib")
#   else
#       pragma comment(lib, "..\\..\\DuiLib\\Lib\\DuiLib.lib")
#   endif
#endif


// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�

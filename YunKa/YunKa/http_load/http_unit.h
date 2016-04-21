#ifndef _HTTP_UNIT_H_
#define _HTTP_UNIT_H_
#include "switch_sock.h"

using namespace std;

class IHttpFileEvent
{
public:
	virtual void OnProgress(int nIndex, int nTransferd, int nTotal) = 0;
	virtual void OnReturn(int nIndex, LPCTSTR lpszRetVal) = 0;
	virtual void OnError(int nIndex, int nCode) = 0;
};

void UseSwitchInHttpDownload(bool bUse = true);

//!����һ��URL���ŵ�body�У�����д������ã���ʹ�ô���, ����HTTP����
//int HttpDownload(tstring URL,tstring &body,const tstring& post=_T(""),const tstring& AdditionHead=_T(""));
//int HttpDownload(string URL,string &body,const string& post="",const string& AdditionHead="");//gaoxiaohu
int HttpDownloadFile(string url,
				 string &body,
				 string &urlfile,
				 string postfile,
				 HWND hWnd,
				 const string& AdditionHead = "",
				 const char* pszProxyip = NULL,
				 unsigned short proxyport =0);

int HttpDownload(string url,
				 string post,
				 string &body,
				 HWND hWnd,
				 const string& AdditionHead = "",
				 const char* pszProxyip = NULL,
				 unsigned short proxyport =0);

string GetModuleFileDir();

// http�����ļ�
DWORD HttpDownloadFile(int nIndex,
					char* lpszUrl,
					char* lpszLocalName,
					char* lpszHeader = NULL,
					IHttpFileEvent* pEvent = NULL);
//---------------------------------------------------------------------------
#endif

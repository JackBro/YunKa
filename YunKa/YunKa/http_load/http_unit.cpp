#include "../stdafx.h"
#include "http_unit.h"
#include "convert.h"
#include "../chat_common/comfunc.h"
#include <WinInet.h>

#pragma comment(lib, "wininet.lib")

void UseSwitchInHttpDownload(bool bUse)
{
	http_useswitch(bUse);
}

//带超时的连接
int Connect_t(SOCKET& sockfd, const struct sockaddr  * pserv_addr,const unsigned int& namelen,int nsecond)
{
	unsigned long ul = 1;
	ioctlsocket(sockfd, FIONBIO, &ul);  //设置为非阻塞模式
	int nret = SOCKET_ERROR;
	if( connect(sockfd, pserv_addr,namelen) == -1)
	{
		
		int error=-1, len;
		timeval tm;
		fd_set set;
		tm.tv_sec  = nsecond;
		tm.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(sockfd, &set);
		if( select(sockfd+1, NULL, &set, NULL, &tm) > 0)
		{
			len=sizeof(error);
			getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
			if(error == 0)
				nret = 0;
		} 
    }else
	{
		nret = 0;
	}
	ul = 0;
	ioctlsocket(sockfd, FIONBIO, &ul);  //设置为阻塞模式
	
	return nret;
}

SOCKET ConnectToServer(const char* cDomain,unsigned short port,
					   const char *proxyip,unsigned short proxyport)
{
	SOCKET s=socket(AF_INET,SOCK_STREAM,0);
	if (INVALID_SOCKET ==s)
	{
		return s;
	}
	int timeout = 5000;//设置socket超时，防止认证过程阻塞
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	sockaddr_in saddr;
	if (proxyip!=NULL && strlen(proxyip) > 0)
	{
		saddr.sin_addr.s_addr=inet_addr(proxyip);
		saddr.sin_family=AF_INET;
		saddr.sin_port=htons(proxyport);
		
		if (SOCKET_ERROR==Connect_t(s,(sockaddr*)&saddr,sizeof(saddr), 5))
		{
			closesocket(s);
			s=INVALID_SOCKET;
			return s;
		}
		
		char buf[2000];
		sprintf(buf,"CONNECT %s:%u HTTP/1.1\r\nAccept: */*\r\nContent-Type: text/html\r\nProxy-Connection: Keep-Alive\r\nContent-length: 0\r\n\r\n",
			cDomain,port);
		send(s,buf,strlen(buf),0);
		
		recv(s,buf,2000,0);
		if (strstr(buf,"established")==NULL)
		{
			return s;
		}
		
	}else
	{
		try
		{
			struct hostent * he = gethostbyname(cDomain);
			if (he==NULL)
			{
				closesocket(s);
				s=INVALID_SOCKET;
				return s;
			}
			
			memset(saddr.sin_zero,0,8);
			saddr.sin_addr=*((struct   in_addr*)he->h_addr);
			saddr.sin_family=AF_INET;
			saddr.sin_port=htons(port);
			
			if (0!=Connect_t(s,(sockaddr*)&saddr,sizeof(saddr), 5))
			{
				closesocket(s);
				s=INVALID_SOCKET;
			}
		}
		catch (...)
		{
			return INVALID_SOCKET;
		}
		
	}
	return s;
}

int HttpDownload(string url,
				 string post,
				 string &body,
				 HWND hWnd,
				 const string& AdditionHead,
				 const char* pszProxyip,
				 unsigned short proxyport)
{
	body="";
	int pos=url.find ('/',8);
	if(pos==-1)
	{
		return 404;
	}
	
	
	string host=url.substr(7,pos-7);
	string remotepath=url.substr (pos,url.length ());
	pos=host.find(':');
	int port=80;
	if(pos!=-1)
	{
        port=atoi(host.substr(pos+1,host.length()).c_str()); 
		if(port==0)
			port=80;
		host=host.substr(0,pos);
	}
	
    
	SOCKET sServer=INVALID_SOCKET;

	if (pszProxyip == NULL || strlen(pszProxyip) == 0)
	{
		sServer = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if( sServer == INVALID_SOCKET)
			return 0;


		struct sockaddr_in inAddr;

		inAddr.sin_addr.S_un.S_addr=inet_addr(host.c_str());
		inAddr.sin_family=AF_INET;
		inAddr.sin_port=htons(port);
		if(inAddr.sin_addr.S_un.S_addr == INADDR_NONE)
		{
			struct hostent* phost = gethostbyname(host.c_str());
			if(phost == NULL)
				return 404;
			inAddr.sin_addr = *((struct in_addr*)phost->h_addr);
		}
		if(connect(sServer, (SOCKADDR*)&inAddr, sizeof(inAddr)) != 0)
		{
			closesocket(sServer);
			sServer=INVALID_SOCKET;
		}
	}else
	{
		sServer=ConnectToServer(host.c_str(), port, pszProxyip, proxyport);

	}
	if (sServer==INVALID_SOCKET)
	{
		return 404;
	}
	
	string cmd;
	if(post=="")
	{
		cmd="GET "+remotepath+" HTTP/1.0\r\nHost: "+host+"\r\nUser-Agent:Tracq\r\n"+AdditionHead+"\r\n";
	}
	else
	{
		cmd="POST "+remotepath+" HTTP/1.0\r\nHost: "+host+"\r\nUser-Agent:Tracq\r\nContent-Length: "+ CConvert::IntToStr(post.length())+"\r\n"+AdditionHead+"\r\n"+post;
	}
	
	string str;
	str = GetCurrentPath() + "\\postu.txt";
	CConvert::SaveFile(str, cmd.c_str(), cmd.length());//modify by zyl
	
	send(sServer, cmd.c_str(),cmd.length(), 0);
	
	int nHttpCode = 404;	  
	
	char *buf=new char[15535];  
	
	int nLenth = 0;   
	int nRecv;
	nRecv = recv(sServer, buf, 15534, 0);
	if (nRecv > 0)
	{
		buf[nRecv]=0;
		char* pStatue = strchr(buf,' ');
		char* headend = strstr(buf,"\r\n\r\n");
		*headend = '\0';
		headend += 4;
		nHttpCode = atoi(pStatue);
		if (nHttpCode != 200)
		{
			closesocket(sServer);
			delete [] buf;
			return nHttpCode;
		}
		
		pStatue = strstr(buf, "Content-Length:");
		pStatue += 15;
		nLenth = atoi(pStatue);  
		
		string strNewURL;
		char* pNewURL= strstr(buf, "Last-Modified:");
		if(pNewURL != NULL)
		{
			pNewURL+=14;
			while((*pNewURL)==' ')
			{
				pNewURL++;
			}
			char* pEnd=strchr(pNewURL,'\n'); 
			if(pEnd) 
				*pEnd='\0';
			pEnd = strchr(pNewURL,'\r');
			if(pEnd) 
				*pEnd='\0';
		}
		
		int headlen=headend-buf;
		body = string(headend, nRecv - headlen); 
		
		nLenth = nLenth - nRecv + headlen;
		while(nLenth > 0)
		{
			nRecv = recv(sServer, buf, 15534, 0);
			if (nRecv<=0)
			{
				break;
			}
			buf[nRecv]=0;
			body += string(buf, nRecv);
			nLenth -= nRecv;
		}
		if (nLenth>0) 
		{
			nHttpCode=404;
		}
	}
	
	
	closesocket(sServer);
	delete [] buf;
    return nHttpCode;
}

void GetHostInfo(const string& strHostInfo, string& strHost, UINT& nPort)
{
	int pos = strHostInfo.find(":");

	strHost = strHostInfo;
	nPort = 80;

	if (pos != -1)
	{
		strHost = strHostInfo.substr(pos);
		string strPort = strHostInfo.substr(strHostInfo.length() - pos - 1);
		nPort = (UINT)atoi(strPort.c_str());
	}
}

void GetRequestInfoFromUrl(const string& strUrl,
	string& strHost,
	UINT& nPort,
	string& strRequest)
{
	int pos, pos1;

	string strHttp = "http://";
	string strSp = "/";

	pos = strUrl.find(strHttp);
	int httplen = strHttp.length();
	if (pos != -1)
	{
		string strHostInfo;
		pos1 = strUrl.find(strSp, pos + httplen);
		if (pos1 != -1)
		{
			strHostInfo = strUrl.substr(pos + httplen, pos1 - pos - httplen);
			GetHostInfo(strHostInfo, strHost, nPort);
			strRequest = strUrl.substr(pos1, strUrl.length() - pos1);
		}
	}
}


#define HTTP_RECV_DATA_BUF 10240
DWORD HttpDownloadFile(int nIndex,
	char* lpszUrl,
	char* lpszLocalName,
	char* lpszHeader,
	IHttpFileEvent* pEvent)
{
	DWORD dwRet = 0;

	string strUrl = lpszUrl;
	string strHost;
	string strRequest;
	UINT nPort;

	GetRequestInfoFromUrl(strUrl, strHost, nPort, strRequest);


	HINTERNET hInternet = InternetOpenA(strHost.c_str(),
		INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	HINTERNET hConn = InternetConnectA(hInternet,
		strHost.c_str(),
		nPort,
		NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

	static char* accept[2] = { "*/*", NULL };
	HINTERNET hRequest = HttpOpenRequestA(hConn, "GET",
		strRequest.c_str(), NULL, NULL,
		(LPCSTR*)accept,
		INTERNET_FLAG_RELOAD,
		0);

	if (lpszHeader)
	{
		HttpAddRequestHeadersA(hRequest, lpszHeader, -1L, HTTP_ADDREQ_FLAG_ADD);
	}

	BOOL bRet = HttpSendRequestA(hRequest, NULL, 0, NULL, 0);

	char databuf[HTTP_RECV_DATA_BUF + 1] = { 0 };
	DWORD dwSize = HTTP_RECV_DATA_BUF;
	bRet = HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE, databuf, &dwSize, NULL);

	if (strcmp(databuf, "200") == 0)
	{

		HANDLE hFile = CreateFileA(lpszLocalName,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			dwRet = 2;
			goto CleanUp;
		}

		dwSize = HTTP_RECV_DATA_BUF;
		bRet = HttpQueryInfoA(hRequest, HTTP_QUERY_CONTENT_LENGTH, databuf, &dwSize, NULL);

		BOOL bKnowSize = FALSE;

		DWORD cbFileSize = HTTP_RECV_DATA_BUF;

		if (bRet != 0)
		{
			bKnowSize = TRUE;
			cbFileSize = (DWORD)atol(databuf);
		}

		DWORD dwFinishLen = 0;
		DWORD dwBytesRead = 0;

		do
		{
			dwBytesRead = 0;

			bRet = InternetReadFile(hRequest, databuf, HTTP_RECV_DATA_BUF, &dwBytesRead);

			if (dwBytesRead > 0)
			{
				DWORD dwBytesWritten;
				WriteFile(hFile, databuf, dwBytesRead, &dwBytesWritten, NULL);

				dwFinishLen += dwBytesWritten;

				if (pEvent)
				{
					pEvent->OnProgress(nIndex, dwFinishLen, cbFileSize);
				}
			}

		} while (dwBytesRead > 0);

		CloseHandle(hFile);

		if (pEvent)
		{
			pEvent->OnReturn(nIndex, lpszLocalName);
		}

	}
	else
	{
		dwRet = 1;
	}

CleanUp:
	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConn);
	InternetCloseHandle(hInternet);

	if (dwRet != 0 && pEvent != NULL)
	{
		pEvent->OnError(nIndex, dwRet);
	}

	return dwRet;
}

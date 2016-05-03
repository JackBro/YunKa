#include "../stdafx.h"
#include "comfunc.h"
#include "comdef.h"
#include "../utils/tstring.h"
//#include <Psapi.h>

//#pragma   comment(lib,"psapi.lib")

//�ַ����������εĻ���
DWORD StringToDWORD(string str)
{
	return (DWORD)atoi(str.c_str());
}

bool IsNumber(string lpStr)
{
	if (lpStr.empty())
	{
		return false;
	}

	char ch;

	for (int i = 0; i < lpStr.size(); i++)
	{
		ch = lpStr.at(i);
		if ((ch < '0') || (ch > '9'))
		{
			return false;
		}
	}
	return true;
}

//��ñ����ĵ�ַ
char * GetLocalHost(char * szAddress)
{
	if (gethostname(szAddress, 128) == SOCKET_ERROR)
	{
	int nError = GetLastError();
	return NULL;
	}
	else
		return szAddress;
}

string GetCurrentPath1()
{
	char szMyDllFileName[MAX_1024_LEN];
	string str;

	memset(szMyDllFileName, 0, 1024);
	::GetModuleFileNameA(NULL, szMyDllFileName, MAX_1024_LEN);
	str = szMyDllFileName;

	int pos = str.find_last_of('\\');

	if (pos > -1)
	{
		str = str.substr(0, pos + 1);
	}
	return str;
}

bool LoadIniString(char *sApp, char *sKey, char *sReturn, int len, char *sFile, char *sDefault)
{
	if (sApp == NULL || sKey == NULL || sFile == NULL)
		return false;

	char sDefault1[1024], strString[1024];
	bool brtn = false;

	if (sDefault == NULL)
	{
		strcpy(sDefault1, sReturn);
	}
	else
	{
		strcpy(sDefault1, sDefault);
	}
	strcpy(strString, sDefault1);

	if (GetPrivateProfileStringA(sApp, sKey, sDefault1, strString, len, sFile))
	{
		if (strlen(strString) >= 0)
		{
			strcpy(sReturn, strString);
			brtn = true;
		}
	}

	return brtn;

}

bool LoadIniInt(char *sApp, char *sKey, int &nReturn, char *sFile, char *sDefault)
{
	if (sApp == NULL || sKey == NULL || sFile == NULL)
		return false;

	bool brtn = false;
	char sDefault1[256], strString[256];
	if (sDefault == NULL)
	{
		sprintf(sDefault1, "%d", nReturn);
	}
	else
	{
		sprintf(sDefault1, sDefault);
	}
	strcpy(strString, sDefault1);

	if (GetPrivateProfileStringA(sApp, sKey, sDefault1, strString, 256, sFile))
	{
		if (strlen(strString) > 0)
		{
			nReturn = atol(strString);
			brtn = true;
		}
	}

	return brtn;
}

//��ȡĳ��λ��ֵ
unsigned long GetMutiByte(unsigned int value, int index, int bytenum)
{
	int byte = (int)(pow(2, bytenum) - 1); //�ṹΪ������ 1111 (2**4 - 1)
	unsigned int rtn = value >> index;  //����
	rtn &= byte;  //1111

	return rtn;
}

unsigned short GetMutiByte(unsigned short value, int index, int bytenum)
{
	int byte = (int)(pow(2, bytenum) - 1); //�ṹΪ������ 1111 (2**4 - 1)
	unsigned long rtn = value >> index;  //����
	rtn &= byte;  //1111

	return (unsigned short)rtn;
}

unsigned char GetMutiByte(unsigned char value, int index, int bytenum)
{
	int byte = (int)(pow(2, bytenum) - 1); //�ṹΪ������ 1111 (2**4 - 1)
	unsigned long rtn = value >> index;  //����
	rtn &= byte;  //1111

	return (unsigned char)rtn;
}

//���ö�λ��ֵ
unsigned int SetMutiByte(unsigned int &source, int index, int bytenum, unsigned int value)
{
	// ��ȡ��Ҫ���õ�λ�����Ƶ�ָ����λ��
	int byte;

	byte = (int)(pow(2, bytenum) - 1); //�ṹΪ������ 1111 (2**4 - 1)
	value &= byte;
	value = value << index;   //����

	//ָ����λȫΪ1 ��ȫΪ0�����
	int rtn0, rtn1;
	rtn1 = byte << index;   //����
	rtn0 = ~rtn1;

	//���Ƚ�ָ����λ��Ϊ0
	source &= rtn0;

	//����ָ��λ
	source |= value;

	return source;
}


unsigned short SetMutiByte(unsigned short &source, int index, int bytenum, unsigned short value)
{
	// ��ȡ��Ҫ���õ�λ�����Ƶ�ָ����λ��
	int byte;

	byte = (int)(pow(2, bytenum) - 1); //�ṹΪ������ 1111 (2**4 - 1)
	value &= byte;
	value = value << index;   //����

	//ָ����λȫΪ1 ��ȫΪ0�����
	int rtn0, rtn1;
	rtn1 = byte << index;   //����
	rtn0 = ~rtn1;

	//���Ƚ�ָ����λ��Ϊ0
	source &= rtn0;

	//����ָ��λ
	source |= value;

	return source & 0xffff;
}

unsigned char SetMutiByte(unsigned char &source, int index, int bytenum, unsigned char value)
{
	// ��ȡ��Ҫ���õ�λ�����Ƶ�ָ����λ��
	int byte;

	byte = (int)(pow(2, bytenum) - 1); //�ṹΪ������ 1111 (2**4 - 1)
	value &= byte;
	value = value << index;   //����

	//ָ����λȫΪ1 ��ȫΪ0�����
	int rtn0, rtn1;
	rtn1 = byte << index;   //����
	rtn0 = ~rtn1;

	//���Ƚ�ָ����λ��Ϊ0
	source &= rtn0;

	//����ָ��λ
	source |= value;

	return source & 0xff;
}

unsigned short SendOnePack(SOCKET socket, char *sbuff, TCP_PACK_HEADER tcppackhead, int &nError)
{
	if (tcppackhead.len > PACKNORMALLEN)
	{
		nError = -1;
		return -1;
	}

	nError = 0;
	char data[PACKMAXLEN + sizeof(TCP_PACK_HEADER)] = { 0 };

	int ret, idx, tcpheadlen;
	int nLeft, nSend;

	tcpheadlen = sizeof(TCP_PACK_HEADER);
	*(TCP_PACK_HEADER *)data = tcppackhead;
	memcpy((data + tcpheadlen), sbuff, tcppackhead.len);

	nLeft = tcpheadlen + tcppackhead.len;
	idx = 0;
	nSend = 0;
	while (nLeft > 0)
	{
		ret = send(socket, (char *)(data + idx), nLeft, 0);
		if (ret == SOCKET_ERROR)
		{
			nError = GetLastError();
			if (nError == WSAEWOULDBLOCK)
			{
				Sleep(500);
				continue;
			}
			goto RETURN;
		}
		idx += ret;
		nSend += ret;
		nLeft -= ret;
	}

RETURN:
	return nSend;
}

unsigned short SendOnePack(SOCKET socket, char *data, int len, int &nError, unsigned short cmd,
	unsigned long senduid, unsigned long recvuid, unsigned long sendsock, unsigned long recvsock,
	unsigned short seq)
{
	TCP_PACK_HEADER tcppackhead;

	memset((char *)(&tcppackhead), 0, sizeof(tcppackhead));


	tcppackhead.len = len;


	return SendOnePack(socket, data, tcppackhead, nError);
}

void ConvertMsg(char *msg, int buflen)
{
	wstring s = convertstring((const char *)msg);
	ConvertWidecharToChar(s.c_str(), -1, msg, buflen, false);
}


void ConvertWidecharToChar(const WCHAR *pFrom, int len, char *pTo, int buflen, bool butf8)
{
	if (pFrom == NULL || pTo == NULL)
		return;

	if (len == 0)
	{
		strcpy(pTo, "");
		return;
	}

	if (butf8)
	{
		len = ::WideCharToMultiByte(CP_UTF8, 0, pFrom, len, pTo, buflen, NULL, NULL);
	}
	else
	{
		len = ::WideCharToMultiByte(CP_ACP, 0, pFrom, len, pTo, buflen, NULL, NULL);
	}

	if (len > 0)
		pTo[len] = '\0';
	else
	{
		DWORD dd = GetLastError();
	}

	return;
}

WxObj* ParseWxJsonMsg(const char* msg)
{
	Json::Value jv;
	if (!ParseJson(msg, jv))
	{
		return NULL;
	}

	string MsgType = GetStrFromJson(jv, "msgtype");
	if (MsgType.empty())
	{
		return NULL;
	}

	WxObj  *pwxobj = NULL;
	if ("text" == MsgType)
	{
		pwxobj = new WxMsgText(MsgType);
	}
	else if ("image" == MsgType)
	{
		pwxobj = new WxMsgImage(MsgType);
	}
	else if ("voice" == MsgType)
	{
		pwxobj = new WxMsgVoice(MsgType);
	}
	else if ("video" == MsgType || "shortvideo" == MsgType)
	{
		pwxobj = new WxMsgVideo(MsgType);
	}
	else if ("location" == MsgType)
	{
		pwxobj = new WxMsgLocation(MsgType);
	}
	else if ("link" == MsgType)
	{
		pwxobj = new WxMsgLink(MsgType);
	}
	else if ("news" == MsgType)
	{
		pwxobj = new WxMsgNews(MsgType);
	}
	else if ("event" == MsgType)
	{
		string szEvent = GetStrFromJson(jv, "event");

		if ("subscribe" == szEvent || "unsubscribe" == szEvent || "SCAN" == szEvent)
		{
			pwxobj = new WxEventSubscribe(MsgType);
		}
		else if ("CLICK" == szEvent || "VIEW" == szEvent)
		{
			pwxobj = new WxEventMenu(MsgType);
		}
		else if ("LOCATION" == szEvent)
		{
			pwxobj = new WxEventLocation(MsgType);

		}
		else if ("MASSSENDJOBFINISH" == szEvent)
		{
			pwxobj = new WxEventMasssendjobfinish(MsgType);
		}
	}
	else if ("userinfo" == MsgType)
	{
		pwxobj = new WxUserInfo();
	}
	else if ("wxactoken" == MsgType)
	{
		pwxobj = new WxAccessTokenInfo();
	}

	if (pwxobj != NULL)
	{
		pwxobj->ParseFromJson(jv);
	}

	return pwxobj;
}

char * GetTimeStringMDAndHMS(unsigned long ntime, char *buff, char ymd /*= '-'*/, char hms /*= ':'*/)
{
	if (buff == NULL)
		return NULL;

	time_t tt;
	if (ntime == 0)
		tt = time(NULL);
	else
		tt = (time_t)(ntime);

	struct tm * ttm = localtime(&tt);
	if (ttm != NULL)
	{
		sprintf(buff, "%d%c%d %d%c%d%c%d", ttm->tm_mon + 1, ymd, ttm->tm_mday, ttm->tm_hour, hms, ttm->tm_min, hms, ttm->tm_sec);
	}
	else
	{
		sprintf(buff, "");
	}

	return buff;
}

char *GetContentBetweenString(const char *str, const char *sstart, const char * send, char *content)
{
	if (str == NULL || sstart == NULL || send == NULL || content == NULL)
		return NULL;

	strcpy(content, "");
	char *p1, *p2;

	p1 = (char*)strstr(str, sstart);
	if (p1 == NULL)
		return NULL;

	p2 = strstr(p1, send);
	if (p2 == NULL)
		strcpy(content, p1);
	else
	{
		int len0 = strlen(sstart);
		int len = p2 - p1 - len0;

		if (len > 0)
		{
			memcpy(content, p1 + len0, len);
			content[len] = '\0';
		}
	}

	return content;
}
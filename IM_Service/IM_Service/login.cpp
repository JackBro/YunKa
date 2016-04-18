#include "stdafx.h"
#include "login.h"
#include "./auth/http_parse.h"
#include "./AuthError.h"
#include "./common/commclt.h"
#include "./common/commsrv.h"

#define CLIENTVERSION	20000

CLogin::CLogin()
{
	m_server = _T("tcp01.tq.cn");
	m_port = 443;

	m_Socket.SetReceiveObject(this);

	m_sysConfig = new CSysConfigFile();

	LoadINIResource();
}

CLogin::~CLogin()
{
}

void CLogin::GetPreLoginInfo(tstring& loginName, tstring& password,	bool& isAutoLogin, bool& isKeepPwd)
{
	loginName = m_sysConfig->lastloginuid;
	password = m_sysConfig->lastloginpass;
	isAutoLogin = m_sysConfig->bAutoLogin;
	isKeepPwd = m_sysConfig->bSavePass;
}

void CLogin::StartLogin(tstring loginName, tstring password, bool isAutoLogin, bool isKeepPwd, CALL_BACK_PROGRESS pCallBack)
{
	m_isAutoLogin = isAutoLogin;
	m_isKeepPwd = isKeepPwd;
	m_loginName = loginName;
	m_password = password;
	m_callbackProgress = pCallBack;

	if (!CheckLoginInfo(m_loginName, m_password, m_isAutoLogin, m_isKeepPwd))
	{
		m_callbackProgress(-1);
		return;
	}

	unsigned int uin(0);

	if (IsNumber(loginName))
	{
		uin = atol(loginName.c_str());
	}

	// ��֤
	if (GetTqAuthToken(uin, loginName.c_str(), password.c_str()) != 1)
	{
		m_lastError = "��֤ʧ��";
		m_callbackProgress(-1);
		return;
	}

	m_callbackProgress(20);

	if (!IsNumber(loginName))
	{
		m_nLoginBy = LOGIN_BYSTRING;
	}
	else
	{
		m_nLoginBy = LOGIN_BYUID;
	}

	if (CheckLoginFlag(uin, loginName))
	{
		m_lastError = "���ʺ��ڱ����Ѿ���¼";
		m_callbackProgress(-1);
		return;
	}

	m_callbackProgress(40);

	// ��¼
	if (!LoginToRealServer(m_server, m_port, uin))
	{
		m_callbackProgress(-1);
	}
}

bool CLogin::CheckLoginInfo(tstring loginName, tstring password, bool isAutoLogin, bool isKeepPwd)
{
	char str[MAX_256_LEN];

	if (loginName.empty())
	{
		m_lastError = "��¼������Ϊ��!";
		return false;
	}
	
	if (password.length() > MAX_PASSWORD_LEN)
	{
		sprintf(str, "���볬�������ĳ��ȣ�%d���ַ�", MAX_PASSWORD_LEN);
		m_lastError = str;
		return false;
	}
	return true;
}

int CLogin::GetTqAuthToken(unsigned int &uin, const char *szStrid, const char *szPassWord)
{
	int nlen = MAX_4096_LEN;
	char recvbuf[MAX_4096_LEN] = { 0 };
	bool butf8(true);
	char myip[100];

	if (this->m_pTqAuthClient == NULL)
		m_pTqAuthClient = new CTqAuthClient(m_InitConfig.sAuthAddr, m_InitConfig.nAuthPort, VERSION);
	else if (strlen(m_szAuthtoken) > 0)
	{
		m_pTqAuthClient->Logout(m_szAuthtoken, recvbuf, nlen, butf8);
		m_szAuthtoken[0] = 0;
	}

	GetLocalHost(myip);

	nlen = 4096;
	int nstate = SESSION_ACTION_ERR_NOTCONNECT;
	if (0 != m_pTqAuthClient->Login(uin, szStrid, szPassWord, 1, myip, recvbuf, nlen, butf8))
	{
		return nstate;
	}

	CHttpParse  p;
	p.ParseHead(recvbuf, nlen);

	string t;
	if (!p.GetPostBodyParams(recvbuf, "state", t))
	{
		return nstate;
	}

	nstate = atoi(t.c_str());
	if (nstate != 1)
	{
		return nstate;
	}

	if (p.GetPostBodyParams(recvbuf, "token", t))
	{
		strncpy(m_szAuthtoken, t.c_str(), MAX_256_LEN);
	}

	if (p.GetPostBodyParams(recvbuf, "adminid", t))
	{
		m_authadminid = atol(t.c_str());
	}

	if (p.GetPostBodyParams(recvbuf, "uin", t))
	{
		uin = atol(t.c_str());
	}

	if (p.GetPostBodyParams(recvbuf, "unreadnoterecnum", t))
	{
		m_unreadnoterecnum = atol(t.c_str());
	}
	else
		m_unreadnoterecnum = 0;

	if (p.GetPostBodyParams(recvbuf, "unreadphonerecnum", t))
	{
		m_unreadphonerecnum = atol(t.c_str());
	}
	else
	{
		m_unreadphonerecnum = 0;
	}


	m_nSendAuthToken = 0;

	return nstate;
}

bool CLogin::LoadINIResource()
{
	char sFile[MAX_256_LEN], sKey[MAX_256_LEN];

	int i, len = MAX_256_LEN;

	sprintf(sFile, "%sTQConfig.ini", GetCurrentPath().c_str());

	LoadIniString("common", "programe name mini", m_InitConfig.sProgrameNameMini, len, sFile, NULL);

	LoadIniString("common", "help url", m_InitConfig.sHelpUrl, len, sFile);

	LoadIniString("common", "copy id", m_InitConfig.sCopyID, len, sFile);

	for (i = 0; i < 3; i++)
	{
		sprintf(sKey, "help about %d", i + 1);
		LoadIniString("common", sKey, m_InitConfig.sHelpAbout[i], len, sFile);

		sprintf(sKey, "help about url %d", i + 1);
		LoadIniString("common", sKey, m_InitConfig.sHelpAboutURL[i], len, sFile);
	}

	LoadIniInt("common", "display username type", m_InitConfig.nDisplayUserNameType, sFile);

	LoadIniString("server", "address", m_InitConfig.sDefaultServer, len, sFile);

	LoadIniInt("server", "port", m_InitConfig.nDefaultPort, sFile);

	LoadIniString("visitor", "address", m_InitConfig.sVisitorServer, len, sFile);

	LoadIniInt("visitor", "port", m_InitConfig.nVisitorServerPort, sFile);

	int temp(0);
	LoadIniInt("visitor", "ZlibEnabled", temp, sFile);
	m_InitConfig.bZlibEnabled = (1 == temp);

	for (i = 0; i < 10; i++)
	{
		sprintf(sKey, "list %d", i + 1);
		LoadIniString("server", sKey, m_InitConfig.sListServer[i], len, sFile);
	}

	LoadIniInt("login", "save pass", m_InitConfig.bSavePass, sFile);
	LoadIniInt("login", "auto login", m_InitConfig.bAutoLogin, sFile);

	//visitor filter
	LoadIniInt("visitor filter", "filter", m_InitConfig.visitorfilter, sFile, "0");
	LoadIniString("visitor filter", "info filter", m_InitConfig.infofilter, len, sFile, "");
	LoadIniString("visitor filter", "info filter", m_InitConfig.tailfilter, len, sFile, "");
	LoadIniString("AuthClient", "AuthAddr", m_InitConfig.sAuthAddr, len, sFile, "passport.tq.cn");
	LoadIniInt("AuthClient", "AuthPort", m_InitConfig.nAuthPort, sFile, "80");
	LoadIniString("WebPages", "logout", m_InitConfig.webpage_lgout, len, sFile, "http://vip.tq.cn/vip/logout.do");
	LoadIniString("WebPages", "webphone", m_InitConfig.webpage_webphone, len, sFile, "http://vip.tq.cn/vip/screenIndex.do?a=1");

	// ��ѯ����
	LoadIniString("WebPages", "querywebphone", m_InitConfig.webpage_querywebphone, len, sFile, "http://vip.tq.cn/vip/screenPhoneRecord.do?action=query&deal_state=0");

	// ����
	LoadIniString("WebPages", "note", m_InitConfig.webpage_note, len, sFile, "http://vip.tq.cn/vip/viewLiuyan.do?a=1");

	// �ͻ�����
	LoadIniString("WebPages", "crm", m_InitConfig.webpage_crm, len, sFile, "http://vip.tq.cn/vip/visitorInfomation.do?a=1");

	// ͳ�Ʒ���
	LoadIniString("WebPages", "analyze", m_InitConfig.webpage_analyze, len, sFile, "http://vip.tq.cn/vip/serviceStutasE.do?a=1");

	// ��������
	LoadIniString("WebPages", "mgmt", m_InitConfig.webpage_mgmt, len, sFile, "http://vip.tq.cn/vip/clientIndex.do?a=1");

	// ���㴰��
	LoadIniString("WebPages", "news", m_InitConfig.webpage_news, len, sFile, "http://vip.tq.cn/vip/rss/tq_mini_loading.jsp?a=");

	// ע�����û�
	LoadIniString("WebPages", "regnewuser", m_InitConfig.webpage_regnewuser, len, sFile, "http://www.tq.cn/vip/prenewqttclt.do");
	LoadIniInt("WebPages", "regnewuser_w", m_InitConfig.webpage_regnewuser_w, sFile, "520");
	LoadIniInt("WebPages", "regnewuser_h", m_InitConfig.webpage_regnewuser_h, sFile, "410");

	// ����ָ��
	LoadIniString("WebPages", "tqhelp", m_InitConfig.webpage_tqhelp, len, sFile, "http://www.tq.cn/help_3.html");

	// ͳһ����
	LoadIniString("WebPages", "UniIdioms", m_InitConfig.webpage_UniIdioms, len, sFile, "http://vip.tq.cn/vip/GetRequestXMLAction.do?cmd=NEWQUICKREPLY");

	// ʵʱ���
	LoadIniString("WebPages", "RtmMsg", m_InitConfig.webpage_RtmMsg, len, sFile, "http://vip.tq.cn/vip/realTime.do?a=");

	// ������Ϣ
	LoadIniString("WebPages", "SvrMsg", m_InitConfig.webpage_SvrMsg, len, sFile, "http://vip.tq.cn/vip/historyDialog.do?a=");

	// TQ����
	LoadIniString("WebPages", "TQadviser", m_InitConfig.webpage_TQadviser, len, sFile, "http://vip.tq.cn/vip/myTqGuWen.do?a=");

	// �ϴ�ͷ��
	LoadIniString("WebPages", "UploadHeadImage", m_InitConfig.webpage_UploadHeadImage, len, sFile, "http://vip.tq.cn/vip/preuploadfacelink.do?a=");

	// ����ͷ��
	LoadIniString("WebPages", "DownloadHeadImage", m_InitConfig.webpage_DownloadHeadImage, len, sFile, "http://vip.tq.cn/vip/preuploadfacelink.do?a=");

	// ����
	LoadIniString("WebPages", "FaceImage", m_InitConfig.webpage_FaceImage, len, sFile, "http://vip.tq.cn/clientimages/face/images");

	// ϵͳ��Ϣ
	LoadIniString("WebPages", "SysMessage", m_InitConfig.webpage_SysMessage, len, sFile, "http://sysmessage.tq.cn/SysMsg/getmesg.do?strid=%s&uin=%lu&departId=%d");

	// Ͷ�߽���
	LoadIniString("WebPages", "complaint", m_InitConfig.webpage_complaint, len, sFile, "http://211.151.52.62:8081/zhengquan/preLeaveMsg.do?uin=%lu");

	//�����ļ�
	LoadIniString("WebPages", "SendWebFileLink", m_InitConfig.webpage_SendWebFileLink, len, sFile, "http://vip.tq.cn/upload/upload2.do?version=100");


	LoadIniString("WebPages", "iconurl", m_InitConfig.webpage_iconurl, len, sFile, "http://sysimages.tq.cn/clientimages/face/ywt_face.html");
	LoadIniString("WebPages", "faqInfo", m_InitConfig.webpage_faqInfo, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=faqInfo");
	LoadIniString("WebPages", "faqSort", m_InitConfig.webpage_faqSort, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=faqSort");
	LoadIniString("WebPages", "faqAll", m_InitConfig.webpage_faqall, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=faqAll");
	LoadIniString("WebPages", "repickchat", m_InitConfig.webpage_repickchaturl, len, sFile, "http://106.120.108.230:8031/wxcreatechat?uin=%lu&signature=%s&timestamp=%s&nonce=%s&admiuin=%lu&%s&msg=hello");
	//��������evaluate
	LoadIniString("WebPages", "evaluate", m_InitConfig.webpage_evaluate, len, sFile, "http://106.120.108.230:8524/vip/scorebill.do?billid=%s&admin_uin=%u&action=scoreedit");

	LoadIniString("WebPages", "sendurl", m_InitConfig.webpage_sendurl, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=url");

	//��ȡ��˾�û�
	LoadIniString("WebPages", "companyuser", m_InitConfig.webpage_companyuser, len, sFile, "http://vip.tq.cn/vip/GetRequestXMLData?cmd=TRANSFER");

	// ��ȡ΢��token
	LoadIniString("WebPages", "accesstoken", m_InitConfig.webpage_accesstoken, len, sFile, "http://106.120.108.230:8031/wxkfgetac?uin=%lu&signature=%s&timestamp=%s&nonce=%s&{$MSG_WORKBILL.UserDefineParams}");

	//����
	LoadIniString("WebPages", "workbillurl", m_InitConfig.webpage_workbillurl, len, sFile, "http://106.120.108.230:8524/vip/workOrder.do?r=%s");

	// �ͻ�����
	LoadIniString("VisitorPages", "visitortail", m_InitConfig.visitorpage_visitortail, len, sFile, "http://vip.tq.cn/vip/ClientDialog.do?billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s");

	//�Ự�ǵ�
	LoadIniString("VisitorPages", "visitorbill", m_InitConfig.visitorpage_visitorbill, len, sFile, "http://vip.tq.cn/vip/recrodworkbill.do?action=prerecord&billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s");

	//�ͻ���Ϣ
	LoadIniString("VisitorPages", "visitorinfo", m_InitConfig.visitorpage_visitorinfo, len, sFile, "http://vip.tq.cn/vip/visitorinfo.do?billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s");
	//�¶���
	LoadIniString("VisitorPages", "visitororder", m_InitConfig.visitorpage_visitororder, len, sFile, "http://106.120.108.230:8524/vip/visitorinfo.do?billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s&clientid=%s&type=0");

	//ͨ����Ϣ
	LoadIniString("VisitorPages", "visitornotice", m_InitConfig.visitorpage_notice, len, sFile, "http://www.tq.cn");
	LoadIniString("WebPages", "RegisterAccount", m_InitConfig.registerAccount, len, sFile, "http://www.tq.cn");
	LoadIniString("WebPages", "ForgetPassword", m_InitConfig.forgetPassword, len, sFile, "http://www.tq.cn");
	LoadIniInt("WebPages", "ForgetPSVisible", m_InitConfig.forgetPSVisible, sFile);

	// ��ѯ��ʷ��¼
	LoadIniString("WebPages", "QueryHistoryRecords", m_InitConfig.query_history_records, len, sFile, "http://106.120.108.230:8524/vip/getbilllist.do?adminuin=%lu&kefu_uin=%lu&client_id=%s&weixin_id=%s&current_id=%s&timestamp=%s&token=%s&pagesize=1");

	// ΢�ŷ�����ý���ļ��ϴ�url
	LoadIniString("WebPages", "WeChatMediaUpload", m_InitConfig.wechat_media_upload, len, sFile, "http://file.api.weixin.qq.com/cgi-bin/media/upload?access_token=%s&type=%s");

	// ��Ѷ��ͼ��̬ͼurl
	LoadIniString("WebPages", "WeChatStaticMap", m_InitConfig.wechat_static_map, len, sFile, "http://apis.map.qq.com/ws/staticmap/v2/?key=JRYBZ-QIAWS-GJ3OB-6GXXF-F3WMZ-RNBGV&size=500x400&center=%s,%s&zoom=12");

	// ��Ѷ��ͼ�ص��עurl
	LoadIniString("WebPages", "WeChatMapLocation", m_InitConfig.wechat_map_location, len, sFile, "http://apis.map.qq.com/uri/v1/marker?marker=coord:%s,%s;title:%s;addr:%s&referer=myapp");

	// �Լ����ļ����������ص�΢�ŷ�������ý���ļ���url
	LoadIniString("WebPages", "FileServerMediaDownload", m_InitConfig.fileserver_media_download, len, sFile, "http://106.120.108.230:8531/media/get?access_token=%s&media_id=%s");

	// �Լ����ļ��������ϴ�url
	LoadIniString("WebPages", "FileServerMediaUpload", m_InitConfig.fileserver_media_upload, len, sFile, "http://106.120.108.230:8531/media/upload/%s");

	// ΢���ļ�����url
	LoadIniString("WebPages", "FileServerMediaTask", m_InitConfig.fileserver_media_task, len, sFile, "http://file.api.weixin.qq.com/cgi-bin/media/get?access_token=%s&media_id=%s");

	// ΢���ļ�����url
	LoadIniString("WebPages", "FileServerMediaFileId", m_InitConfig.fileserver_media_fileid, len, sFile, "http://wxm.tq.cn/media/%s");

	return true;
}

static HANDLE NameMutex = NULL;
static HANDLE UinMutex = NULL;
bool CLogin::CheckLoginFlag(unsigned long uin, const tstring& strID, bool bCreate, bool bFree)
{
	bool bRetVal = false;
	HANDLE hUinFlag = NULL, hNameFlag = NULL;

	if (uin != 0)
	{
		char strUin[MAX_256_LEN];
		sprintf_s(strUin, "_TQ_%lu_", uin);

		if (bCreate)
		{
			UinMutex = CreateMutex(NULL, FALSE, strUin);
		}
		else
		{
			hUinFlag = OpenMutex(MUTEX_ALL_ACCESS, FALSE, strUin);
		}

		if (hUinFlag)
		{
			bRetVal = TRUE;
			if (hUinFlag)
			{
				CloseHandle(hUinFlag);
				hUinFlag = NULL;
			}
		}
	}

	if (!strID.empty())
	{
		char strName[MAX_256_LEN];
		sprintf_s(strName, "_TQ_%s_", strID);

		if (bCreate)
		{
			NameMutex = CreateMutex(NULL, FALSE, strName);
		}
		else
		{
			hNameFlag = OpenMutex(MUTEX_ALL_ACCESS, FALSE, strName);
		}

		if (hNameFlag)
		{
			bRetVal = TRUE;
			if (hNameFlag)
			{
				CloseHandle(hNameFlag);
				hNameFlag = NULL;
			}
		}
	}

	if (bFree)
	{
		if (UinMutex)
		{
			CloseHandle(UinMutex);
			UinMutex = NULL;
		}

		if (NameMutex)
		{
			CloseHandle(NameMutex);
			NameMutex = NULL;
		}
	}

	return bRetVal;
}

bool CLogin::ReadSystemConfig()
{
	char sFile[MAX_256_LEN];
	tstring strFile = GetCurrentPath();

	sprintf(sFile, "%sconfig.dat", strFile.c_str());

	if (m_sysConfig->LoadData(sFile, CLIENTVERSION))
	{
	}
	else
	{
		SetSystemConfigByInitconfig();
	}

	m_sysConfig->strVisitorServer = m_InitConfig.sVisitorServer;
	m_sysConfig->nVisitorServerPort = m_InitConfig.nVisitorServerPort;
	return false;
}

void CLogin::SetSystemConfigByInitconfig()
{
	int i;

	m_sysConfig->strServer = m_InitConfig.sDefaultServer;
	m_sysConfig->nServerPort = m_InitConfig.nDefaultPort;

	for (i = 0; i<10; i++)
	{
		if (strlen(m_InitConfig.sListServer[i]) > 0)
		{
			m_sysConfig->m_listServerAddress.push_back(m_InitConfig.sListServer[i]);
		}
	}

	m_sysConfig->filtertype = m_InitConfig.visitorfilter;
	SolveUserdefineFilter(m_sysConfig->infofilter, m_InitConfig.infofilter);
}

void CLogin::SolveUserdefineFilter(FILTER_USERDEFINE &filter, char *sfilter)
{
	if (sfilter == NULL)
		return;

	char sbuff[MAX_256_LEN];

	char seps[] = ",";
	char *token;
	int i;

	memset(&filter, '\0', sizeof(FILTER_USERDEFINE));
	strncpy(sbuff, sfilter, MAX_256_LEN);
	token = strtok(sbuff, seps);
	i = 0;
	while (token != NULL)
	{
		if (i == 0)
		{
			filter.include = atol(token);
		}
		else
		{
			strncpy(filter.text[i - 1], token, MAX_STRINGFILTER_LEN - 2);
			filter.num++;
		}
		if (i > MAX_STRINGFILTER_NUM)
			break;

		token = strtok(NULL, seps);
		i++;
	}

	return;
}

bool CLogin::LoadUserConfig()
{
	//char sFile[MAX_256_LEN];
	tstring strFile = GetCurrentPath();

	//sprintf(sFile, "%sconfig_%u.dat", strFile, m_UserInfo.UserInfo.uid);
	//this->m_sysConfig->LoadUserData(sFile, CLIENTVERSION);

	return false;
}

bool CLogin::LoginToRealServer(tstring strServer, int nPort, unsigned int uin)
{
	int nError;

	if ((nError = ConnectToServer(strServer, nPort)) != 0)
		return false;

	m_callbackProgress(60);

	if ((nError = SendLoginInfo(uin)) != 0)
		return false;
	return true;
}

int CLogin::ConnectToServer(tstring sip, unsigned short port)
{
	int nError = 0;

	for (int i = 0; i < 2; i++)
	{
		if (m_Socket.Connect(sip.c_str(), port))
		{
			return nError;
		}
	}

	nError = SYS_ERROR_SOCKETCONNECTFAIL;
	return nError;
}

int CLogin::SendLoginInfo(unsigned int uin)
{
	int nError = 0;

	CLT_LOGON SendInfo(VERSION);


	SendInfo.uin = uin;
	strcpy(SendInfo.strid, "");
	SendInfo.type_loginby = LOGIN_BYUID;

	strcpy(SendInfo.pwd, m_password.c_str());

	SendInfo.status = STATUS_ONLINE;
	SendInfo.langtype = 0;
	SendInfo.bak = 0;


	SendInfo.terminaltype = TERMINAL_PCEX;
	SendInfo.connecttype = MSG_TCPUI;

	SendInfo.type_loginsort = 10;

	m_UserInfo.UserInfo.uid = uin;
	nError = SendPackTo(&SendInfo);
	if (nError != 0)
	{
		goto FAIL;
	}

FAIL:

	return nError;
}

int CLogin::SendPackTo(CPackInfo *pPackInfo, unsigned long recvuid /*= 0*/, unsigned long recvsock /*= 0*/, HWND hWnd /*= NULL*/)
{
	int nrtn = 0;

	TCP_PACK_HEADER TcpPackHead;
	int nError;
	int nPackHeadLen = COM_HEAD_PACK::GetHeadLen();
	int nTcpPackHeadLen = sizeof(TCP_PACK_HEADER);
	int nSendLen;

	if (pPackInfo == NULL)
	{
		nrtn = SYS_ERROR_MEMORY;
		goto RETURN;
	}

	pPackInfo->m_Head.SetValue(pPackInfo->version,
		m_usCltRand,
		m_UserInfo.UserInfo.uid,
		m_usSrvRand,
		pPackInfo->m_Head.head.cmd, 0,
		pPackInfo->m_Head.head.langtype,
		0,
		TERMINAL_PCEX);

	if (!(pPackInfo->packhead()))
	{
		nrtn = SYS_ERROR_PACKHEAD;
		goto RETURN;
	}
	if (!(pPackInfo->pack()))
	{
		nrtn = SYS_ERROR_PACKINFO;
		goto RETURN;
	}

	//���ܰ�  20090420
	pPackInfo->m_Pack.Encode();

	memset(&TcpPackHead, '\0', sizeof(TCP_PACK_HEADER));
	TcpPackHead.len = pPackInfo->m_Pack.GetPackLength();

	g_WriteLog.WriteLog(C_LOG_TRACE, "send pack cmd:%.4X", pPackInfo->m_Head.head.cmd);

	nSendLen = m_Socket.SendImPack((char *)(pPackInfo->m_Pack.GetPackBuff()), TcpPackHead, nError);
	if (nSendLen != TcpPackHead.len + nTcpPackHeadLen)
	{
		nrtn = SYS_ERROR_SENDFAIL;

		g_WriteLog.WriteLog(C_LOG_ERROR, "SendOnePack failed %d,fd=%d", TcpPackHead.len, m_Socket.getfd());
		goto RETURN;
	}

	nrtn = 0;

RETURN:
	return nrtn;
}

void CLogin::OnReceive(void* pHead, void* pData)
{
	int nError = 0;
	TCP_PACK_HEADER TcpPackHead;

	int nPackHeadLen = COM_HEAD_PACK::GetHeadLen();
	int nTcpPackHeadLen = sizeof(TCP_PACK_HEADER);
	int nErrType = 0;

	TcpPackHead = *((TCP_PACK_HEADER *)pHead);
	assert(TcpPackHead.len <= PACKMAXLEN);

	char *RecvBuf = (char*)pData;

	COM_HEAD_PACK Head;
	Head.head = *((PACK_HEADER*)RecvBuf);
	g_WriteLog.WriteLog(C_LOG_TRACE, "logondlg recv Cmd:%.4x", Head.head.cmd);

	switch (Head.head.cmd)
	{
	case CMD_SRV_CONF_LOGON:
		nError = RecvSrvConfLogon(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		if (nError != 0)
		{
			goto FAIL;
		}
		m_callbackProgress(80);
		break;
	case CMD_SRV_REP_USERINFO:
		nError = RecvSrvRepUserinfo(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);

		// �����¼��Ϣ
		if (nError != 0)
		{
			goto FAIL;
		}

		m_callbackProgress(100);
		break;
	case CMD_SRV_DENY_LOGON:
		nError = RecvSrvDenyLogon(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen, nErrType);

		m_Socket.Close();

		if (nError != 0)
		{
			goto FAIL;
		}
		else
		{
			nError = COMM_DENYLOGIN_SEG + nErrType;
			goto FAIL;
		}
		m_callbackProgress(-1);
		break;
	default:
		break;
	}

	return;

FAIL:
	//ConnectFail(nError);
	m_callbackProgress(-1);
	return;
}

int CLogin::RecvSrvConfLogon(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	SRV_CONF_LOGON RecvInfo(packhead.ver);
	int nError = 0;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		return nError;
	}

	m_sysConfig->strServer = m_server;
	m_sysConfig->nServerPort = m_port;

	m_sysConfig->strRealServer = m_server;
	m_sysConfig->nRealServerPort = m_port;

	m_UserInfo.UserInfo.uid = RecvInfo.uin;
	strcpy(m_UserInfo.UserInfo.sid, RecvInfo.strid);
	strcpy(m_UserInfo.UserInfo.pass, m_password.c_str());

	//���������ص�RecvInfo.type_loginbyΪ0����
	m_sysConfig->lastloginby = LOGIN_BYSTRING;
	if (strlen(RecvInfo.strid) <= 0)
		strcpy(m_sysConfig->lastloginstr, m_loginName.c_str());
	else
		strcpy(m_sysConfig->lastloginstr, RecvInfo.strid);
	strcpy(m_sysConfig->lastloginpass, m_password.c_str());


	m_sysConfig->AddServerAddress(m_sysConfig->strServer);

	m_usSrvRand = RecvInfo.m_Head.head.random;
	nError = 0;

	char lognmame[100] = { 0 };
	sprintf(lognmame, "_%s(%u)_emocss.log", RecvInfo.strid, RecvInfo.uin);
	g_WriteLog.InitLog(GetCurrentPath() + "log", lognmame, C_LOG_ALL);

	return nError;
}

int CLogin::RecvSrvRepUserinfo(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	SRV_REP_USERINFO RecvInfo(packhead.ver);
	int nError = 0;
	CUserObject *pUser;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		goto RETURN;
	}

	// ���ص�½�û���ͷ��

	nError = 0;
RETURN:

	return nError;
}

int CLogin::RecvSrvDenyLogon(PACK_HEADER packhead, char *pRecvBuff, int len, int &errtype)
{
	SRV_DENY_LOGON RecvInfo(packhead.ver);
	int nError = 0;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		goto RETURN;
	}

	errtype = RecvInfo.deny;

RETURN:

	return nError;
}

int CLogin::UnPack(CPackInfo *pPackInfo, char *buff, int len)
{
	int nError = 0;

	if (pPackInfo == NULL)
	{
		nError = SYS_ERROR_MEMORY;
		goto RETURN;
	}

	pPackInfo->m_Pack.Clear();

	pPackInfo->m_Pack.CopyBuff((unsigned char *)buff, len, 0);

	if (!(pPackInfo->unpack()))
	{
		nError = SYS_ERROR_UNPACKINFO;
		goto RETURN;
	}

	nError = 0;

RETURN:
	return nError;
}

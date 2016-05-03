#include "comobject.h"
#include  <io.h>
#include "comfunc.h"

CSysConfigFile::CSysConfigFile()
{
	ResetValue();
}


CSysConfigFile::~CSysConfigFile()
{
	
}

LOGIN_INFO* CSysConfigFile::GetLoginInfo(int type, unsigned long uid, string sid)
{
	LOGIN_INFO* loginInfo = NULL;

	if ((type == LOGIN_BYUID && uid != 0) || 
		((type == LOGIN_BYSTRING || type == LOGIN_BYMOBILE || type == LOGIN_BYMAIL) && !sid.empty()))
	{
		list<LOGIN_INFO*>::iterator iter;
		for (iter = m_listLogin.begin(); iter != m_listLogin.end(); ++iter)
		{
			if (type == LOGIN_BYUID)
			{
				if ((*iter)->uid == uid && uid != 0)
				{
					loginInfo = (*iter);
				}
			}
			else if (type == LOGIN_BYSTRING)
			{
				if ((*iter)->sid == sid)
				{
					loginInfo = (*iter);
				}
			}
		}
	}
	return loginInfo;
}

void CSysConfigFile::ResetValue()
{
	sprintf_s(m_sConfigname, "%sconfig.dat", GetCurrentPath().c_str());

	nStartFlag = FLAG_FILE_START;
	nEndFlag = FLAG_FILE_END;

	//nClientCopyInfo = ((CEmocssApp *)AfxGetApp())->nClientVersion;  //�汾��Ϣ 
	//nServerCopyInfo = ((CEmocssApp *)AfxGetApp())->nClientVersion;  //�汾��Ϣ 

	bSavePass = false;
	bAutoLogin = false;

	bAutoSearchKeyword = false;
	nKeySendType = 0;
	bIsFirstRun = true;
	dwLastUpdateTime = 0;
	nCopyCheckType = COPY_AUTOCHECK;
	nCopyCheckTimeMWD = 0;
	nCopyCheckTimeMonth = 1;
	nCopyCheckTimeWeek = 2;
	nCopyCheckTimeDay = 10;
	nCopyDownType = 0;			//0 ��̨���� 1 ��ʾ����

	nWndDock = WNDDOCK_NO;

	SetWndInitPos(true);
	bThickMainWnd = false;
	nInfoHeight = 100;
	nSendMsgHeight = 100;
	nPreSendMsgHeight = 35;

	//����¼������
	lastloginby = LOGIN_BYSTRING;

	this->lastloginuid = 0;
	//	lastsavepass = 0;
	strcpy_s(lastloginstr, "");
	strcpy_s(lastloginpass, "");

	lastloginstatus = STATUS_ONLINE;
	strcpy_s(lastloginsstatus, "");


	//����¼�ķ�������Ϣ
	nConnectType = CONNECT_UDP;	//��������
	strServer = "tcp01.tq.cn";//em.itoyou.cn gxl
	nServerPort = 443;

	//�ÿͽӴ��������ĵ�ַ
	strVisitorServer = "vipwebscreen.tq.cn";
	nVisitorServerPort = EMVISITOR_PORT;

	strRealServer = "tcp01.tq.cn";//em.itoyou.cn
	nRealServerPort = 443;//EM_PORT

	nProxyType = false;
	this->strProxyServer = "192.168.1.1";
	this->nProxyPort = PROXY_PORT;
	nProxyLocalPort = PROXY_LOCALPORT;

	m_listLogin.clear();
	m_listServerAddress.clear();

	m_listKeyWordSearchString.clear();

	m_listForbidWebUserSid.clear();

	filtertype = VISITORFILTER_ALL;
	memset(&infofilter, '\0', sizeof(FILTER_USERDEFINE));

	m_bAutoResp = false;
	m_strWellcomeMsg = "���ã��������߿ͷ����ܸ���Ϊ������";

	m_nUserTimeoutTime = 6;
	m_strUserTimeoutMsg = "���ã���������ʱ�뿪��������������ϵ��ʽ�����ԣ��һᾡ������ظ���";

	m_nVisitorTimeoutTime = 8;
	m_strVisitorTimeoutMsg = "��������ʲô���⾡�ܸ����ң��һᾡ������������⡣";

	m_bVisotorTimeoutClose = false;
	m_nVisitorTimeoutCloseTime = 10;
	m_strVisitorTimeoutCloseMsg = "�Ѿ��ܾ�û���յ�����ѶϢ�����������ڵ����������û���������⣬ϵͳ�������������ζԻ���<br>��������⣬��ӭ���ٴ�������ѯ��ף�����칤�������飬�ټ���";

	m_bAutoRespUnnormalStatus = false;
	m_strUnnormalStatusMsg[0].empty();
	m_strUnnormalStatusMsg[1].empty();
	m_strUnnormalStatusMsg[2].empty();
	m_strUnnormalStatusMsg[3].empty();
	m_strUnnormalStatusMsg[4].empty();
	m_strUnnormalStatusMsg[0] = "���ã���������ʱ�뿪��������������ϵ��ʽ�����ԣ��һᾡ������ظ���";
	m_strUnnormalStatusMsg[1] = "���ã���������ʱ�뿪��������������ϵ��ʽ�����ԣ��һᾡ������ظ���";

	m_strInviteWords = "���ã�����Ϊ������ʲô?";
	m_strInviteTitle = "��վ����Ա�������ͨ��";

	//�����Զ���������
	m_listInviteWords.push_back("��ӭ���٣�");
	m_listInviteWords.push_back("���ã���ӭ���ʱ���վ��");
	m_listInviteWords.push_back("�ܺ���������");
	m_listInviteWords.push_back("���ã���������ʲô�������ֱ�����ҽ�����");
	m_listInviteWords.push_back("���ã���Ϊ����Щʲô�أ�");
	m_listInviteWords.push_back("���ã�������վ����Ա��");

	m_nInviteType = INVITE_USERDEFINE;

	m_nKeywordsSort = 0;
	m_nKeywordsUser = 0;
	m_strKeywordsFind = "";

	//SetAllDefaultAlertInfo();
}

bool CSysConfigFile::LoadData(char *sFilename, unsigned short ver)
{
	if (sFilename != NULL && strlen(sFilename) == 0)
	{
		strcpy_s(m_sConfigname, sFilename);
	}

	if (_access(m_sConfigname, 0) == -1)
	{
		ResetValue();
		return false;
	}

	if (strServer.empty())
	{
		strServer = "tcp01.tq.cn";
	}

	if (nServerPort == 0)
	{
		nServerPort = 443;
	}

	return true;
}

ALERT_INFO *CSysConfigFile::SetAllDefaultAlertInfo(int type)
{
	/*DeleteAllAlertInfo();

	string strPath = GetCurrentPath();
	strPath.TrimRight("\\");
	strPath += "\\sound";

	ALERT_INFO *pInfo, *pRtn;

	pRtn = NULL;
	if (type == -1 || type == SOUND_USERONLINE)
	{
	pInfo = new ALERT_INFO;
	memset(pInfo, '\0', sizeof(ALERT_INFO));
	pInfo->type = SOUND_USERONLINE;
	pInfo->sound = 0;
	pInfo->tray = 0;
	pInfo->showwnd = 0;
	strcpy(pInfo->soundfilename, strPath + _T("\\online.wav"));
	m_ListAlertInfo.Add(pInfo);

	if (type != -1)
	{
	pRtn = pInfo;
	}
	}

	if (type == -1 || type == SOUND_INVITE)
	{
	pInfo = new ALERT_INFO;
	memset(pInfo, '\0', sizeof(ALERT_INFO));
	pInfo->type = SOUND_INVITE;
	pInfo->sound = 0;
	pInfo->tray = 0;
	pInfo->showwnd = 0;
	strcpy(pInfo->soundfilename, strPath + _T("\\invite.wav"));
	m_ListAlertInfo.Add(pInfo);
	if (type != -1)
	{
	pRtn = pInfo;
	}
	}

	if (type == -1 || type == SOUND_MSG)
	{
	pInfo = new ALERT_INFO;
	memset(pInfo, '\0', sizeof(ALERT_INFO));
	pInfo->type = SOUND_MSG;
	pInfo->sound = 0;
	pInfo->tray = 0;
	pInfo->showwnd = 0;
	strcpy(pInfo->soundfilename, strPath + _T("\\msg.wav"));
	m_ListAlertInfo.Add(pInfo);
	if (type != -1)
	{
	pRtn = pInfo;
	}
	}

	if (type == -1 || type == SOUND_RING)
	{
	pInfo = new ALERT_INFO;
	memset(pInfo, '\0', sizeof(ALERT_INFO));
	pInfo->type = SOUND_RING;
	pInfo->sound = 1;
	pInfo->tray = 1;
	pInfo->showwnd = 0;
	strcpy(pInfo->soundfilename, strPath + _T("\\ring.wav"));
	m_ListAlertInfo.Add(pInfo);
	if (type != -1)
	{
	pRtn = pInfo;
	}
	}

	if (type == -1 || type == SOUND_ALERT)
	{
	pInfo = new ALERT_INFO;
	memset(pInfo, '\0', sizeof(ALERT_INFO));
	pInfo->type = SOUND_ALERT;
	pInfo->sound = 1;
	pInfo->tray = 1;
	pInfo->showwnd = 0;
	strcpy(pInfo->soundfilename, strPath + _T("\\alert.wav"));
	m_ListAlertInfo.Add(pInfo);
	if (type != -1)
	{
	pRtn = pInfo;
	}
	}

	return pRtn;*/
	return NULL;
}

void CSysConfigFile::SetWndInitPos(bool bAlways)
{
	int scx, scy;
	scx = GetSystemMetrics(SM_CXSCREEN);
	scy = GetSystemMetrics(SM_CYSCREEN);

	if (bAlways ||
		((x < 0 || x > scx) || (y < 0 || y >scy) || (w < 0 || w > scx) || (h < 0 || h >scy)))
	{
		w = 264;
		h = 450;
		x = scx - w;
		y = (scy - h) / 2;
	}
}

void CSysConfigFile::AddServerAddress(string strServer, bool bMoveToTop)
{
	if (strServer.empty())
		return;

	list<string>::iterator iter;
	for (iter = m_listServerAddress.begin(); iter != m_listServerAddress.end(); ++iter)
	{
		if (*iter == strServer)
		{
			return;
		}
	}

	m_listServerAddress.push_back(strServer);
}

bool CSysConfigFile::IsWebuserSidForbid(string sid)
{
	if (sid.empty())
		return false;

	bool brtn = false;
	
	list<string>::iterator iter = m_listForbidWebUserSid.begin();

	for (iter; iter != m_listForbidWebUserSid.end(); iter++)
	{
		if ((*iter) == sid)
		{
			brtn = true;
			break;
		}
	}

	return brtn;
}

CUserObject::CUserObject()
{
	
}

CUserObject::~CUserObject()
{

}

bool CUserObject::Load(unsigned short ver)
{
	return false;
}

bool CUserObject::Save(unsigned short ver)
{
	return false;
}

bool CWebUserObject::Load(unsigned short ver)
{
	return false;
}

bool CWebUserObject::Save(unsigned short ver)
{
	return false;
}

void CWebUserObject::AddCommonTalkId(unsigned long uid)
{
	if (uid == NULL)
		return;

	if (IsExistCommonTalkId(uid))
		return;

	m_listCommonTalkId.push_back(uid);
}

bool CWebUserObject::IsExistCommonTalkId(unsigned long uid)
{
	list<unsigned long>::iterator iter = m_listCommonTalkId.begin();
	for (iter; iter != m_listCommonTalkId.end(); iter++)
	{
		if ((*iter) == uid)
		{
			return true;
		}
	}
	return false;
}

bool CWebUserObject::IsOnline()
{
	return false;
}

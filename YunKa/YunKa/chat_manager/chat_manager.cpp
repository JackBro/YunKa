#include "../stdafx.h"
#include "chat_manager.h"
#include "../chat_common/comclt.h"
#include "../chat_common/comsrv.h"
#include "../chat_common/comfloat.h"
#include "../chat_common/comform.h"
#include "../chat_common/comfunc.h"
#include "../chat_common/comdef.h"
#include "../chat_common/comstruct.h"
#include "chat_history_msg.h"
#include <cctype>
#include <algorithm>
using namespace std;

#define CLIENTVERSION	20000
#define  HASINSERTRIGHT   FALSE

unsigned short g_packSeq = 0;


CChatManager::CChatManager(IBaseMsgs* iBaseMsgs)
{
	m_isLoginSuccess = false;
	
	m_chatObjects = NULL;
	m_baseMsgs = iBaseMsgs;
	m_server = "tcp01.tq.cn";
	m_port = 443;
	m_usSrvRand = 0;
	m_usCltRand = (unsigned short)(rand() & 0xFFFF);
	m_tResentVisitPackTime = 0;
	m_socket.SetReceiveObject(this);
	m_login = new CLogin();
	m_login->m_manager = this;
	m_sysConfig = new CSysConfigFile();
	m_timers = new CTimerManager(CChatManager::TimerProc, this);
	m_msgId = 0;

	LoadINIResource();
}

CChatManager::~CChatManager()
{
	delete m_timers;
	delete m_sysConfig;
	delete m_login;
}


CSysConfig* CChatManager::GetSysConfig()
{
	return NULL;
}


CMsgs* CChatManager::GetMsgs()
{
	return NULL;
}

void CChatManager::ScreenCapture()
{

}

CChatObjects* CChatManager::GetChatObjects()
{
	if (m_isLoginSuccess)
	{
		m_chatObjects = new CChatObjects();
		m_chatObjects->m_login = m_login;
	}
	return m_chatObjects;
}

void CChatManager::GetPreLoginInfo(string& loginName, string& password, bool& isAutoLogin, bool& isKeepPwd)
{
	loginName = m_sysConfig->lastloginuid;
	password = m_sysConfig->lastloginpass;
	isAutoLogin = m_sysConfig->bAutoLogin;
	isKeepPwd = m_sysConfig->bSavePass;
}

void CChatManager::StartLogin(string loginName, string password, bool isAutoLogin, bool isKeepPwd)
{
	m_loginName = loginName;
	m_password = password;
	m_isAutoLogin = isAutoLogin;
	m_isKeepPwd = isKeepPwd;

	// ��ʼ��¼ʱ������һ���жϵ�¼��ʱ�Ķ�ʱ��
	m_timers->SetTimer(5000, TIMER_NAME_LOGIN);

	m_login->StartLogin(loginName, password, isAutoLogin, isKeepPwd);
}

bool CChatManager::ReadSystemConfig()
{
	char sFile[MAX_256_LEN];
	string strFile = GetCurrentPath();

	sprintf(sFile, "%sconfig.dat", strFile.c_str());

	if (m_sysConfig->LoadData(sFile, CLIENTVERSION))
	{
	}
	else
	{
		SetSystemConfigByInitconfig();
	}

	m_sysConfig->strVisitorServer = m_initConfig.sVisitorServer;
	m_sysConfig->nVisitorServerPort = m_initConfig.nVisitorServerPort;
	return false;
}

void CChatManager::SetSystemConfigByInitconfig()
{
	int i;

	m_sysConfig->strServer = m_initConfig.sDefaultServer;
	m_sysConfig->nServerPort = m_initConfig.nDefaultPort;

	for (i = 0; i<10; i++)
	{
		if (strlen(m_initConfig.sListServer[i]) > 0)
		{
			m_sysConfig->m_listServerAddress.push_back(m_initConfig.sListServer[i]);
		}
	}

	m_sysConfig->filtertype = m_initConfig.visitorfilter;
	SolveUserdefineFilter(m_sysConfig->infofilter, m_initConfig.infofilter);
}

void CChatManager::SolveUserdefineFilter(FILTER_USERDEFINE &filter, char *sfilter)
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

void CChatManager::OnReceive(void* pHead, void* pData)
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
	case CMD_SRV_CONF_LOGON: // ��¼ȷ��
		nError = RecvSrvConfLogon(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		if (nError != 0)
		{
			goto FAIL;
		}
		if (m_baseMsgs)
		{
			m_baseMsgs->LoginProgress(80);
		}
		break;
	case CMD_SRV_REP_USERINFO: // �û���Ϣ��
		nError = RecvSrvRepUserinfo(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);

		// �����¼��Ϣ
		if (nError != 0)
		{
			goto FAIL;
		}
		break;
	case CMD_SRV_DENY_LOGON: // �ܾ���¼
		nError = RecvSrvDenyLogon(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen, nErrType);

		if (nError != 0)
		{
			goto FAIL;
		}
		else
		{
			nError = COMM_DENYLOGIN_SEG + nErrType;
			goto FAIL;
		}
		break;
	case CMD_SRV_CONF_LOGOFF:   // �ǳ�ȷ�ϰ�
		nError = RecvSrvConfLogOff(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_STATUS_FRDONLINE: // ��������
		nError = RecvSrvStatusFrdOnline(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_STATUS_FRDOFFLINE: // ��������
		nError = RecvSrvStatusFrdOffline(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_STATUS_USER_FORM: // ���Ѹ�������״̬
		nError = RecvSrvStatusUserForm(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_SHARELIST: // �յ����Ѷ����б�
		nError = RecvFloatShareList(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CREATECHAT: // ����Ự
		nError = RecvFloatCreateChat(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CHATINFO: // �Ự��ϸ��Ϣ
		nError = RecvFloatChatInfo(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_COM_SEND_MSG:
		nError = RecvComSendMsg(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CHATMSG: // �Ự��Ϣ
		nError = RecvFloatChatMsg(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CHATMSG_ACK: // ������ϢӦ��
		nError = RecvFloatChatMsgAck(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_ACCEPTCHAT: // ���ܻỰ
		nError = RecvFloatAcceptChat(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_TRANSQUEST: // �ȴ�Ӧ��  �Ựת������
		nError = RecvFloatTransQuest(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_TRANSFAILED:   // �ȴ�Ӧ��  �Ựת��ʧ��
		nError = RecvFloatTransFailed(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_INVITE_REQUEST:// �ȴ�Ӧ��  ��������
		nError = RecvInviteRequest(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_INVITE_RESULT:// �ȴ�Ӧ��  ������
		nError = RecvInviteResult(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_KEFU_RELEASE: // ��ϯ�����ͷŻỰ
		nError = RecvFloatKefuRelease(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CMDERROR: // �ȴ�Ӧ��  ����ʧ��
		nError = RecvFloatCMDError(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CLOSECHAT: // �Ự�ر�
		nError = RecvFloatCloseChat(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_LISTCHAT: //�Ự�б�
		nError = RecvFloatListChat(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;	
	case CMD_EVENT_ANNOUNCEMENT: // �յ���Ϣͨ��
		nError = RecvEventAnnouncement(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_UPDATE_SUCC: //������Ϣ�ɹ�
		nError = RecvSrvUpdateSucc(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_UPDATE_FAIL: // ������Ϣʧ��
		nError = RecvSrvUpdateFail(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_SERVER_COPY: // �û�����ص�½
		nError = RecvSrvDonw(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_REP_TRANSFERCLIENT: // 440 ת����ʱ�û��ɹ�ʧ��
		nError = RecvRepTransferClient(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_TRANSFERCLIENT: // ����ת�ƻỰ���û�����
		nError = RecvTransferClient(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	default:
		break;
	}

	return;

FAIL:
	if (m_baseMsgs)
	{
		m_baseMsgs->LoginProgress(-1);
	}
	return;
}

int CChatManager::RecvSrvConfLogon(PACK_HEADER packhead, char *pRecvBuff, int len)
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

	m_userInfo.UserInfo.uid = RecvInfo.uin;
	strcpy(m_userInfo.UserInfo.sid, RecvInfo.strid);
	strcpy(m_userInfo.UserInfo.pass, m_password.c_str());

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

int CChatManager::RecvSrvRepUserinfo(PACK_HEADER packhead, char *pRecvBuff, int len)
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

	if (!m_isLoginSuccess)
	{
		// �յ���¼�û�����Ϣ������¼�ɹ�
		if (m_baseMsgs)
		{
			m_baseMsgs->LoginProgress(100);
		}
		m_nOnLineStatus = STATUS_ONLINE;
		m_isLoginSuccess = true;
		m_timers->SetTimer(1000, TIMER_NAME_NORMAL);

		pUser = AddUserObject(RecvInfo.uin, RecvInfo.strid, RecvInfo.UserInfo.info.nickname, STATUS_ONLINE, -1);

		SendTo_UpdateOnlineStatus(m_nOnLineStatus);
	}
	else
	{
		// ��¼�ɹ����յ��û���Ϣ������
		if (packhead.langtype == LANGUAGE_UTF8)
		{
			// ת��
			ConvertMsg(RecvInfo.UserInfo.info.nickname, sizeof(RecvInfo.UserInfo.info.nickname) - 1);
		}

		g_WriteLog.WriteLog(C_LOG_TRACE, "recvsrvrepuserinfo uid=%u,sid=%s,nickname=%s,username=%s,langtype=%u", RecvInfo.uin,
			RecvInfo.UserInfo.info.sid, RecvInfo.UserInfo.info.nickname, RecvInfo.UserInfo.info.username, RecvInfo.UserInfo.info.langtype);

		if (RecvInfo.uin > WEBUSER_UIN)
		{
			//pWebUser = GetWebUserObjectByID(RecvInfo.uin);
			//
			//if (pWebUser != NULL)
			//{
			//	strcpy(pWebUser->info.name, RecvInfo.UserInfo.info.nickname);
			//	m_pFormUser->InvalidateTreeItem((DWORD)pWebUser, pWebUser->info.name);
			//	if (strlen(pWebUser->info.name) < 2)
			//	{
			//		g_WriteLog.WriteLog(C_LOG_ERROR, "name length is null��RecvSrvRepUserinfo() name length��%d", strlen(pWebUser->info.name));
			//	}
			//}
		}
		else
		{
			ONLINEFLAGUNION onlineflag;
			onlineflag.online_flag = RecvInfo.UserInfo.info.onlineflag;


			pUser = GetUserObjectByUid(RecvInfo.uin);
			if (pUser == NULL)
			{
				pUser = AddUserObject(RecvInfo.uin, RecvInfo.strid, RecvInfo.UserInfo.info.nickname, onlineflag.stStatus.nOnlineStatus, -1);

				if (pUser == NULL)
				{
					goto RETURN;
				}

				pUser->m_nFlag = 1;
			}
			else
			{
				if (pUser->status != onlineflag.stStatus.nOnlineStatus
					|| strcmp(pUser->UserInfo.nickname, RecvInfo.UserInfo.info.nickname) != 0)
				{
					bool bexpand = (pUser->status != onlineflag.stStatus.nOnlineStatus);

					pUser->status = onlineflag.stStatus.nOnlineStatus;
					strcpy(pUser->UserInfo.nickname, RecvInfo.UserInfo.info.nickname);
					
				}
			}

			m_baseMsgs->RecvOneUserInfo(pUser);

			// ����ͷ��
			//pUser->DownLoadFace();
		}
	}
	

	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvSrvDenyLogon(PACK_HEADER packhead, char *pRecvBuff, int len, int &errtype)
{
	m_socket.Close();

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

int CChatManager::UnPack(CPackInfo *pPackInfo, char *buff, int len)
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

bool CChatManager::LoadINIResource()
{
	char sFile[MAX_256_LEN], sKey[MAX_256_LEN];

	int i, len = MAX_256_LEN;

	sprintf(sFile, "%sTQConfig.ini", GetCurrentPath().c_str());

	LoadIniString("common", "programe name mini", m_initConfig.sProgrameNameMini, len, sFile, NULL);

	LoadIniString("common", "help url", m_initConfig.sHelpUrl, len, sFile);

	LoadIniString("common", "copy id", m_initConfig.sCopyID, len, sFile);

	for (i = 0; i < 3; i++)
	{
		sprintf(sKey, "help about %d", i + 1);
		LoadIniString("common", sKey, m_initConfig.sHelpAbout[i], len, sFile);

		sprintf(sKey, "help about url %d", i + 1);
		LoadIniString("common", sKey, m_initConfig.sHelpAboutURL[i], len, sFile);
	}

	LoadIniInt("common", "display username type", m_initConfig.nDisplayUserNameType, sFile);

	LoadIniString("server", "address", m_initConfig.sDefaultServer, len, sFile);

	LoadIniInt("server", "port", m_initConfig.nDefaultPort, sFile);

	LoadIniString("visitor", "address", m_initConfig.sVisitorServer, len, sFile);

	LoadIniInt("visitor", "port", m_initConfig.nVisitorServerPort, sFile);

	int temp(0);
	LoadIniInt("visitor", "ZlibEnabled", temp, sFile);
	m_initConfig.bZlibEnabled = (1 == temp);

	for (i = 0; i < 10; i++)
	{
		sprintf(sKey, "list %d", i + 1);
		LoadIniString("server", sKey, m_initConfig.sListServer[i], len, sFile);
	}

	LoadIniInt("login", "save pass", m_initConfig.bSavePass, sFile);
	LoadIniInt("login", "auto login", m_initConfig.bAutoLogin, sFile);

	//visitor filter
	LoadIniInt("visitor filter", "filter", m_initConfig.visitorfilter, sFile, "0");
	LoadIniString("visitor filter", "info filter", m_initConfig.infofilter, len, sFile, "");
	LoadIniString("visitor filter", "info filter", m_initConfig.tailfilter, len, sFile, "");
	LoadIniString("AuthClient", "AuthAddr", m_initConfig.sAuthAddr, len, sFile, "passport.tq.cn");
	LoadIniInt("AuthClient", "AuthPort", m_initConfig.nAuthPort, sFile, "80");
	LoadIniString("WebPages", "logout", m_initConfig.webpage_lgout, len, sFile, "http://vip.tq.cn/vip/logout.do");
	LoadIniString("WebPages", "webphone", m_initConfig.webpage_webphone, len, sFile, "http://vip.tq.cn/vip/screenIndex.do?a=1");

	// ��ѯ����
	LoadIniString("WebPages", "querywebphone", m_initConfig.webpage_querywebphone, len, sFile, "http://vip.tq.cn/vip/screenPhoneRecord.do?action=query&deal_state=0");

	// ����
	LoadIniString("WebPages", "note", m_initConfig.webpage_note, len, sFile, "http://vip.tq.cn/vip/viewLiuyan.do?a=1");

	// �ͻ�����
	LoadIniString("WebPages", "crm", m_initConfig.webpage_crm, len, sFile, "http://vip.tq.cn/vip/visitorInfomation.do?a=1");

	// ͳ�Ʒ���
	LoadIniString("WebPages", "analyze", m_initConfig.webpage_analyze, len, sFile, "http://vip.tq.cn/vip/serviceStutasE.do?a=1");

	// ��������
	LoadIniString("WebPages", "mgmt", m_initConfig.webpage_mgmt, len, sFile, "http://vip.tq.cn/vip/clientIndex.do?a=1");

	// ���㴰��
	LoadIniString("WebPages", "news", m_initConfig.webpage_news, len, sFile, "http://vip.tq.cn/vip/rss/tq_mini_loading.jsp?a=");

	// ע�����û�
	LoadIniString("WebPages", "regnewuser", m_initConfig.webpage_regnewuser, len, sFile, "http://www.tq.cn/vip/prenewqttclt.do");
	LoadIniInt("WebPages", "regnewuser_w", m_initConfig.webpage_regnewuser_w, sFile, "520");
	LoadIniInt("WebPages", "regnewuser_h", m_initConfig.webpage_regnewuser_h, sFile, "410");

	// ����ָ��
	LoadIniString("WebPages", "tqhelp", m_initConfig.webpage_tqhelp, len, sFile, "http://www.tq.cn/help_3.html");

	// ͳһ����
	LoadIniString("WebPages", "UniIdioms", m_initConfig.webpage_UniIdioms, len, sFile, "http://vip.tq.cn/vip/GetRequestXMLAction.do?cmd=NEWQUICKREPLY");

	// ʵʱ���
	LoadIniString("WebPages", "RtmMsg", m_initConfig.webpage_RtmMsg, len, sFile, "http://vip.tq.cn/vip/realTime.do?a=");

	// ������Ϣ
	LoadIniString("WebPages", "SvrMsg", m_initConfig.webpage_SvrMsg, len, sFile, "http://vip.tq.cn/vip/historyDialog.do?a=");

	// TQ����
	LoadIniString("WebPages", "TQadviser", m_initConfig.webpage_TQadviser, len, sFile, "http://vip.tq.cn/vip/myTqGuWen.do?a=");

	// �ϴ�ͷ��
	LoadIniString("WebPages", "UploadHeadImage", m_initConfig.webpage_UploadHeadImage, len, sFile, "http://vip.tq.cn/vip/preuploadfacelink.do?a=");

	// ����ͷ��
	LoadIniString("WebPages", "DownloadHeadImage", m_initConfig.webpage_DownloadHeadImage, len, sFile, "http://vip.tq.cn/vip/preuploadfacelink.do?a=");

	// ����
	LoadIniString("WebPages", "FaceImage", m_initConfig.webpage_FaceImage, len, sFile, "http://vip.tq.cn/clientimages/face/images");

	// ϵͳ��Ϣ
	LoadIniString("WebPages", "SysMessage", m_initConfig.webpage_SysMessage, len, sFile, "http://sysmessage.tq.cn/SysMsg/getmesg.do?strid=%s&uin=%lu&departId=%d");

	// Ͷ�߽���
	LoadIniString("WebPages", "complaint", m_initConfig.webpage_complaint, len, sFile, "http://211.151.52.62:8081/zhengquan/preLeaveMsg.do?uin=%lu");

	//�����ļ�
	LoadIniString("WebPages", "SendWebFileLink", m_initConfig.webpage_SendWebFileLink, len, sFile, "http://vip.tq.cn/upload/upload2.do?version=100");


	LoadIniString("WebPages", "iconurl", m_initConfig.webpage_iconurl, len, sFile, "http://sysimages.tq.cn/clientimages/face/ywt_face.html");
	LoadIniString("WebPages", "faqInfo", m_initConfig.webpage_faqInfo, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=faqInfo");
	LoadIniString("WebPages", "faqSort", m_initConfig.webpage_faqSort, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=faqSort");
	LoadIniString("WebPages", "faqAll", m_initConfig.webpage_faqall, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=faqAll");
	LoadIniString("WebPages", "repickchat", m_initConfig.webpage_repickchaturl, len, sFile, "http://106.120.108.230:8031/wxcreatechat?uin=%lu&signature=%s&timestamp=%s&nonce=%s&admiuin=%lu&%s&msg=hello");
	//��������evaluate
	LoadIniString("WebPages", "evaluate", m_initConfig.webpage_evaluate, len, sFile, "http://106.120.108.230:8524/vip/scorebill.do?billid=%s&admin_uin=%u&action=scoreedit");

	LoadIniString("WebPages", "sendurl", m_initConfig.webpage_sendurl, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=url");

	//��ȡ��˾�û�
	LoadIniString("WebPages", "companyuser", m_initConfig.webpage_companyuser, len, sFile, "http://vip.tq.cn/vip/GetRequestXMLData?cmd=TRANSFER");

	// ��ȡ΢��token
	LoadIniString("WebPages", "accesstoken", m_initConfig.webpage_accesstoken, len, sFile, "http://106.120.108.230:8031/wxkfgetac?uin=%lu&signature=%s&timestamp=%s&nonce=%s&{$MSG_WORKBILL.UserDefineParams}");

	//����
	LoadIniString("WebPages", "workbillurl", m_initConfig.webpage_workbillurl, len, sFile, "http://106.120.108.230:8524/vip/workOrder.do?r=%s");

	// �ͻ�����
	LoadIniString("VisitorPages", "visitortail", m_initConfig.visitorpage_visitortail, len, sFile, "http://vip.tq.cn/vip/ClientDialog.do?billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s");

	//�Ự�ǵ�
	LoadIniString("VisitorPages", "visitorbill", m_initConfig.visitorpage_visitorbill, len, sFile, "http://vip.tq.cn/vip/recrodworkbill.do?action=prerecord&billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s");

	//�ͻ���Ϣ
	LoadIniString("VisitorPages", "visitorinfo", m_initConfig.visitorpage_visitorinfo, len, sFile, "http://vip.tq.cn/vip/visitorinfo.do?billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s");
	//�¶���
	LoadIniString("VisitorPages", "visitororder", m_initConfig.visitorpage_visitororder, len, sFile, "http://106.120.108.230:8524/vip/visitorinfo.do?billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s&clientid=%s&type=0");

	//ͨ����Ϣ
	LoadIniString("VisitorPages", "visitornotice", m_initConfig.visitorpage_notice, len, sFile, "http://www.tq.cn");
	LoadIniString("WebPages", "RegisterAccount", m_initConfig.registerAccount, len, sFile, "http://www.tq.cn");
	LoadIniString("WebPages", "ForgetPassword", m_initConfig.forgetPassword, len, sFile, "http://www.tq.cn");
	LoadIniInt("WebPages", "ForgetPSVisible", m_initConfig.forgetPSVisible, sFile);

	// ��ѯ��ʷ��¼
	LoadIniString("WebPages", "QueryHistoryRecords", m_initConfig.query_history_records, len, sFile, "http://106.120.108.230:8524/vip/getbilllist.do?adminuin=%lu&kefu_uin=%lu&client_id=%s&weixin_id=%s&current_id=%s&timestamp=%s&token=%s&pagesize=1");

	// ΢�ŷ�����ý���ļ��ϴ�url
	LoadIniString("WebPages", "WeChatMediaUpload", m_initConfig.wechat_media_upload, len, sFile, "http://file.api.weixin.qq.com/cgi-bin/media/upload?access_token=%s&type=%s");

	// ��Ѷ��ͼ��̬ͼurl
	LoadIniString("WebPages", "WeChatStaticMap", m_initConfig.wechat_static_map, len, sFile, "http://apis.map.qq.com/ws/staticmap/v2/?key=JRYBZ-QIAWS-GJ3OB-6GXXF-F3WMZ-RNBGV&size=500x400&center=%s,%s&zoom=12");

	// ��Ѷ��ͼ�ص��עurl
	LoadIniString("WebPages", "WeChatMapLocation", m_initConfig.wechat_map_location, len, sFile, "http://apis.map.qq.com/uri/v1/marker?marker=coord:%s,%s;title:%s;addr:%s&referer=myapp");

	// �Լ����ļ����������ص�΢�ŷ�������ý���ļ���url
	LoadIniString("WebPages", "FileServerMediaDownload", m_initConfig.fileserver_media_download, len, sFile, "http://106.120.108.230:8531/media/get?access_token=%s&media_id=%s");

	// �Լ����ļ��������ϴ�url
	LoadIniString("WebPages", "FileServerMediaUpload", m_initConfig.fileserver_media_upload, len, sFile, "http://106.120.108.230:8531/media/upload/%s");

	// ΢���ļ�����url
	LoadIniString("WebPages", "FileServerMediaTask", m_initConfig.fileserver_media_task, len, sFile, "http://file.api.weixin.qq.com/cgi-bin/media/get?access_token=%s&media_id=%s");

	// ΢���ļ�����url
	LoadIniString("WebPages", "FileServerMediaFileId", m_initConfig.fileserver_media_fileid, len, sFile, "http://wxm.tq.cn/media/%s");

	return true;
}

int CChatManager::SendPackTo(CPackInfo *pPackInfo, unsigned long recvuid, unsigned long recvsock, HWND hWnd)
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
		m_userInfo.UserInfo.uid,
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

	nSendLen = m_socket.SendImPack((char *)(pPackInfo->m_Pack.GetPackBuff()), TcpPackHead, nError);
	if (nSendLen != TcpPackHead.len + nTcpPackHeadLen)
	{
		nrtn = SYS_ERROR_SENDFAIL;

		g_WriteLog.WriteLog(C_LOG_ERROR, "SendOnePack failed %d,fd=%d", TcpPackHead.len, m_socket.getfd());
		goto RETURN;
	}

	nrtn = 0;

RETURN:
	return nrtn;
}

bool CChatManager::LoadUserConfig()
{
	//char sFile[MAX_256_LEN];
	string strFile = GetCurrentPath();

	//sprintf(sFile, "%sconfig_%u.dat", strFile, m_userInfo.UserInfo.uid);
	//this->m_sysConfig->LoadUserData(sFile, CLIENTVERSION);

	return false;
}

int CChatManager::RecvSrvConfLogOff(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	return 0;
}

int CChatManager::RecvSrvStatusFrdOnline(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	SRV_STATUS_FRDONLINE RecvInfo(packhead.ver);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0 || RecvInfo.m_Head.head.random != m_usSrvRand)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "recvsrvstatusfrdonline unpack failed");
		return nError;
	}

	if (packhead.langtype == LANGUAGE_UTF8)
	{
		ConvertMsg(RecvInfo.nickname, sizeof(RecvInfo.nickname) - 1);
	}

	if (RecvInfo.uin > WEBUSER_UIN)
	{
		//�ÿͶԻ���ʼ
		pWebUser = GetWebUserObjectByUid(RecvInfo.uin);
		if (pWebUser == NULL)
		{
			//GetWebUserID(RecvInfo.uin,NULL);
		}
	}
	else
	{

		pUser = GetUserObjectByUid(RecvInfo.uin);
		if (pUser == NULL)
		{
			//���ﲻ���Ǻܸ��ӣ�������µ��û�����Ҫ�Զ���ˢ�����£��Ժ�����ʵʱ�ش���
			pUser = AddUserObject(RecvInfo.uin, "", RecvInfo.nickname, RecvInfo.status, -1);

			pUser->status = STATUS_ONLINE;
			strcpy(pUser->UserInfo.nickname, RecvInfo.nickname);

			//this->m_pFormUser->AddNewUserToTree(pUser);
			pUser->m_nFlag = 1;

			//if (m_pUserAndAuthDlg != NULL && m_pUserAndAuthDlg->IsWindowVisible())
			//{
			//	m_pUserAndAuthDlg->InitialSetup();
			//}
			//
			//m_pFormUser->ExpandUserItem(pUser);
		}
		else
		{
			if (pUser->status == STATUS_OFFLINE)
			{
				pUser->status = STATUS_ONLINE;
				strcpy(pUser->UserInfo.nickname, RecvInfo.nickname);

				//m_pFormUser->MoveUserToOnlineItem(pUser, true);
			}
			else
			{
				if (strcmp(pUser->UserInfo.nickname, RecvInfo.nickname) != 0 || pUser->status != RecvInfo.status)
				{
					bool bexpand = (pUser->status != STATUS_ONLINE);

					pUser->status = STATUS_ONLINE;	//RecvInfo.status;
					strcpy(pUser->UserInfo.nickname, RecvInfo.nickname);
					//m_pFormUser->InvalidateTreeItem((DWORD)pUser, pUser->GetUserDisplayName());

					//if (bexpand)
						//m_pFormUser->ExpandUserItem(pUser);
				}
			}
		}

		if (pUser->UserInfo.uid == m_userInfo.UserInfo.uid)
		{
			m_nOnLineStatus = RecvInfo.status;
		}

		m_baseMsgs->RecvOnline(pUser);
	}

	nError = 0;

	return nError;
}

int CChatManager::RecvSrvStatusFrdOffline(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	SRV_STATUS_FRDOFFLINE RecvInfo(packhead.ver);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0 || RecvInfo.m_Head.head.random != m_usSrvRand)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "recvsrvstatusfrdoffline unpack failed");
		return nError;
	}
	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvSrvStatusFrdOffline uin:%u,externid:%u, bak:%u",
		RecvInfo.uin, RecvInfo.externid, RecvInfo.bak);
	if (RecvInfo.uin > WEBUSER_UIN)
	{
		//�ÿͶԻ�����
		pWebUser = GetWebUserObjectByUid(RecvInfo.uin);
		if (pWebUser != NULL)
		{
			pUser = GetUserObjectByUid(pWebUser->talkuid);

			pWebUser->m_bNewComm = false;
			pWebUser->cTalkedSatus = HASTALKED;
			pWebUser->transferuid = 0;
			if (pWebUser->m_bConnected)
			{
				//SendWebuserTalkEnd(pWebUser);

				pWebUser->m_bConnected = true;
				pWebUser->m_nWaitTimer = -20;
				pWebUser->onlineinfo.talkstatus = TALKSTATUS_NO;

				//m_pFormMain->RecvWebUserStartTalk(pWebUser, false);
				//m_pFormUser->MoveWebUserToOnlineItem(pWebUser, pWebUser->IsOnline());
			}
			//�����Ự��Ҫ���û�Ⱥ�б���ɾ��
			//DelWebUserInMultiList(pWebUser);
		}
	}
	else
	{
		pUser = GetUserObjectByUid(RecvInfo.uin);
		if (pUser != NULL)
		{
			bool bexpand = false;
			if (pUser->status != STATUS_OFFLINE)
			{
				bexpand = true;
				pUser->m_bConnected = true;
				pUser->talkstatus = TALKSTATUS_NO;
				pUser->status = STATUS_OFFLINE;//RecvInfo.status;

				//m_pFormUser->RemoveUserAllActiveTalkUser(pUser);
				//m_pFormUser->MoveUserToOnlineItem(pUser, false);
			}

			//m_pFormUser->InvalidateTreeItem((DWORD)pUser, pUser->GetUserDisplayName());
			//if (bexpand)
				//m_pFormUser->ExpandUserItem(pUser);

			if (pUser->UserInfo.uid == m_userInfo.UserInfo.uid)
			{
				g_WriteLog.WriteLog(C_LOG_TRACE, "RecvSrvStatusFrdOffline--SetOfflineStatus uid%u", m_userInfo.UserInfo.uid);
				m_nOnLineStatus = STATUS_OFFLINE;
				m_nOnLineStatusEx = STATUS_OFFLINE;
			}
		}
		m_baseMsgs->RecvOffline(pUser);
	}

	nError = 0;

	return nError;
}

int CChatManager::RecvSrvStatusUserForm(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	SRV_STATUS_USER_FORM RecvInfo(packhead.ver);
	int nError = 0;
	CUserObject *pUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "recvsrvstatususerform unpack failed");
		return nError;
	}

	pUser = GetUserObjectByUid(RecvInfo.uin);
	if (pUser != NULL)
	{
		pUser->status = RecvInfo.GetOnlineStatus();
		//m_pFormUser->InvalidateTreeItem((DWORD)pUser, pUser->GetUserDisplayName());

		//m_pFormUser->ExpandUserItem(pUser);
	}

	m_baseMsgs->RecvUserStatus(pUser);

	nError = 0;

	return nError;
}

int CChatManager::RecvFloatShareList(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_SHARELIST RecvInfo(packhead.ver);
	int nError = 0;
	CUserObject *pUser = NULL;

	std::vector<SHAREUSEROBJ>::iterator iter;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatShareList unpack failed,Cmd:%.4x", packhead.cmd);
		return nError;
	}

	ONLINEFLAGUNION onlineflag;
	string szsharelsit, sztemp;
	for (iter = RecvInfo.sharememlist.begin(); iter != RecvInfo.sharememlist.end(); iter++)
	{
		SHAREUSEROBJ ShareUserOb = (SHAREUSEROBJ)(*iter);

		onlineflag.online_flag = ShareUserOb.onlineflag;

		pUser = GetUserObjectByUid(ShareUserOb.uin);
		if (pUser == NULL)
		{
			pUser = AddUserObject(ShareUserOb.uin, "", "", onlineflag.stStatus.nOnlineStatus, -1);
			//pUser->DownLoadFace();
		}

		//sztemp.Format("uin=%u:OnlineStatus=%d,", ShareUserOb.uin, onlineflag.stStatus.nOnlineStatus);
		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatShareList:%s", sztemp);
		pUser->m_bFriend = true;
	}

	SendTo_GetAllUserInfo();

	nError = 0;

	return nError;
}

int CChatManager::RecvComSendMsg(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_SEND_MSG RecvInfo(packhead.ver);
	int nError = 0;
	int msgType = MSG_TYPE_NORMAL;
	int msgFrom = MSG_FROM_USER;
	char msgTime[MAX_256_LEN];
	CWebUserObject *pWebUser = NULL;
	CUserObject *pUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);

	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendMsg unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvComSendMsg chatid:%s,Rand:%s,Thirdid:%s,recvuin:%u,senduin:%u,strfontinfo:%s,msgtype:%d,msg:%s",
		RecvInfo.strChatid, RecvInfo.strRand, RecvInfo.strThirdid, RecvInfo.msg.recvuin,
		RecvInfo.msg.senduin, RecvInfo.msg.strfontinfo, RecvInfo.msg.msgtype, RecvInfo.msg.strmsg);

	//����ʾ���ݣ� ���ƶ�item
	if (packhead.langtype == 4)
	{
		ConvertMsg(RecvInfo.msg.strmsg, sizeof(RecvInfo.msg.strmsg) - 1);

		ConvertMsg(RecvInfo.msg.strmobile, sizeof(RecvInfo.msg.strmobile) - 1);
	}

	if (RecvInfo.msg.senduin > WEBUSER_UIN)
	{
		//�ÿͷ�����Ϣ
		switch (RecvInfo.msg.msgtype)
		{
		case MSG_WORKBILL:
		case MSG_CLIENT_COME:
			RecvComSendWorkBillMsg(RecvInfo.msg.senduin, RecvInfo.msg.recvuin, RecvInfo.msg.strmsg, RecvInfo.msg.strmobile);
			break;
		case MSG_WEBUI_SEND_CHAT_HISMSG:   //�յ�webui�������ĻỰ��ʷ��Ϣ�����ܶ����,��Ҫ�ͻ��˷�������MSG_KEFU_GET_CHAT_HISMSG
			RecvComSendNormalChatidHisMsg(RecvInfo.msg.senduin, RecvInfo.msg.recvuin, RecvInfo);
			break;
		case MSG_TRANSFER_REQUEST://�Ựת������
			RecvComTransRequest(packhead.uin, RecvInfo);
			break;
		case MSG_TRANSFER_ANSWER://�Ựת������
			RecvComTransAnswer(packhead.uin, RecvInfo);
			break;
		case MSG_NORMAL:
		case MSG_WX:
			pWebUser = GetWebUserObjectByUid(RecvInfo.msg.senduin);
			if (pWebUser == NULL)
			{
				//��Ҫȥ��������ȡ�÷ÿ���Ϣ
				string l_msg = RecvInfo.msg.strmsg;
				if (l_msg != "null")
				{
					SendTo_GetWebUserInfo(RecvInfo.msg.senduin, RecvInfo.strChatid);

					int pos = l_msg.find("\"msgtype\":\"userinfo\"");
					int pos1 = l_msg.find("\"msgtype\":\"wxactoken\"");
					if (pos < 0 && pos1 < 0)
					{
						SaveEarlyMsg(&(RecvInfo.msg));
					}
					else
					{
						g_WriteLog.WriteLog(C_LOG_ERROR, "userinfo��Ϣ������Ϣ��¼bug��%s", RecvInfo.msg.strmsg);
					}
				}
			}
			else
			{
				if (strcmp(RecvInfo.msg.strfontinfo, "JSON=WX") == 0)
				{
					//΢����Ϣ���ͣ�utf8��������
					WxObj *pwxobj = ParseWxJsonMsg(RecvInfo.msg.strmsg);

					if (pwxobj != NULL)
					{
						pWebUser->m_bIsFrWX = true;
						if ("userinfo" == pwxobj->MsgType)
						{
							// �˴�ע�⣬����ڶ����յ�userinfo��Ӧ����ǰ���յ��Ǵ���Ϣ������
							if (pWebUser->m_pWxUserInfo == NULL)
							{
								pWebUser->m_pWxUserInfo = (WxUserInfo*)pwxobj;
							}
							else
							{
								delete pWebUser->m_pWxUserInfo;
								pWebUser->m_pWxUserInfo = (WxUserInfo*)pwxobj;
							}
							nError = true;
							pWebUser->m_bIsGetInfo = true;
							// �˴�΢���û���Ϣ�����������沢����ʹ��
							goto RETURN;
						}
						else if ("wxactoken" == pwxobj->MsgType)
						{
							//todo:�յ���������������΢�ŵ�access_token��Ϣ
							WxTokens::iterator iter = m_mapTokens.find(RecvInfo.strThirdid);
							if (iter == m_mapTokens.end())
							{
								m_mapTokens.insert(map<string, string>::value_type(RecvInfo.strThirdid, ((WxAccessTokenInfo*)pwxobj)->szAcToken));
							}
							else
							{
								iter->second = ((WxAccessTokenInfo*)pwxobj)->szAcToken;
							}
							delete pwxobj;
							nError = true;
							goto RETURN;
						}
					}
					else
					{
						g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendMsg NOT the weixin Json:%s", RecvInfo.msg.strmsg);
						goto RETURN;
					}
				}
				//��ҳ�����ĻỰ������chatid��仯����Ҫ��ʱ����
				if (strcmp(pWebUser->chatid, RecvInfo.strChatid) != 0 && strlen(RecvInfo.strChatid) > 0)
					strcpy(pWebUser->chatid, RecvInfo.strChatid);
				pWebUser->m_bNewComm = false;

				if (strstr(RecvInfo.msg.strmsg, "[ϵͳ��Ϣ]") != NULL
					&& strstr(RecvInfo.msg.strmsg, "����վ���ÿ���") != NULL)
				{
					SendTo_GetWebUserInfo(RecvInfo.msg.senduin, RecvInfo.strChatid);
				}

				if (pWebUser->info.name[0] == '\0')
				{
					strcpy(pWebUser->info.name, RecvInfo.msg.strmobile);
					if (strlen(pWebUser->info.name) < 2)
					{
						g_WriteLog.WriteLog(C_LOG_ERROR, "name length is null��RecvComSendMsg1() name length��%d", strlen(pWebUser->info.name));
					}
				}
				else if (!pWebUser->info.nameflag && strcmp(RecvInfo.msg.strmobile, pWebUser->info.name) != 0)
				{
					if (RecvInfo.msg.strmobile[0] != '\0')
					{
						strcpy(pWebUser->info.name, RecvInfo.msg.strmobile);
					}
					else
					{
						if (strlen(pWebUser->info.name) < 2)
						{
							g_WriteLog.WriteLog(C_LOG_ERROR, "name length is null��RecvComSendMsg2() name length��%d", strlen(pWebUser->info.name));
						}
					}
				}

				if (m_sysConfig->IsWebuserSidForbid(pWebUser->info.sid))
				{
					//��Ϣ�����ˣ�ֱ������������
					goto RETURN;
				}

				if (!(strstr(RecvInfo.msg.strmsg, "[ϵͳ��Ϣ]") != NULL && strstr(RecvInfo.msg.strmsg, "�����뿪") != NULL))
				{
					pWebUser->m_resptimeoutmsgtimer = -1;
					pWebUser->m_resptimeoutclosetimer = -1;
					if (pWebUser->m_waitresptimeouttimer < 0)
						pWebUser->m_waitresptimeouttimer = 0;	// �ÿͷ�˵���� [12/29/2010 SC]

					m_baseMsgs->RecvOneMsg(pWebUser, msgFrom, GetMsgId(), msgType, MSG_DATA_TYPE_TEXT, 
						RecvInfo.msg.strmsg,msgTime, NULL, NULL, "");

					if ((RecvInfo.msg.bak == MSG_BAK_NORMAL) && !pWebUser->m_bConnected)
					{
						SendWebuserTalkBegin(pWebUser);
						pWebUser->cTalkedSatus = INTALKING;
						pWebUser->talkuid = m_userInfo.UserInfo.uid;

						pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
						pWebUser->transferuid = 0;
						pWebUser->m_nWaitTimer = 0;
						pWebUser->m_bConnected = TRUE;
						//�ÿ����ˣ��϶���������
						if (pWebUser->info.status == STATUS_OFFLINE)
							pWebUser->info.status = STATUS_ONLINE;

						// ��ʾ
					}
					else if (RecvInfo.msg.bak == MSG_BAK_NORMAL)
					{
						pWebUser->cTalkedSatus = INTALKING;

						// ��ʾ 
					}

					if (m_sysConfig->m_bAutoRespUnnormalStatus)
					{
						string strResp;

						switch (m_nOnLineStatus)
						{
						case STATUS_BUSY:
						case STATUS_WORK:
						case STATUS_REFUSE_NEWWEBMSG:
							strResp = m_sysConfig->m_strUnnormalStatusMsg[0];
							break;
						case STATUS_LEAVE:
						case STATUS_EATING:
						case STATUS_OFFLINE:
							strResp = m_sysConfig->m_strUnnormalStatusMsg[1];
							break;
						}

						if (!strResp.empty())
						{
							SendAutoRespMsg(pWebUser, strResp.c_str());
						}
					}
				}
				else
				{
					pWebUser->cTalkedSatus = HASTALKED;
					SendWebuserTalkEnd(pWebUser);
					if (pWebUser->m_bConnected)
					{
						pWebUser->m_bConnected = FALSE;
						pWebUser->m_nWaitTimer = -20;
						pWebUser->onlineinfo.talkstatus = TALKSTATUS_NO;
					}
				}

				pWebUser->m_nWaitTimer = 0;
			}
			break;
		default:
			break;
		}
	}
	else
	{
		//�ͷ�������Ϣ
		pUser = GetUserObjectByUid(RecvInfo.msg.senduin);
		if (pUser == NULL)
		{
			pUser = AddUserObject(RecvInfo.msg.senduin, "", RecvInfo.msg.strmobile, STATUS_ONLINE, -1);
			// ���ظ���ϯ��ͷ��
			//pUser->DownLoadFace();
			if (pUser == NULL)
			{
				goto RETURN;
			}
			pUser->m_nWaitTimer = 0;

			m_baseMsgs->RecvOneMsg(pUser, msgFrom, GetMsgId(), msgType, MSG_DATA_TYPE_TEXT, RecvInfo.msg.strmsg,
				msgTime, NULL, NULL, "");
			if ((RecvInfo.msg.bak == MSG_BAK_NORMAL || RecvInfo.msg.bak == MSG_BAK_AUTOANSER) && !(pUser->m_bInnerTalk))
			{
				pUser->m_bInnerTalk = true;
			}
		}
		else
		{
			strncpy(pUser->UserInfo.nickname, RecvInfo.msg.strmobile, MAX_USERNAME_LEN);//�˴��᷵�ء�ϵͳ������
			pUser->m_nWaitTimer = 0;

			m_baseMsgs->RecvOneMsg(pUser, msgFrom, GetMsgId(), msgType, MSG_DATA_TYPE_TEXT, RecvInfo.msg.strmsg, msgTime, NULL, NULL, "");

			if ((RecvInfo.msg.bak == MSG_BAK_NORMAL || RecvInfo.msg.bak == MSG_BAK_AUTOANSER) && !(pUser->m_bInnerTalk))
			{
				pUser->m_bInnerTalk = true;
			}
		}
		
		// ��ʾ
	}

RETURN:
	SendAckEx(RecvInfo.msg.seq, RecvInfo.msg.senduin, RecvInfo.sendip, RecvInfo.sendport);
	nError = 0;
	return 0;
}

int CChatManager::RecvFloatCreateChat(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_CREATECHAT RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "recvfloatcreatechat unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}
	else
	{
		g_WriteLog.WriteLog(C_LOG_TRACE, "**RecvFloatCreateChat uAdminId:%u,usort:%hu,uWebuin:%u,chatid:%s,clienttid:%s,webname:%s,uKefu:%u,uFromAdmin:%u,uFromSort:%hu",
			RecvInfo.uAdminId, RecvInfo.usort, RecvInfo.uWebuin, RecvInfo.chatid, RecvInfo.clienttid, RecvInfo.webname, RecvInfo.uKefu, RecvInfo.uFromAdmin, RecvInfo.uFromSort);
	}
	if (0 == RecvInfo.clienttid[0])
	{
		//����cookie��û��clientid�Ի�����
		sprintf(RecvInfo.clienttid, "%lu", RecvInfo.uWebuin);
		g_WriteLog.WriteLog(C_LOG_TRACE, "recvfloatcreatechat clientid is empty,set as webuin:%u", RecvInfo.uWebuin);
	}
	pWebUser = GetWebUserObjectBySid(RecvInfo.clienttid);
	if (pWebUser == NULL)
	{
		pWebUser = AddWebUserObject(RecvInfo.clienttid, RecvInfo.thirdid, RecvInfo.webname, "", "", STATUS_ONLINE, 0);
		pWebUser->m_onlinetime = 0;
		pWebUser->m_bIsShow = false;
	}

	if (RecvInfo.webname[0] != '\0' && strcmp(RecvInfo.webname, pWebUser->info.name) != 0)
	{
		strcpy(pWebUser->info.name, RecvInfo.webname);
	}

	//GetWxUserInfoAndToken(pWebUser);

	pWebUser->cTalkedSatus = INTALKING;
	if (RecvInfo.uFromAdmin == 0)
	{
		pWebUser->nVisitFrom = WEBUSERICON_DIRECTURL;
	}
	else if (RecvInfo.uFromAdmin == m_login->m_authAdminid)
	{

	}
	else
	{
		pWebUser->nVisitFrom = WEBUSERICON_UNIONURL;
	}

	pWebUser->webuserid = RecvInfo.uWebuin;
	if (strcmp(pWebUser->chatid, RecvInfo.chatid) != 0 && strlen(RecvInfo.chatid) > 0)
		strcpy(pWebUser->chatid, RecvInfo.chatid);
	pWebUser->floatadminuid = RecvInfo.uAdminId;
	pWebUser->floatfromadminuid = RecvInfo.uFromAdmin;
	pWebUser->floatfromsort = RecvInfo.uFromSort;
	pWebUser->gpid = packhead.random;
	pWebUser->m_bNewComm = true;
	pWebUser->m_sNewSeq = packhead.sendrandom; //������Ϣid
	pWebUser->info.status = STATUS_ONLINE;
	pWebUser->talkuid = 0;//�ȴ�Ӧ������ĻỰ����û�н���ͷ�
	if (RecvInfo.uKefu == 0)
	{
		if (pWebUser->onlineinfo.talkstatus != TALKSTATUS_REQUEST)
		{
			pWebUser->onlineinfo.talkstatus = TALKSTATUS_REQUEST;
			pWebUser->m_nWaitTimer = 0;

			string str;
			//str = "%s �û�����Ի�!" + pWebUser->info.name;
			

			// ��ʾ��??
		}
	}
	else
	{
		//pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;

		//pUser = GetUserObjectByUid(RecvInfo.uKefu);

		//if (pUser != NULL &&
		//	(pUser->UserInfo.uid == m_userInfo.UserInfo.uid || pUser->m_bFriend)
		//	)
		//{

		//	pWebUser->transferuid = 0;

		//	if (RecvInfo.uKefu == m_userInfo.UserInfo.uid)
		//	{
		//		//���Լ�����ķÿ�
		//		pWebUser->m_nWaitTimer = 0;
		//		pWebUser->m_bConnected = true;
		//	}
		//	else
		//	{
		//		//����������ķÿ�
		//		pWebUser->m_nWaitTimer = -20;
		//		pWebUser->m_bConnected = HASINSERTRIGHT;
		//	}

		//	pWebUser->talkuid = RecvInfo.uKefu;

		//	//��ȡ�Ự��ʷ��Ϣ���������µ���Ϣ
		//	StartRecvFloatMsg(packhead.random, RecvInfo.uAdminId, RecvInfo.chatid, pWebUser->m_sNewSeq);

		//	//�ÿ�����
		//	m_pFormMain->RecvUserAcceptChat(pWebUser, pUser);
		//	g_WriteLog.WriteLog(C_LOG_TRACE, "Recvfloatcreatechat kefuName��%s,webuserName:%s", pUser->UserInfo.nickname, pWebUser->info.name);
		//	//�ÿ��Ƶ��ͷ��ĶԻ��б�
		//	//m_pFormUser->MoveWebUserToUserTalkItem(pWebUser, pUser);
		//	int nRet = this->m_pFormUser->MoveWebUserToUserTalkItem(pWebUser, pUser);
		//	if (nRet < 0)
		//		m_pFormUser->m_TreeListUser.DeleteItemByLParam((LPARAM)pWebUser);
		//	else if (RecvInfo.uKefu == m_userInfo.UserInfo.uid)
		//		AddMultiWebUserToList(pWebUser);//�ѽ���ĻỰ�ƶ����Ự�б���

		//}
	}

	if (RecvInfo.memlist.find(m_userInfo.UserInfo.uid) != RecvInfo.memlist.end())
		pWebUser->m_bNotResponseUser = 0;
	else
		pWebUser->m_bNotResponseUser = 1;

	m_baseMsgs->RecvCreateChat(pWebUser);

	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvFloatChatInfo(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_CHATINFO RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;
	std::vector<unsigned int>::iterator iter;
	char strMsg[MAX_256_LEN];

	strcpy(strMsg, "waiter");

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "recvfloatchatinfo unpack failed,Cmd:%.4x", packhead.cmd);

		goto RETURN;
	}
	else
	{
		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatChatInfo uAdminId:%u,usort:%hu,uWebuin:%u,chatid:%s,clienttid:%s,webname:%s,uKefu:%u,uFromAdmin:%u,uFromSort:%hu",
			RecvInfo.uAdminId, RecvInfo.sSort, RecvInfo.uWebUin, RecvInfo.chatid, RecvInfo.strClientId, RecvInfo.webnickname, RecvInfo.uKefuUin, RecvInfo.uFromAmdind, RecvInfo.sFromSort);

	}

	pWebUser = GetWebUserObjectByUid(RecvInfo.uWebUin);
	if (pWebUser == NULL)
	{
		pWebUser = AddWebUserObject(RecvInfo.strClientId, RecvInfo.strThirdId, RecvInfo.webnickname, "", "", STATUS_UNDEFINE, 0);
		pWebUser->webuserid = RecvInfo.uWebUin;
	}
	else
	{
		//ChangeWebUserSid(pWebUser, RecvInfo.strClientId, RecvInfo.strThirdId);
	}

	// ֻҪ�յ��Ự��Ϣ��һ��ȥ��������ȡwork_bill��
	//GetWebUserID(pWebUser->webuserid, RecvInfo.chatid, strMsg.GetBuffer(0));
	if (pWebUser->m_nEMObType != OBJECT_WEBUSER)
	{
		pWebUser->m_nEMObType = OBJECT_WEBUSER;
	}
	pWebUser->m_bNewComm = true;
	pWebUser->cTalkedSatus = INTALKING;

	pWebUser->floatadminuid = RecvInfo.uAdminId;
	if (strcmp(pWebUser->chatid, RecvInfo.chatid) != 0 && strlen(RecvInfo.chatid) > 0)
		strcpy(pWebUser->chatid, RecvInfo.chatid);

	pWebUser->floatfromadminuid = RecvInfo.uFromAmdind;
	pWebUser->floatfromsort = RecvInfo.sFromSort;
	pWebUser->gpid = packhead.random;

	if (RecvInfo.uKefuUin == 0)
	{
		//�ȴ�Ӧ��
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_REQUEST;
		pWebUser->info.status = STATUS_ONLINE;
		pWebUser->m_nWaitTimer = 0;

		//m_pFormMain->RecvWebUserInvite(pWebUser, "");
		//m_pFormUser->MoveWebUserToWaitItem(pWebUser);//�ȴ�Ӧ��ģ��ƶ����ȴ�Ӧ�����
		//StartRecvFloatMsg(packhead.random, RecvInfo.uAdminId, RecvInfo.chatid, pWebUser->m_sNewSeq);
	}
	else
	{
		if (m_nNextInviteWebuserUid == pWebUser->webuserid)
		{
			pWebUser->onlineinfo.talkstatus = TALKSTATUS_INVITE;
			pWebUser->inviteuid = m_userInfo.UserInfo.uid;
			pWebUser->info.status = STATUS_ONLINE;
			pWebUser->frominviteuid = m_nNextInviteUid;

			pWebUser->m_nWaitTimer = 0;

			CUserObject *pInviteUser = GetUserObjectByUid(m_nNextInviteUid);

			//�����������htmleditor����ʾ��Ȼ�����ƶ�λ�ã���Ϊ�ƶ�λ�ÿ��ܻᵼ��������л��������ط�ͬ������
			//this->m_pFormMain->SendInviteToWebUser(pInviteUser, pWebUser, INVITE_HELP, m_userInfo);
			//m_pFormUser->MoveWebUserToUserInviteItem(pWebUser, m_userInfo);

			m_nNextInviteWebuserUid = 0;
			m_nNextInviteUid = 0;
		}
		else
		{
			pUser = NULL;
			if (m_userInfo.UserInfo.uid != RecvInfo.uKefuUin)
			{
				for (int i = 0; i < RecvInfo.InviteUin.size(); ++i)
				{
					if (RecvInfo.InviteUin[i] == m_userInfo.UserInfo.uid)
					{
						pUser = GetUserObjectByUid(m_userInfo.UserInfo.uid);
						pWebUser->onlineinfo.bInvited = true;
						break;
					}
				}
			}

			if (pUser == NULL)
			{
				pUser = GetUserObjectByUid(RecvInfo.uKefuUin);
				pWebUser->onlineinfo.bInvited = false;
			}

			if (RecvInfo.uTansferingToKefu == 0)
			{
				if (pUser != NULL)
				{
					if (pUser->UserInfo.uid == m_userInfo.UserInfo.uid ||
						pWebUser->onlineinfo.bInvited)
					{
						//���Լ�ת�Ƶķÿ�
						pWebUser->m_nWaitTimer = 0;
						pWebUser->m_bConnected = true;
					}
					else
					{
						//������ת�Ƶķÿ�
						pWebUser->m_bConnected = false;
						pWebUser->m_nWaitTimer = -20;
					}


					//���ڶԻ�
					pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
					pWebUser->info.status = STATUS_ONLINE;
					pWebUser->transferuid = 0;



					pWebUser->talkuid = pUser->UserInfo.uid;//�����û���Ҫ�ı�talkidΪ�Լ���

					//�ÿ�ת��
					//m_pFormMain->RecvUserAcceptChat(pWebUser, pUser);

					//�ÿ��Ƶ��ͷ��ĶԻ��б�
					//int nRet = m_pFormUser->MoveWebUserToUserTalkItem(pWebUser, pUser);
					//if (nRet < 0)
					//m_pFormUser->m_TreeListUser.DeleteItemByLParam((LPARAM)pWebUser);
					//else if (pUser->UserInfo.uid == m_userInfo.UserInfo.uid)
					//AddMultiWebUserToList(pWebUser);//�ѽ���ĻỰ�ƶ����Ự�б���


					if (RecvInfo.uKefuUin&&pUser->m_bFriend
						|| !RecvInfo.uKefuUin&&!pWebUser->m_bNotResponseUser)
					{
						//��ȡ�Ự��ʷ��Ϣ���������µ���Ϣ
						//StartRecvFloatMsg(packhead.random, RecvInfo.uAdminId, RecvInfo.chatid, pWebUser->m_sNewSeq);
					}
				}
			}
			else
			{
				//����ת��
				pWebUser->onlineinfo.talkstatus = TALKSTATUS_TRANSFER;
				pWebUser->info.status = STATUS_ONLINE;
				pWebUser->transferuid = RecvInfo.uTansferingToKefu;
				CUserObject *pAcceptUser = GetUserObjectByUid(RecvInfo.uTansferingToKefu);

				//�����������htmleditor����ʾ��Ȼ�����ƶ�λ�ã���Ϊ�ƶ�λ�ÿ��ܻᵼ��������л��������ط�ͬ������
				if (strcmp(pWebUser->chatid, RecvInfo.chatid) != 0 && strlen(RecvInfo.chatid) > 0)
					strcpy(pWebUser->chatid, RecvInfo.chatid);
				pWebUser->floatadminuid = RecvInfo.uAdminId;

				//if (pUser != NULL)
				//	m_pFormMain->SendInviteToWebUser(pUser, pWebUser, INVITE_TRANSFER, pAcceptUser);
				//else if (pAcceptUser != NULL)
				//	m_pFormMain->RecvTransferWebUser(pWebUser, pAcceptUser->UserInfo.nickname);

				//m_pFormUser->DeleteChildItem((DWORD)pUser, TREELPARAM_USER_TALK, (DWORD)pWebUser);
				//m_pFormUser->MoveWebUserToUserTransferItem(pWebUser, pAcceptUser);


				//���ﴦ���ǲ�����Ҫ�û��ֶ�����
				if (RecvInfo.uTansferingToKefu == this->m_userInfo.UserInfo.uid)
				{

				}
			}
		}
	}

	pWebUser->m_bNotResponseUser = 1;
	for (iter = RecvInfo.webRecvUin.begin(); iter != RecvInfo.webRecvUin.end(); iter++)
	{
		unsigned long uid = (unsigned long)(*iter);

		if (uid == m_userInfo.UserInfo.uid)
		{
			pWebUser->m_bNotResponseUser = 0;
			break;
		}
	}

	for (iter = RecvInfo.InviteUin.begin(); iter != RecvInfo.InviteUin.end(); iter++)
	{
		unsigned long uid = (unsigned long)(*iter);

		pWebUser->AddCommonTalkId(uid);
	}

	m_baseMsgs->RecvChatInfo(pWebUser);

	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvFloatChatMsg(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	if (packhead.uin == m_userInfo.UserInfo.uid)
		return 0;

	COM_FLOAT_CHATMSG RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	int msgType = MSG_TYPE_NORMAL;
	int msgFrom = MSG_FROM_USER;
	char msgTime[MAX_256_LEN];
	WxMsgBase* msgContentWx = NULL;
	CUserObject *pAssistUser = NULL;
	CWebUserObject *pWebUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "recvfloatchatmsg unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}

	pWebUser = GetWebUserObjectByUid(RecvInfo.webuin);
	if (pWebUser == NULL)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "recvfloatchatmsg getwebuserobjectbyid(%u) failed", RecvInfo.webuin);
		goto RETURN;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatChatMsg chatid:%s,uAdminId:%u,uWebuin:%u,msgtype:%d,sendname:%s,strfontinfo:%s,msg:%s",
		RecvInfo.chatid, RecvInfo.uAdminId, RecvInfo.webuin, RecvInfo.nMsgType, RecvInfo.nickname, RecvInfo.strfontinfo, RecvInfo.strmsg);

	if (pWebUser->m_sWxAppid.empty()) //WxAppidΪ�գ�ȥ����˻�ȡ
	{
		SendTo_GetWebUserInfo(pWebUser->webuserid, RecvInfo.chatid);
	}

	WxMsgBase* pWxMsg = NULL;
	if (strcmp(RecvInfo.strfontinfo, "JSON=WX") == 0)
	{
		pWebUser->m_bIsFrWX = true;
		pWxMsg = ParseWxMsg(pWebUser, RecvInfo);
	}
	else
	{
		std::string content = RecvInfo.strmsg;

		bool isUrl = false;// PathIsURL(content.c_str());
		if (isUrl && pWebUser->m_bIsFrWX)
		{
			content = "<a href=\"" + content + "\" target=\"blank\">" + content + "</a>";
		}

		TransforFaceMsg(content);
		strncpy(RecvInfo.strmsg, content.c_str(), MAX_MSG_RECVLEN);
	}

	if (RecvInfo.strRand[0] != 0)
	{
		strcpy(pWebUser->info.sid, RecvInfo.strRand);
	}

	if (RecvInfo.strThirdid[0] != 0)
	{
		strcpy(pWebUser->info.thirdid, RecvInfo.strThirdid);
	}

	if (packhead.uin != RecvInfo.webuin)
	{
		// ����΢���û���������Ϣ������Э������������Ϣ
		pAssistUser = GetUserObjectByUid(packhead.uin);
		if (pAssistUser == NULL)
		{
			// ����uin���ǳ��ȳ�ʼ����ϯ����Ϣ
			pAssistUser = AddUserObject(packhead.uin, "", RecvInfo.nickname, STATUS_ONLINE, -1);

			// Ȼ����¸���ϯ��������Ϣ
			//GetUserInfoFromSrv(packhead.uin);
		}
	}

	if (packhead.langtype == LANGUAGE_UTF8)
	{
		ConvertMsg(RecvInfo.strmsg, sizeof(RecvInfo.strmsg) - 1);

		ConvertMsg(RecvInfo.nickname, sizeof(RecvInfo.nickname) - 1);
	}
	//�ÿͷ�����Ϣ
	switch (RecvInfo.nMsgType)
	{
	case MSG_FLOAT_NORMAL:
	case MSG_FLOAT_PRV:
	case MSG_FLOAT_INPUTING:
	case MSG_WX://΢����Ϣ 
	{
		if (pWebUser->m_nWaitTimer<0)
		{
			pWebUser->m_nWaitTimer = 0;
		}

		// �Ŷӵȴ����ܺ���Ϣ�������� [12/9/2010 SC]
		if (m_sysConfig->IsWebuserSidForbid(pWebUser->info.sid))
		{
			//��Ϣ�����ˣ�ֱ������������
			goto RETURN;
		}

		if (RecvInfo.nMsgType == 2)
		{
			string strMsg = RecvInfo.strmsg;
			transform(strMsg.begin(), strMsg.end(), strMsg.begin(), ::toupper);
			if (strMsg.find("userfile") > -1 && strMsg.find("�յ��ļ�") > -1)
			{
				// �Ӵ�����ϯ��΢�Ŷ��󷢵�					
				if (strMsg.find(".jpg") > -1 || strMsg.find(".jpeg") > -1 || strMsg.find(".bmp") > -1 || strMsg.find(".png") > -1)
				{
					RecvInfo.nMsgDataType = MSG_DATA_TYPE_IMAGE;
				}
				else
				{
					RecvInfo.nMsgDataType = MSG_DATA_TYPE_FILE;
				}
			}
			else if (strMsg.find("http:") > -1 && (strMsg.find(".jpg") > -1 || strMsg.find(".jpeg") > -1 ||
				strMsg.find(".bmp") > -1 || strMsg.find(".png") > -1) || strMsg.find("�յ�һ��ͼƬ") > -1)
			{
				RecvInfo.nMsgDataType = MSG_DATA_TYPE_IMAGE;
			}
			else if (strMsg.find("userfile") > -1)
			{
				// �Ӵ�����ϯ��web�û�����
				RecvInfo.nMsgDataType = MSG_DATA_TYPE_FILE;
			}

			if (pAssistUser == NULL)
			{
				// �ÿͷ�����Ϣʱ����δӦ��ȴ�ʱ������
				pWebUser->m_resptimeoutmsgtimer = -1;
				pWebUser->m_resptimeoutclosetimer = -1;

				if (pWebUser->m_waitresptimeouttimer < 0)
					pWebUser->m_waitresptimeouttimer = 0;
			}
			else
			{
				msgFrom = MSG_FROM_ASSIST;
			}

			// ����Ӧ������Ϣ��ʾ
		}
		else
		{
			msgType = MSG_TYPE_PREV;
		}

		pWebUser->m_sNewSeq = packhead.sendrandom;

		if (RecvInfo.tMsgTime == 0)
		{
			// ��ȡϵͳ��ǰʱ��
		}

		GetTimeStringMDAndHMS(RecvInfo.tMsgTime, msgTime);

		m_baseMsgs->RecvOneMsg((IBaseObject*)pWebUser, msgFrom, GetMsgId(),	msgType, RecvInfo.nMsgDataType, 
			RecvInfo.strmsg, msgTime, pAssistUser, msgContentWx, "");


		// ͬ�����¹�����
		//if (m_sysConfig.bAutoSearchKeyword)
		{
			//CUserObject *pUser = m_pFormKeyWord->GetCurSelUserOb();
			//m_pFormKeyWord->ResetKeyWord((m_nOnLineStatus != STATUS_OFFLINE), pUser, RecvInfo.strmsg);
		}

		if (m_sysConfig->m_bAutoRespUnnormalStatus)
		{
			string strResp;

			switch (this->m_nOnLineStatus)
			{
			case STATUS_BUSY:
			case STATUS_WORK:
			case STATUS_REFUSE_NEWWEBMSG:
				strResp = m_sysConfig->m_strUnnormalStatusMsg[0];
				break;
			case STATUS_LEAVE:
			case STATUS_EATING:
			case STATUS_OFFLINE:
				strResp = m_sysConfig->m_strUnnormalStatusMsg[1];
				break;
			}

			if (!strResp.empty())
			{
				//SendAutoRespMsg(pWebUser, LPCTSTR(strResp));
			}
		}
	}
		break;
	case MSG_WORKBILL:
	case MSG_CLIENT_COME:
		g_WriteLog.WriteLog(C_LOG_TRACE, "--recvfloatchatmsg MSG_WORKBILL msgtype--");
		//RecvComSendWorkBillMsg(RecvInfo.webuin, -1, RecvInfo.strmsg, RecvInfo.nickname);
		break;
	default:
		g_WriteLog.WriteLog(C_LOG_ERROR, "recvfloatchatmsg unkown msgtype:%d", RecvInfo.nMsgType);
		break;
	}

	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvFloatChatMsgAck(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_CHATMSG_ACK RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "recvfloatchatmsgack unpack failed,Cmd:%.4x", packhead.cmd);

		goto RETURN;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "recvfloatchatmsgack uAdminId:%u,uWebuin:%u,chatid:%s,msgseq:%hu",
		RecvInfo.uAdminId, RecvInfo.webuin, RecvInfo.chatid, RecvInfo.chatseq);

	pWebUser = GetWebUserObjectByUid(RecvInfo.webuin);

	if (pWebUser != NULL)
		pWebUser->m_sNewSeq = RecvInfo.chatseq;

	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvFloatAcceptChat(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_ACCEPTCHAT RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;
	int nRet = 0;
	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "recvfloatacceptchat unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "recvfloatacceptchat sender:%u,uAdminId:%u,uWebuin:%u,chatid:%s,nickname:%s",
		packhead.uin, RecvInfo.uAdminId, RecvInfo.uWebuin, RecvInfo.chatid, RecvInfo.nickname);


	pUser = GetUserObjectByUid(packhead.uin);
	pWebUser = GetWebUserObjectByUid(RecvInfo.uWebuin);
	if (pWebUser == NULL)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "recvfloatacceptchat GetWebUserObjectByID(%u) failed", RecvInfo.uWebuin);
		goto RETURN;
	}
	if (m_userInfo.UserInfo.uid != packhead.uin)//�����߲��ǵ�ǰ��ϯ
	{
		pWebUser->m_nWaitTimer = -20;
		pWebUser->m_bConnected = HASINSERTRIGHT;
	}
	pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
	pWebUser->transferuid = 0;
	pWebUser->talkuid = packhead.uin;
	if (pUser == NULL || (m_userInfo.UserInfo.uid != pUser->UserInfo.uid&&!pUser->m_bFriend))
	{
		pWebUser->cTalkedSatus = HASTALKED;
		//m_pFormUser->m_TreeListUser.DeleteItemByLParam((LPARAM)pWebUser);
		g_WriteLog.WriteLog(C_LOG_ERROR, "delete user name is :%s, chatid: %s", pWebUser->info.name, pWebUser->chatid);
	}
	else
	{
		pWebUser->cTalkedSatus = INTALKING;
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
		if (packhead.uin == m_userInfo.UserInfo.uid)
		{
			//���Լ���������ķÿ�
			pWebUser->m_nWaitTimer = 0;
			pWebUser->m_bConnected = TRUE;
		}

		//����������ķÿ�
		//pWebUser->RemoveMutiUser(pWebUser->talkuid);
		pWebUser->inviteuid = 0;
		//�ÿ�����
		//m_pFormMain->RecvUserAcceptChat(pWebUser, pUser);

		//�ÿ��Ƶ����ܿͷ��ĶԻ��б�
		//nRet = m_pFormUser->MoveWebUserToUserTalkItem(pWebUser, pUser);
		//if (nRet < 0)
		//	m_pFormUser->m_TreeListUser.DeleteItemByLParam((LPARAM)pWebUser);
		//else if (packhead.uin == m_pUserInfo->UserInfo.uid)
		//	AddMultiWebUserToList(pWebUser);//�ѽ���ĻỰ�ƶ����Ự�б���
		////��ȡ�Ự��ʷ��Ϣ���������µ���Ϣ
		//StartRecvFloatMsg(packhead.random, RecvInfo.uAdminId, RecvInfo.chatid, pWebUser->m_sNewSeq);

	}
	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvFloatTransQuest(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_TRANSREQUEST RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pInviteUser = NULL;
	CUserObject *pAcceptUser = NULL;
	CWebUserObject *pWebUser = NULL;
	char sztime[256] = { 0 };
	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "recvfloattransquest unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatTransQuest sender:%u,uAdminId:%u,uWebuin:%u,chatid:%s",
		packhead.uin, RecvInfo.uAdminId, RecvInfo.uWebuin, RecvInfo.chatid);


	pAcceptUser = GetUserObjectByUid(RecvInfo.uToKefu);
	pInviteUser = GetUserObjectByUid(packhead.uin);
	pWebUser = GetWebUserObjectByUid(RecvInfo.uWebuin);
	if (pAcceptUser == NULL)//��ȡ�������Ͳ����Լ���Э������
	{
		goto RETURN;
	}
	//ת�ƻỰ��Ҫ���û�Ⱥ�б���ɾ��
	//DelWebUserInMultiList(pWebUser);

	if (pWebUser == NULL)
	{
		//SendTo_GetWebUserInfo(packhead.random, RecvInfo.uAdminId, RecvInfo.chatid);
	}
	else if (RecvInfo.uToKefu == m_userInfo.UserInfo.uid)//��ǰ��ϯ�ǽ����ߣ��ƶ���ת���С�����
	{
		if (strcmp(pWebUser->chatid, RecvInfo.chatid) != 0 && strlen(RecvInfo.chatid) > 0)
			strcpy(pWebUser->chatid, RecvInfo.chatid);
		pWebUser->floatadminuid = RecvInfo.uAdminId;
		if (!pWebUser->m_bNewComm)
		{
			pWebUser->m_bNewComm = true;
		}
		pWebUser->gpid = packhead.random;
		pWebUser->info.status = STATUS_ONLINE;
		pWebUser->onlineinfo.bInvited = false;//ת�ӹ������Ͳ�������Э����
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_TRANSFER;
		pWebUser->transferuid = RecvInfo.uToKefu;

		//if (pInviteUser != NULL)
		//	m_pFormMain->SendInviteToWebUser(pInviteUser, pWebUser, INVITE_TRANSFER, pAcceptUser);
		//else
		//	m_pFormMain->RecvTransferWebUser(pWebUser, pAcceptUser->UserInfo.nickname);

		//m_pFormUser->DeleteChildItem((DWORD)pInviteUser, TREELPARAM_USER_TALK, (DWORD)pWebUser);
		//m_pFormUser->MoveWebUserToUserTransferItem(pWebUser, pAcceptUser);

	}
	if (packhead.uin == m_userInfo.UserInfo.uid)//��ǰ��ϯ�Ƿ���ת����
	{
		GetTimeStringMDAndHMS(0, sztime);
		//CString strTip = "";
		//�ѷ���ת������%s
		//strTip.Format("�ѷ���ת������%s", pAcceptUser->UserInfo.nickname);
		//m_pFormMain->AddToMsgList(pWebUser, "", sztime, strTip);

	}
	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvFloatTransFailed(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	return 0;
}

int CChatManager::RecvInviteRequest(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	return 0;
}

int CChatManager::RecvInviteResult(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	return 0;
}

int CChatManager::RecvFloatKefuRelease(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	return 0;
}

int CChatManager::RecvFloatCMDError(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	return 0;
}

int CChatManager::RecvFloatCloseChat(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_CLOSECHAT RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "recvfloatclosechat unpack failed,Cmd:%.4x", packhead.cmd);
		return nError;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatCloseChat sender:%s(%u),uAdminId:%u,uWebuin:%u,chatid:%s,usType:%hu",
		RecvInfo.nickname, packhead.uin, RecvInfo.uAdminId, RecvInfo.webuin, RecvInfo.chatid, RecvInfo.usType);

	pUser = GetUserObjectByUid(packhead.uin);
	pWebUser = GetWebUserObjectByUid(RecvInfo.webuin);
	if (pWebUser == NULL || strcmp(pWebUser->chatid, RecvInfo.chatid) != 0)
	{
		if (pWebUser == NULL)
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "recvfloatclosechat GetWebUserObjectByID(%u) failed", RecvInfo.webuin);
		}
		else
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "recvfloatclosechat chatid(local:%s,pack:%s) not same", pWebUser->chatid, RecvInfo.chatid);
		}
		return nError;
	}

	if (CHATCLOSE_INVISTEXIT == RecvInfo.usType)
	{
		g_WriteLog.WriteLog(C_LOG_TRACE, "recvfloatclosechat ��������ϯ����(%u)", packhead.uin);
		//����������˳�����
		//pWebUser->RemoveMutiUser(packhead.uin);
		//�����ʾ
		//if (pUser != NULL)
		//{
		//	m_pFormMain->RecvInvestUserExit(pWebUser, packhead.uin, pUser->UserInfo.nickname);
		//}
		//else
		//	m_pFormMain->RecvInvestUserExit(pWebUser, packhead.uin, "");


		//pWebUser->RemoveMutiUser(packhead.uin);
		//if (!pWebUser->IsMutiUser())
		{
			//����ͼ��
			//m_pFormMain->m_pFormUser->ResetWebUserIcon(pWebUser);
		}
	}
	else
	{
		//�����Ự�����Э����ϯ
		//if (pWebUser->IsMutiUser())
		//	pWebUser->RemoveAllMutiUser();
		pWebUser->onlineinfo.bInvited = false;
		pWebUser->m_bNewComm = false;
		pWebUser->m_bConnected = true;
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_NO;
		pWebUser->cTalkedSatus = HASTALKED;
		string strMsg = "";
		if (CHATCLOSE_UNSUBSCRIBE == RecvInfo.usType)
		{
			strMsg = "΢���û�ȡ����ע�����Ự";
		}
		else
		{
			strMsg = "�ÿͻỰ�ѽ���";
		}
		//m_pFormMain->AddToMsgList(pWebUser, "", "", strMsg);
		if (!pWebUser->IsOnline())//�����ǲ��Ǹ��û�����������
		{
			g_WriteLog.WriteLog(C_LOG_TRACE, "recvfloatclosechat ��ϯ����(%u)�ÿ�����", packhead.uin);

			//SetVisitorOffline(pWebUser);
		}
		else
		{
			g_WriteLog.WriteLog(C_LOG_TRACE, "recvfloatclosechat ��ϯ����(%u)�ÿͷ�����", packhead.uin);
			//�ÿ��˻ص������б�
			pWebUser->m_nWaitTimer = -20;
			pWebUser->onlineinfo.talkstatus = TALKSTATUS_NO;
			//m_pFormUser->MoveWebUserToOnlineItem(pWebUser, pWebUser->IsOnline());
		}
		//�����Ự��Ҫ���û�Ⱥ�б���ɾ��
		//DelWebUserInMultiList(pWebUser);
	}

	nError = 0;

	return nError;
}

int CChatManager::RecvFloatListChat(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	return 0;
}

int CChatManager::RecvEventAnnouncement(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	return 0;
}

int CChatManager::RecvSrvUpdateSucc(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	return 0;
}

int CChatManager::RecvSrvUpdateFail(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	return 0;
}

int CChatManager::RecvSrvDonw(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	return 0;
}

int CChatManager::RecvRepTransferClient(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	return 0;
}

int CChatManager::RecvTransferClient(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	return 0;
}

int CChatManager::SendTo_GetShareList()
{
	int nError = SYS_ERROR_SEND_BEFORE_LOGIN;
	if (m_isLoginSuccess)
	{
		COM_FLOAT_SHARELIST SendInfo(VERSION);

		nError = SendPackTo(&SendInfo);
	}
	return nError;
}

int CChatManager::SendTo_GetListChatInfo()
{
	int nError = SYS_ERROR_SEND_BEFORE_LOGIN;
	if (m_isLoginSuccess)
	{
		COM_FLOAT_GETLIST SendInfo(VERSION);

		SendInfo.uAdminId = m_login->m_authAdminid;

		nError = SendPackTo(&SendInfo);
	}
	return nError;
}

CUserObject * CChatManager::AddUserObject(unsigned long id, char *sid, char *name, unsigned char status, unsigned long fd)
{
	CUserObject *pUser = GetUserObjectByUid(id);

	if (pUser == NULL)
	{
		pUser = new CUserObject();

		pUser->UserInfo.uid = id;

		m_mapUsers.insert(map<unsigned long, CUserObject*>::value_type(id, pUser));
		pUser->m_nFlag = 2;
	}
	else
	{
		pUser->m_nFlag = 1;
	}

	strcpy(pUser->UserInfo.sid, sid);
	strcpy(pUser->UserInfo.nickname, name);
	pUser->status = status;
	pUser->fd = fd;

	if (m_myInfoIsGet >= 0 && pUser->UserInfo.uid != 0 && pUser->UserInfo.uid == m_userInfo.UserInfo.uid)
	{
		m_myInfoIsGet = -20;
	}
	return pUser;
}

CUserObject * CChatManager::GetUserObjectByUid(unsigned long id)
{
	if (id == 0)
	{
		return &m_CommUserInfo;
	}

	if (m_userInfo.UserInfo.uid != 0 && m_userInfo.UserInfo.uid == id)
		return &m_userInfo;

	CUserObject *pUser = NULL;

	map <unsigned long, CUserObject*>::iterator iter_user;
	iter_user = m_mapUsers.find(id);
	if (iter_user != m_mapUsers.end())
	{
		pUser = iter_user->second;
		return pUser;
	}
	else
	{
		return NULL;
	}
}

void CChatManager::TimerProc(string timeName, LPVOID pThis)
{
	CChatManager* chat_manager = (CChatManager*)pThis;
	if (timeName == TIMER_NAME_NORMAL)
	{
		if (chat_manager->m_nOnLineStatus == STATUS_OFFLINE)
			return;
		if (chat_manager->m_nMyInfoIsGet >= 0)
		{
			chat_manager->m_nMyInfoIsGet++;

			if (chat_manager->m_nMyInfoIsGet >= 3)
			{
				//����Լ�����Ϣû�л�ȡ������ô��Ҫ�ظ���ȥ��ȡ
				chat_manager->m_nMyInfoIsGet = 0;
				chat_manager->SendGetClkInfo(chat_manager->m_userInfo.UserInfo.uid, chat_manager->m_userInfo.UserInfo.sid, CMD_SRV_REP_USERINFO);
			}
		}
	}
	else if (timeName == TIMER_NAME_LOGIN)
	{
		if (!chat_manager->m_isLoginSuccess)
		{
			chat_manager->m_lastError = "��¼��ʱ";
		}
		chat_manager->m_timers->KillTimer(TIMER_NAME_LOGIN);
	}
}

int CChatManager::SendTo_GetAllUserInfo()
{
	int nError = SYS_ERROR_SEND_BEFORE_LOGIN;
	if (m_isLoginSuccess)
	{
		CUserObject *pUser;
		map <unsigned long, CUserObject*>::iterator iter_user;
		for (iter_user = m_mapUsers.begin(); iter_user != m_mapUsers.end(); iter_user++)
		{
			pUser = iter_user->second;
			if (pUser != NULL && strlen(pUser->UserInfo.nickname) == NULL)
			{
				nError = SendTo_GetUserInfo(pUser->UserInfo.uid);
			}
		}
	}
	return nError;
}

int CChatManager::SendTo_GetUserInfo(unsigned long uid)
{
	int nError = SYS_ERROR_SEND_BEFORE_LOGIN;
	if (m_isLoginSuccess)
	{
		if (uid == 0)
		{
			nError = 0;
		}
		else
		{
			CLT_GET_CLT_INFO SendInfo(VERSION);

			SendInfo.seq = GetPackSeq();
			SendInfo.uin = uid;
			SendInfo.type = UPDATE_ALLINFO;

			nError = SendPackTo(&SendInfo);
		}
	}

	return nError;
}

unsigned short CChatManager::GetPackSeq()
{
	++g_packSeq;
	if (g_packSeq == 0)
	{
		++g_packSeq;
	}
	return g_packSeq;
}

int CChatManager::SendGetClkInfo(unsigned long id, char *strid, unsigned short cmd, unsigned short cmdtype, unsigned short type)
{
	int nError = 0;
	CLT_GET_INFO SendInfo(VERSION, cmd);

	SendInfo.cmdtype = cmdtype;
	SendInfo.type = type;

	SendInfo.id = id;
	strcpy(SendInfo.strid, strid);

	nError = SendPackTo(&SendInfo);

	return nError;
}

void CChatManager::TimerSolveAck()
{
	if (m_nSendPing < 0)
		return;

	m_nSendPing++;

	//30�뷢��һ�δ̼���
	if (m_nSendPing > 60)
	{
		if (m_nOnLineStatus != STATUS_OFFLINE)
		{
			// ����ping�������������˽�����Լ����ping��
			if (SendPing() != 0)
			{
				m_nSendPingFail++;
			}
			else
			{
				m_nSendPingFail = 0;
			}

			if (m_nSendPingFail > 3)
			{
				// �ۼ�3�����Ӳ��ϣ������ߴ���
				g_WriteLog.WriteLog(C_LOG_TRACE, "SendPingFailed3times--SetOfflineStatus ");
				SetOfflineStatus();

				CloseAllSocket();
			}
		}

		if (m_nOnLineStatusEx != STATUS_OFFLINE)
		{
			// ��������㼶��ping����ȷ�Ϸ������Ƿ�������
			if (SendPingToVisitorServer() != 0)
			{
				// ���Ӳ��ϴ���
				m_nOnLineStatusEx = STATUS_OFFLINE;
				m_nLoginToVisitor = 0;
			}
		}

		// ʱ������
		m_nSendPing = 0;
	}
}

int CChatManager::SendPing()
{
	int nError = 0;

	COM_SEND_PING SendInfo(VERSION);
	SendInfo.o.online_flag = m_userInfo.UserInfo.onlineflag;

	nError = SendPackTo(&SendInfo, 0, 0);

	return nError;
}

void CChatManager::SetOfflineStatus()
{
	
}

void CChatManager::CloseAllSocket()
{

}

int CChatManager::SendPingToVisitorServer()
{
	char sbuff[512];
	int nError(0);

	if (time(NULL) - m_tResentVisitPackTime > 60)
	{
		time(&m_tResentVisitPackTime);

		sprintf(sbuff, "<PING><PING>\r\n");
		nError = SendBuffToVisitorServer(sbuff, strlen(sbuff));
	}
	return nError;
}

int CChatManager::SendBuffToVisitorServer(char *sbuff, int len)
{
	int nrtn = 0;
	int nError;
	g_VisitLog.WriteLog(C_LOG_TRACE, "send:%s", sbuff);

	if (!m_socketEx.SendBuff(sbuff, len, nError))
	{
		nrtn = SYS_ERROR_SENDFAIL;

		g_VisitLog.WriteLog(C_LOG_TRACE, "send visit pack failed:%s!", sbuff);
	}

	return nrtn;
}

int CChatManager::SendTo_UpdateOnlineStatus(unsigned short status)
{
	int nError = SYS_ERROR_SEND_BEFORE_LOGIN;
	if (m_isLoginSuccess)
	{
		unsigned int dwstatus = 0;

		CLT_UPDATE_STATUS SendInfo(VERSION);

		SendInfo.seq = GetPackSeq();
		SendInfo.bSendToServer = true;
		SendInfo.type = UPDATE_STATUS;
		SendInfo.onlinestatus = SetMutiByte(dwstatus, ONLINE_INFO_STATUS, ONLINE_INFO_STATUS_LEN, (DWORD)status);

		nError = SendPackTo(&SendInfo);
	}
	return nError;
}

CWebUserObject * CChatManager::GetWebUserObjectBySid(char *sid)
{
	return NULL;
}

CWebUserObject * CChatManager::AddWebUserObject(char *sid, char *thirdid, char *name, char *scriptflag, char *url, unsigned char status, unsigned char floatauth)
{
	return NULL;
}

CWebUserObject * CChatManager::GetWebUserObjectByUid(unsigned long uid)
{
	return NULL;
}

int CChatManager::SendTo_GetWebUserInfo(unsigned long webuserid, const char *chatid, char *szMsg, unsigned int chatkefuid)
{
	int nError = 0;
	COM_SEND_MSG SendInfo(VERSION);
	Json::Value jv;
	if (szMsg != NULL && chatkefuid > 0)
	{
		jv["ReturnParameters"] = szMsg;
		jv["ChatKefu"] = chatkefuid;
	}
	else if (chatkefuid > 0)
	{
		jv["ChatKefu"] = chatkefuid;
	}
	else if (szMsg != NULL){
		jv["ReturnParameters"] = szMsg;
	}

	SendInfo.msg.msgtype = MSG_GET_WORKBILL;
	SendInfo.msg.recvuin = webuserid;
	SendInfo.msg.sendtime = 0;
	strncpy(SendInfo.strChatid, chatid, MAX_CHATID_LEN);
	SendInfo.msg.bak = 0;
	SendInfo.msg.seq = GetPackSeq();
	SendInfo.version = VERSION;
	SendInfo.msg.senduin = m_userInfo.UserInfo.uid;
	strncpy(SendInfo.msg.strmsg, jv.toStyledString().c_str(), MAX_MSG_RECVLEN);
	g_WriteLog.WriteLog(C_LOG_TRACE, "GET_WORKBILL: recvuin:%u,senduin:%u��szMsg=%s", SendInfo.msg.recvuin, SendInfo.msg.senduin, szMsg);

	nError = SendPackTo(&SendInfo);

	return nError;
}

void CChatManager::TransforFaceMsg(string& str)
{
	char buf[8];
	std::string::size_type pos, n;
	const std::string *fs;
	const wx_face_t *face = wx_faces;
	for (int i = 0; i < _countof(wx_faces); ++i, ++face)
	{
		for (pos = 0;;)
		{
			fs = &face->raw;
			if (std::string::npos == (n = str.find(*fs, pos)))
			{
				fs = &face->enc;
				if (face->enc.empty() || std::string::npos == (n = str.find(*fs, pos)))
					break;
			}
			pos = n;
			_itoa(i, buf, 10);
			str.insert(pos, FACE_PREFIX);
			pos += sizeof(FACE_PREFIX)-1;
			str.insert(pos, m_initConfig.webpage_FaceImage);
			pos += strlen(m_initConfig.webpage_FaceImage);
			str.insert(pos, buf);
			pos += strlen(buf);
			str.replace(pos, fs->length(), FACE_ALT FACE_SUFFIX);
			pos += sizeof(FACE_ALT FACE_SUFFIX) - 1;
		}
	}

	// deal with android/ios emoji face
	int len = str.length() - 3;
	for (int i = 0; i < len; ++i)
	{
		if (str[i] == -16 && str[i + 1] == -97)
		{
			str[i] = -18;
			str[i + 1] = -112;
			str[i + 2] = -116;
			str.erase(i + 3, 1);
		}
	}
}

WxMsgBase* CChatManager::ParseWxMsg(CWebUserObject* pWebUser, COM_FLOAT_CHATMSG& recvInfo)
{
	//΢����Ϣ���ͣ�utf8�������� 
	WxMsgBase* msgBase = NULL;
	WxObj *pwxobj = ParseWxJsonMsg(recvInfo.strmsg);

	if (pwxobj != NULL)
	{
		if ("userinfo" == pwxobj->MsgType)
		{
			// �˴�ע�⣬����ڶ����յ�userinfo��Ӧ����ǰ���յ��Ǵ���Ϣ������
			if (pWebUser->m_pWxUserInfo == NULL)
			{
				pWebUser->m_pWxUserInfo = (WxUserInfo*)pwxobj;
			}
			else
			{
				delete pWebUser->m_pWxUserInfo;
				pWebUser->m_pWxUserInfo = (WxUserInfo*)pwxobj;
			}
			pWebUser->m_bIsGetInfo = true;
		}
		else if ("wxactoken" == pwxobj->MsgType)
		{
			//todo:�յ���������������΢�ŵ�access_token��Ϣ
			WxTokens::iterator iter = m_mapTokens.find(recvInfo.strThirdid);
			if (iter == m_mapTokens.end())
			{
				m_mapTokens.insert(map<string, string>::value_type(recvInfo.strThirdid, ((WxAccessTokenInfo*)pwxobj)->szAcToken));
			}
			else
			{
				iter->second = ((WxAccessTokenInfo*)pwxobj)->szAcToken;
			}
			delete pwxobj;
		}
		else if ("image" == pwxobj->MsgType)
		{
			//GetWxUserInfoAndToken(pWebUser);
			msgBase = (WxMsgBase*)pwxobj;
			recvInfo.nMsgDataType = MSG_DATA_TYPE_IMAGE;
		}
		else if ("voice" == pwxobj->MsgType)
		{
			msgBase = (WxMsgBase*)pwxobj;
			recvInfo.nMsgDataType = MSG_DATA_TYPE_VOICE;
		}
		else if ("video" == pwxobj->MsgType || "shortvideo" == pwxobj->MsgType)
		{
			msgBase = (WxMsgBase*)pwxobj;
			recvInfo.nMsgDataType = MSG_DATA_TYPE_VIDEO;
		}
		else if ("location" == pwxobj->MsgType)
		{
			msgBase = (WxMsgBase*)pwxobj;
			recvInfo.nMsgDataType = MSG_DATA_TYPE_LOCATION;
		}
		else if ("link" == pwxobj->MsgType)
		{
			msgBase = (WxMsgBase*)pwxobj;
			recvInfo.nMsgDataType = MSG_DATA_TYPE_LINK;
		}
		else
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "recvfloatchatmsg() unknow weixin Json:%s", recvInfo.strmsg);
			delete pwxobj;
		}
	}
	else
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "recvfloatchatmsg() NOT the weixin Json:%s", recvInfo.strmsg);
	}
	return msgBase;
}

string CChatManager::GetMsgId()
{
	char msgid[256];
	sprintf(msgid, "id_%d", m_msgId);
	m_idLock.Lock();
	m_msgId++;
	m_idLock.Unlock();
	return msgid;
}

void CChatManager::SaveEarlyMsg(MSG_INFO *pMsgInfo)
{
	MSG_INFO *pInfo = new MSG_INFO;

	memcpy(pInfo, pMsgInfo, sizeof(MSG_INFO));

	m_listEarlyMsg.push_back(pInfo);
}

int CChatManager::SendAckEx(unsigned short seq, unsigned long uid /*= 0*/, unsigned long ip /*= 0*/, unsigned short port /*= 0*/)
{
	int nError = 0;
	COM_ACKEX SendInfo(VERSION);

	SendInfo.seq = seq;
	SendInfo.uin = uid;
	SendInfo.ip = ip;
	SendInfo.port = port;

	nError = SendPackTo(&SendInfo);

	return nError;
}

int CChatManager::SendAutoRespMsg(CWebUserObject *pWebUser, const char *msg, BOOL bClearTimer /*= true*/)
{
	int rtn = SYS_ERROR_SENDFAIL;
	if (pWebUser == NULL || msg == NULL || strlen(msg) <= 0)
		return rtn;

	if (!pWebUser->m_bConnected)
	{
		return rtn;
	}

	//rtn = SendMsgToWebUser(pWebUser, msg);
	if (rtn == 0)
	{
		//m_pFormMain->SendComMsg(pWebUser, LPCTSTR(msg));

		if (bClearTimer && pWebUser != NULL)
		{
			pWebUser->m_resptimeoutmsgtimer = -1;
			pWebUser->m_resptimeoutclosetimer = -1;
			pWebUser->m_waitresptimeouttimer = -1; // 0 to -1 �Լ�������Ϣ�� ��ֻ�ȴ��ͻ�Ӧ�� [12/29/2010 SC]
		}
	}
	else
	{
		//m_pFormMain->AddSysBuffToListMsg(pWebUser, "�Զ��ظ���Ϣʧ��!");
	}

	return rtn;
}

int CChatManager::SendWebuserTalkBegin(CWebUserObject *pWebUser)
{
	if (pWebUser == NULL)
		return SYS_ERROR_SENDFAIL;

	char sbuff[512];
	int nError;

	sprintf(sbuff, "<SCRIPTMSG><COMMAND>TALKBEG</COMMAND><ADMINID>%lu</ADMINID><CLIENTID>%s</CLIENTID><SERVICEUIN>%lu</SERVICEUIN><NICKNAME>%s</NICKNAME></SCRIPTMSG>\r\n",
		pWebUser->floatadminuid, pWebUser->info.sid, m_userInfo.UserInfo.uid, m_userInfo.UserInfo.nickname);

	nError = SendBuffToVisitorServer(sbuff, strlen(sbuff));
	return nError;
}

int CChatManager::SendWebuserTalkEnd(CWebUserObject *pWebUser)
{
	if (pWebUser == NULL)
		return SYS_ERROR_SENDFAIL;

	char sbuff[512];
	int nError;

	sprintf(sbuff, "<SCRIPTMSG><COMMAND>TALKEND</COMMAND><ADMINID>%lu</ADMINID><CLIENTID>%s</CLIENTID><SERVICEUIN>%lu</SERVICEUIN><NICKNAME>%s</NICKNAME></SCRIPTMSG>\r\n",
		pWebUser->floatadminuid, pWebUser->info.sid, m_userInfo.UserInfo.uid, m_userInfo.UserInfo.nickname);

	nError = SendBuffToVisitorServer(sbuff, strlen(sbuff));
	return nError;
}

void CChatManager::RecvComSendWorkBillMsg(unsigned long senduid, unsigned long recvuid, char *msg, char* mobile)
{
	char sid[MAX_WEBCLIENID_LEN + 1] = { 0 }, billid[MAX_CHATID_LEN + 1] = { 0 }, szReturnParameters[51] = { 0 };
	CWebUserObject *pWebUser;
	string strMsg = "", strReturnParameters = "";
	strMsg = msg;
	int nPos = strMsg.find("ReturnParameters:");
	strReturnParameters = strMsg.substr(0, strMsg.size() - nPos - 17);
	if (!GetContentBetweenString(msg, "WorkBillID:", "\n", billid))
	{
		billid[0] = 0;
	}
	if (GetContentBetweenString(msg, "rand=", "&", sid))
	{
		char thirdid[MAX_THIRDID_LEN + 1] = { 0 };
		GetContentBetweenString(msg, "rand=", "&", sid);
		GetContentBetweenString(msg, "clientid=", "&", thirdid);

		char chatfrom[MAX_CHATID_LEN + 1] = { 0 };
		GetContentBetweenString(msg, "chatfrom=", "&", chatfrom);

		char wxappid[100] = { 0 };
		GetContentBetweenString(msg, "wxappid=", "&", wxappid);
		GetContentBetweenString(msg, "ReturnParameters:", "\0", szReturnParameters);
		pWebUser = GetWebUserObjectByUid(senduid);
		if (pWebUser != NULL)
		{
			//��webuiʱ���get��û�в���rand��webui��rand��Ϊclientuin
			if (strlen(sid) > 0 || ((unsigned long)atol(sid)) != senduid)
			{
				//ChangeWebUserSid(pWebUser, sid, thirdid);
			}
			//���˷ÿͣ��϶������ߵ�
			if (pWebUser->info.status == STATUS_OFFLINE)
				pWebUser->info.status = STATUS_ONLINE;
			if (senduid > WEBUSER_UIN && pWebUser->webuserid != senduid)
			{
				pWebUser->webuserid = senduid;
			}
		}
		else
		{

			pWebUser = GetWebUserObjectBySid(sid);

			if (pWebUser == NULL)
			{
				pWebUser = AddWebUserObject(sid, "", mobile, "", "", STATUS_ONLINE, 0);

				SendTo_GetUserInfo(senduid);
			}

			pWebUser->webuserid = senduid;
			SolveWebUserEarlyMsg(pWebUser);
		}

		if (pWebUser != NULL&&billid[0] != 0)
		{
			strncpy(pWebUser->info.chatfrom, chatfrom, MAX_CHATID_LEN);
			strncpy(pWebUser->chatid, billid, MAX_CHATID_LEN);
			pWebUser->m_sWxAppid = wxappid;

			// ΢���û���������ȡ΢�ŵ�userinfo
			if (!pWebUser->m_sWxAppid.empty())
			{
				pWebUser->m_bIsFrWX = true;
				//GetWxUserInfoAndToken(pWebUser);
			}

			time_t tnow = time(NULL);

			if (!pWebUser->GetNormalChatHisMsgSuccess
				&&tnow - pWebUser->tGetNormalChatHismsgTime > 3) //�жϳ�ʱ���������ж�ʱ���ж��£���ֹ�յ�����������Ӧ��
			{
				//GetNormalChatHisMsg(senduid, billid);// ����Ƿǵȴ�Ӧ��Ĳ���Ҫȥ��ȡ��Ϣ��¼
				pWebUser->tGetNormalChatHismsgTime = tnow;
			}
		}

		if (strReturnParameters.find("transfer") >= 0)//ת����
		{
			//����ת��
			//strReturnParameters.Remove('\\');
			//strReturnParameters.Remove('\n');
			Json::Value jv;
			//if (!ParseJson(strReturnParameters.GetBuffer(0), jv))
			{
				return;
			}
			//�жϷ��سɹ�ʧ�ܽ��
			unsigned long uTransuid = GetIntFromJson(jv, "transfer");
			if (pWebUser->transferuid != m_userInfo.UserInfo.uid)
			{
				pWebUser->onlineinfo.talkstatus = TALKSTATUS_TRANSFER;
				pWebUser->info.status = STATUS_ONLINE;
				pWebUser->transferuid = m_userInfo.UserInfo.uid;
				pWebUser->talkuid = uTransuid;

				//m_pFormMain->RecvTransferWebUser(pWebUser, m_userInfo.UserInfo.nickname);
				//m_pFormUser->MoveWebUserToUserTransferItem(pWebUser, m_pUserInfo);
				//m_tranferList[pWebUser->webuserid] = 0;
				//KillTimer(TIMER_TRANS_TIMEOUT);
				//SetTimer(TIMER_TRANS_TIMEOUT, 1000, NULL);
			}
		}
		else if (strReturnParameters.find("waiter") >= 0) {
		}
		else if ((pWebUser->cTalkedSatus != INTALKING || !pWebUser->m_bConnected)
			&& m_userInfo.UserInfo.uid == recvuid && !pWebUser->m_bNewComm)//�ǵȴ�Ӧ��ĻỰ
		{
			SendWebuserTalkBegin(pWebUser);

			pWebUser->cTalkedSatus = INTALKING;

			pWebUser->talkuid = m_userInfo.UserInfo.uid;

			pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
			pWebUser->transferuid = 0;
			pWebUser->m_nWaitTimer = 0;
			pWebUser->m_bConnected = TRUE;

			//int nRet = this->m_pFormUser->MoveWebUserToUserTalkItem(pWebUser, m_pUserInfo);
			//if (nRet < 0)
			//	m_pFormUser->m_TreeListUser.DeleteItemByLParam((LPARAM)pWebUser);
			//else
			//	AddMultiWebUserToList(pWebUser);//�ѽ���ĻỰ�ƶ����Ự�б���
			
			// ��ʾ
		}
	}
}

void CChatManager::SolveWebUserEarlyMsg(CWebUserObject *pWebUser)
{
	list<MSG_INFO*>::iterator iter = m_listEarlyMsg.begin();

	for (iter; iter != m_listEarlyMsg.end(); iter++)
	{
		if (pWebUser->webuserid == (*iter)->senduin)
		{
			pWebUser->m_nWaitTimer = 0;
			//this->m_pFormMain->RecvComMsg(pWebUser, msginfo, true);
			if (strlen(pWebUser->info.name) <= 0)
			{
				//strcpy(pWebUser->info.name, msginfo->strmobile);
				//if (strlen(pWebUser->info.ipfromname) <= 0)
				//	strcpy(pWebUser->info.ipfromname, pWebUser->info.name);
				if (strlen(pWebUser->info.name) < 2)
				{
					g_WriteLog.WriteLog(C_LOG_ERROR, "name length is null��SolveWebUserEarlyMsg() name length��%d", strlen(pWebUser->info.name));
				}
			}
		}
	}

	
	for (iter = m_listEarlyMsg.begin(); iter != m_listEarlyMsg.end(); iter++)
	{
		if (pWebUser->webuserid == (*iter)->senduin)
		{
			delete (*iter);
			m_listEarlyMsg.erase(iter);
			return;
		}
	}
}

void CChatManager::RecvComSendNormalChatidHisMsg(unsigned long senduid, unsigned long recvuid, COM_SEND_MSG& RecvInfo)
{
	if (!g_NomalChatHistoryMsgManage.ParseJsonHisMsg(RecvInfo.strChatid, RecvInfo.msg.strmsg))
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendNormalChatidHisMsg failed,chatid:%s,msg:%s", RecvInfo.strChatid, RecvInfo.msg.strmsg);
		return;
	}
	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvComSendNormalChatidHisMsg msg:%s", RecvInfo.msg.strmsg);
	
	
	CNChatHisMsg nchatmsg;
	if (CNomalChatHistoryMsgManage::NHISMSGTYPE::SUCCESS != g_NomalChatHistoryMsgManage.GetCompleteMsgs(RecvInfo.strChatid, nchatmsg))
	{
		return;
	}
	
	//��ʾ��ʷ��Ϣ���������ѻ�ȡ��ʷ��Ϣ״̬
	
	CWebUserObject *pWebUser = GetWebUserObjectBySid(RecvInfo.strChatid);
	if (pWebUser != NULL)
	{
		pWebUser->GetNormalChatHisMsgSuccess = true; // ���λỰ��Ҫ���ñ���
	}
}

int CChatManager::RecvComTransRequest(unsigned long senduid, COM_SEND_MSG& RecvInfo)
{
	int nError = 0;
	CUserObject *pAcceptUser = NULL;
	CWebUserObject *pWebUser = NULL;

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvComTransRequest ��%s", RecvInfo.msg.strmsg);

	pAcceptUser = GetUserObjectByUid(RecvInfo.msg.recvuin);
	pWebUser = GetWebUserObjectByUid(RecvInfo.msg.senduin);
	if (pAcceptUser == NULL)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComTransRequest GetUserObject failed");
		goto RETURN;
	}

	if (pWebUser == NULL)
	{
		pWebUser = GetWebUserObjectBySid(RecvInfo.strRand);
	}

	if (pWebUser == NULL)
	{
		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvComTransRequest pWebUser==NULL");
		char strMsg[MAX_256_LEN];
		sprintf(strMsg, "{\"transfer\":%u}", senduid);
		SendTo_GetWebUserInfo(RecvInfo.msg.senduin, RecvInfo.strChatid, strMsg, senduid);
	}
	else if (RecvInfo.msg.recvuin == m_userInfo.UserInfo.uid && pWebUser->transferuid != m_userInfo.UserInfo.uid)//��ǰ��ϯ�ǽ����ߣ��ƶ���ת���С�����
	{
		if (strcmp(pWebUser->chatid, RecvInfo.strChatid) != 0 && strlen(RecvInfo.strChatid) > 0)
			strcpy(pWebUser->chatid, RecvInfo.strChatid);
		pWebUser->talkuid = senduid;
		pWebUser->m_bNewComm = false;
		pWebUser->info.status = STATUS_ONLINE;
		if (RecvInfo.msg.senduin > WEBUSER_UIN && pWebUser->webuserid != RecvInfo.msg.senduin)
		{
			pWebUser->webuserid = RecvInfo.msg.senduin;
		}

		pWebUser->onlineinfo.talkstatus = TALKSTATUS_TRANSFER;
		pWebUser->transferuid = m_userInfo.UserInfo.uid;

		//m_pFormMain->RecvTransferWebUser(pWebUser, pAcceptUser->UserInfo.nickname);

		//m_pFormUser->MoveWebUserToUserTransferItem(pWebUser, pAcceptUser);
		
		//��ȡ��ʷ��Ϣ

		//���ó�ʱ��ʱ��
		//m_TranfserList[pWebUser->webuserid] = 0;
		//KillTimer(TIMER_TRANS_TIMEOUT);
		//SetTimer(TIMER_TRANS_TIMEOUT, 1000, NULL);
	}

	nError = 0;
RETURN:
	return nError;
}

int CChatManager::RecvComTransAnswer(unsigned long senduid, COM_SEND_MSG& RecvInfo)
{
	int nError = 0;
	CUserObject *pAcceptUser = NULL;//��ת����ϯ
	CWebUserObject *pWebUser = NULL;
	string strMsg = "";
	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvComTransAnswer ��%s,recvuin:%u", RecvInfo.msg.strmsg, RecvInfo.msg.recvuin);

	pAcceptUser = GetUserObjectByUid(senduid);
	pWebUser = GetWebUserObjectByUid(RecvInfo.msg.senduin);
	if (senduid == 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComTransAnswer GetUserObject��ȡ��ת����ʧ�ܣ�%u", senduid);
		nError = -1;
		goto RETURN;
	}
	if (pWebUser == NULL)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComTransAnswer GetWebUserObject��ȡ�û�ʧ�ܣ�chatid%s", RecvInfo.strChatid);
		nError = -2;
		goto RETURN;
	}

	strMsg = RecvInfo.msg.strmsg;
	if (strMsg.find("OK") >= 0)//ͬ��ת��
	{
		//��ϯͬ�����CLT_TRANSFERCLIENT��
		strMsg = pWebUser->info.name;
		strMsg = "�Է�ͬ�����Էÿ�[%s]��ת��" + strMsg;
		//SendTransferClinet(pAcceptUser, pWebUser, senduid);
	}
	else if (strMsg.find("NO") >= 0)//�ܾ�ת��
	{
		strMsg = pWebUser->info.name;
		strMsg = "�Է��ܾ����Էÿ�[%s]��ת��" + strMsg;
	}
	else if (strMsg.find("TIMEOUT") >= 0)//��ʱ
	{
		strMsg = pWebUser->info.name;
		strMsg = "���Էÿ�[%s]��ת�ӳ�ʱ" + strMsg;
	}

RETURN:
	return nError;
}

int CChatManager::SendTo_GetWebUserChatInfo(unsigned short gpid, unsigned long adminid, char *chatid)
{
	int nError = 0;
	COM_FLOAT_CHATINFO SendInfo(VERSION, gpid);

	SendInfo.uAdminId = adminid;
	strcpy(SendInfo.chatid, chatid);

	nError = SendPackTo(&SendInfo);
	g_WriteLog.WriteLog(C_LOG_TRACE, "SendTo_GetWebUserChatInfo chatid:%s,gpid:%u", chatid, gpid);
	return nError;
}

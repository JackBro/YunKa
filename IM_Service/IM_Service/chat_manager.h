#pragma once

#include "comm_struct.h"
#include "sys_config.h"
#include "chat_objects.h"
#include "msgs.h"
#include "login.h"
#include "./common/comobject.h"
#include "./common/comstruct.h"

class IBaseMsgs
{
public:
	// ��¼�Ľ���,percent=100ʱ��ʾ��¼�ɹ�
	virtual void LoginProgress(int percent) = 0;

	// ��ȡ��һ�δ�����Ϣ
	virtual tstring GetLastError() = 0;

	// �յ�һ����Ϣ
	virtual void OnRecvOneMsg() = 0;

	// ��ϯ������Ϣ
	virtual void LogingMsg() = 0;

	// ��ϯ������Ϣ
};

class CChatManager : public IBaseReceive
{
private:
	CChatManager(IBaseMsgs* iBaseMsgs);
	~CChatManager();

	friend class CLogin;
	friend class CMySocket;

	virtual void OnReceive(void* pHead, void* pData);

public:

	static CChatManager* GetInstance(IBaseMsgs* iBaseMsgs)
	{
		static CChatManager instance(iBaseMsgs);
		return &instance;
	}

public:

	// ��ȡ��һ�ε�¼��Ϣ
	void GetPreLoginInfo(tstring& loginName, tstring& password, bool& isAutoLogin, bool& isKeepPwd);

	// ��ʼ��¼
	void StartLogin(tstring loginName, tstring password, bool isAutoLogin, bool isKeepPwd);

	// ��ȡϵͳ���ö���
	CSysConfig* GetSysConfig();

	// ��ȡ������������ʵ��
	CChatObjects* GetChatObjects();

	// ��ȡ������Ϣ������ʵ��
	CMsgs* GetMsgs();

	// ��ͼ
	void ScreenCapture();

	
private:
	/***************     �����ļ���������     *****************/

	bool ReadSystemConfig();

	void SetSystemConfigByInitconfig();

	void SolveUserdefineFilter(FILTER_USERDEFINE &filter, char *sfilter);


	/***************     ��Ϣ��������      *****************/

	int RecvSrvConfLogon(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvRepUserinfo(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvDenyLogon(PACK_HEADER packhead, char *pRecvBuff, int len, int &errtype);

	int RecvSrvConfLogOff(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvStatusFrdOnline(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvStatusFrdOffline(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvStatusUserForm(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatShareList(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvComSendMsg(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatCreateChat(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatChatMsg(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatChatMsgAck(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatAcceptChat(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatTransQuest(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatTransFailed(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvInviteRequest(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvInviteResult(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatKefuRelease(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatCMDError(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatCloseChat(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatListChat(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatChatInfo(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvEventAnnouncement(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvUpdateSucc(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvUpdateFail(PACK_HEADER packhead, char *pRecvBuff, int len);

	//�û�����ص�½
	int RecvSrvDonw(PACK_HEADER packhead, char *pRecvBuff, int len);

	//ת����ʱ�û��ɹ�ʧ��
	int RecvRepTransferClient(PACK_HEADER packhead, char *pRecvBuff, int len);

	//����ת�ƻỰ���û�����
	int RecvTransferClient(PACK_HEADER packhead, char *pRecvBuff, int len);

	/***************     ��Ϣ��������      *****************/


	int UnPack(CPackInfo *pPackInfo, char *buff, int len);

	bool LoadINIResource();

	int SendPackTo(CPackInfo *pPackInfo, unsigned long recvuid = 0, unsigned long recvsock = 0, HWND hWnd = NULL);

	bool LoadUserConfig();  //������ϯ�����ڼ���һ�������ļ�

private:
	CLogin* m_login;				// ��¼������
	CChatObjects* m_chatObjects;	// ��Ϣ������
	bool m_isLoginSuccess;			// �Ƿ��ѵ�¼
	IBaseMsgs* m_baseMsgs;			// �������Ϣ���սӿ�
	CSysConfigFile* m_sysConfig;	// ϵͳ�����ļ���
	INIT_CONF m_InitConfig;

	CMySocket	m_Socket;	// ��Ϣ����
	CMySocket	m_SocketEx; // �ÿͽӴ�����

	tstring m_loginName;			// ��¼��
	tstring m_password;				// ��¼����
	bool m_isAutoLogin;				// �Ƿ��Զ���¼
	bool m_isKeepPwd;				// �Ƿ��ס����
	tstring	m_server;
	int		m_port;

	CUserObject m_UserInfo;
	unsigned short	m_usSrvRand;	//�������������
	unsigned short	m_usCltRand;	//�������е������

	tstring m_lastError;
};


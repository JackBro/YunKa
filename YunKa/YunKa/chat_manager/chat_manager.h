#pragma once

#include "sys_config.h"
#include "chat_objects.h"
#include "msgs.h"
#include "login.h"
#include "../chat_common/comobject.h"
#include "../chat_common/comstruct.h"
#include "timer.h"

class IBaseMsgs
{
public:
	// ��¼�Ľ���,percent=100ʱ��ʾ��¼�ɹ�
	virtual void LoginProgress(int percent) = 0;

	// ��ȡ��һ�δ�����Ϣ
	virtual string GetLastError() = 0;

	// �յ�һ����Ϣ
	virtual void OnRecvOneMsg() = 0;

	// ��ϯ������Ϣ
	virtual void Online() = 0;

	// ��ϯ������Ϣ
	virtual void Offline() = 0;

	// �Ự�ر�
	virtual void CloseChat() = 0;
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
	void GetPreLoginInfo(string& loginName, string& password, bool& isAutoLogin, bool& isKeepPwd);

	// ��ʼ��¼
	void StartLogin(string loginName, string password, bool isAutoLogin, bool isKeepPwd);

	// ���ͻ�ȡ���Ѷ����б��¼�
	void SendTo_GetShareList();

	// ���ͻ�ȡ�Ự�б��¼�
	void SendTo_GetListChatInfo();

	// ���ͻ�ȡĳ���û�����Ϣ
	int SendTo_GetUserInfo(unsigned long uid);

	// ��ȡϵͳ���ö���
	CSysConfig* GetSysConfig();

	// ��ȡ������������ʵ��
	CChatObjects* GetChatObjects();

	// ��ȡ������Ϣ������ʵ��
	CMsgs* GetMsgs();

	// ��ͼ
	void ScreenCapture();

	static void CALLBACK TimerProc(void* p);

private:
	/***************     �����ļ���������     *****************/

	bool ReadSystemConfig();

	void SetSystemConfigByInitconfig();

	void SolveUserdefineFilter(FILTER_USERDEFINE &filter, char *sfilter);

	bool LoadINIResource();

	bool LoadUserConfig();  //������ϯ�����ڼ���һ�������ļ�


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

	int SendPackTo(CPackInfo *pPackInfo, unsigned long recvuid = 0, unsigned long recvsock = 0, HWND hWnd = NULL);


	CUserObject *AddUserObject(unsigned long id, char *sid, char *name, unsigned char status, unsigned long fd);

	CUserObject *GetUserObject(unsigned long id);

	int GetAllUser();

	unsigned short GetPackSeq();

private:
	CLogin*					m_login;				// ��¼������
	CChatObjects*			m_chatObjects;			// ��Ϣ������	
	IBaseMsgs*				m_baseMsgs;				// �������Ϣ���սӿ�
	CSysConfigFile*			m_sysConfig;			// �û������ļ���
	INIT_CONF				m_InitConfig;			// �����ļ�
	CMySocket				m_Socket;				// ��Ϣ����
	CMySocket				m_SocketEx;				// �ÿͽӴ�����
	string					m_loginName;			// ��¼��
	string					m_password;				// ��¼����
	string					m_server;				// ��������ַ
	string					m_lastError;			// ��һ�δ�����Ϣ
	bool					m_isAutoLogin;			// �Ƿ��Զ���¼
	bool					m_isKeepPwd;			// �Ƿ��ס����
	bool					m_isLoginSuccess;		// �Ƿ��ѵ�¼
	int						m_port;					// �������˿�
	CUserObject				m_UserInfo;				// ��¼�û�����Ϣ	
	unsigned short			m_usSrvRand;			// �������������
	unsigned short			m_usCltRand;			// �������е������

	map<unsigned long, CUserObject*> m_mapUsers;
	map<string, CTimer*> m_mapTimers;
	CUserObject m_CommUserInfo;	//�����û�
	int m_myInfoIsGet;		//�Ƿ��ҵ���Ϣ�Ѿ���ȡ����
	int						m_nOnLineStatus;		// �û��Ƿ�����
	
};


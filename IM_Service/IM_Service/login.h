#pragma once

#include "./socket.h"
#include "./auth/auth_client.h"
#include "./common/comobject.h"
#include "./common/comfunc.h"
#include "./common/commcom.h"
#include "./socket.h"

typedef void (*CALL_BACK_PROGRESS)(int percent);

class CChatManager;

class CLogin : public IBaseReceive
{
public:
	CLogin();
	~CLogin();

	friend class CMySocket;

public:
	void GetPreLoginInfo(tstring& loginName, tstring& password,	bool& isAutoLogin, bool& isKeepPwd); // ��ȡ��һ�ε�¼��Ϣ

	void StartLogin(tstring loginName, tstring password, bool isAutoLogin, bool isKeepPwd, CALL_BACK_PROGRESS pCallBack);	// ��ʼ��¼

private:
	virtual void OnReceive(void* pHead, void* pData);

	bool CheckLoginInfo(tstring loginName, tstring password,bool isAutoLogin, bool isKeepPwd);

	int GetTqAuthToken(unsigned int &uin, const char *szStrid, const char *szPassWord);

	bool LoadINIResource();

	bool CheckLoginFlag(unsigned long uin, const tstring& strID, bool bCreate = false, bool bFree = false);

	bool ReadSystemConfig();

	void SetSystemConfigByInitconfig();

	void SolveUserdefineFilter(FILTER_USERDEFINE &filter, char *sfilter);

	bool LoadUserConfig();  //������ϯ�����ڼ���һ�������ļ�

	bool LoginToRealServer(tstring strServer, int nPort, unsigned int uin);

	int ConnectToServer(tstring sip, unsigned short port);

	int SendLoginInfo(unsigned int uin);

	int SendPackTo(CPackInfo *pPackInfo, unsigned long recvuid = 0, unsigned long recvsock = 0, HWND hWnd = NULL);

	int RecvSrvConfLogon(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvRepUserinfo(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvDenyLogon(PACK_HEADER packhead, char *pRecvBuff, int len, int &errtype);

	int UnPack(CPackInfo *pPackInfo, char *buff, int len);
private:
	CALL_BACK_PROGRESS m_callbackProgress;
	tstring m_lastError;	

public:
	CChatManager* m_manager;
	CSysConfigFile* m_sysConfig;
	CTqAuthClient *m_pTqAuthClient;
	char m_szAuthtoken[MAX_256_LEN + 1];//�ĳɹ�����
	unsigned long m_authadminid;
	int m_unreadnoterecnum;
	int m_unreadphonerecnum;
	int m_nSendAuthToken;
	INIT_CONF m_InitConfig;
	int m_nLoginBy;					//��ǰ�ĵ�½����
	CUserObject m_UserInfo;
	unsigned short	m_usCltRand;	//�������е������
	unsigned short	m_usSrvRand;	//�������������
	tstring m_loginName;			// ��¼��
	tstring m_password;				// ��¼����
	bool m_isAutoLogin;				// �Ƿ��Զ���¼
	bool m_isKeepPwd;				// �Ƿ��ס����
	tstring	m_server;
	int		m_port;

public:
	CMySocket	m_Socket;	// ��Ϣ����
	CMySocket	m_SocketEx; // �ÿͽӴ�����
};
#pragma once

#include "sys_config.h"
#include "chat_objects.h"
#include "msgs.h"
#include "login.h"
#include "../chat_common/comobject.h"
#include "../chat_common/comstruct.h"
#include "../chat_common/comfloat.h"
#include "timer.h"

typedef map<string/*thirdId*/, string/*���ں�token*/> WxTokens; // ���ںŵ�thirdid��tokenһһ��Ӧ
typedef map<unsigned long, CUserObject*> MapUsers; // ������ϯ�û�

class IBaseMsgs
{
public:
	// ��¼�Ľ���,percent=100ʱ��ʾ��¼�ɹ�
	virtual void LoginProgress(int percent) = 0;

	// �յ�һ����ϯ�û�����Ϣ,������ʼ����ϯ�б�
	virtual void RecvOneUserInfo(CUserObject* pWebUser) = 0;

	// �յ�һ���½��ĻỰ��Ϣ
	virtual void RecvCreateChat(CWebUserObject* pWebUser) = 0;

	// �յ�һ���Ự��Ϣ
	virtual void RecvChatInfo(CWebUserObject* pWebUser) = 0;

	// �յ������û�������״̬
	virtual void RecvUserStatus(CUserObject* pUser) = 0;

	// ��ȡ��һ�δ�����Ϣ
	virtual string GetLastError() = 0;

	/*************************************************
	Description: �յ�һ����Ϣ
	
	@param:	pObj ����Ķ���������ϯ��web�ÿ͡�΢�ŷÿ�
	@param: msgFrom ��Ϣ�ķ��������ͣ�������ϯ���ÿ�(΢�Ż�web)��Э������(��һ����ϯ)
	@param: msgId ��Ϣ��Ψһid
	@param: msgType ��Ϣ���ͣ�Ԥ֪��Ϣ(��Ҫ���web�ÿ�)����ͨ��Ϣ
	@param: msgDataType ��Ϣ���������ͣ�����(��������)��������ͼƬ�����ꡢ��Ƶ��
	@param: msgContent ��Ϣ�ľ�������
	@param: msgTime �յ���Ϣ��ʱ��
	@param: pAssistUser	Э�����󣬵���ϢΪЭ��������ʱ����Ҫ�ò���
	@param: msgContentWx ΢����Ϣ���������ֵ�΢����Ϣʱ����Ҫ�ò���
	@param: msgExt Ԥ���Ĳ���

	Return: ��
	*************************************************/
	virtual void RecvOneMsg(IBaseObject* pObj, int msgFrom, string msgId, int msgType, int msgDataType,
		string msgContent, string msgTime, CUserObject* pAssistUser, WxMsgBase* msgContentWx, string msgExt) = 0;

	// ��ϯ������Ϣ
	virtual void RecvOnline(CUserObject* pUser) = 0;

	// ��ϯ������Ϣ
	virtual void RecvOffline(CUserObject* pUser) = 0;

	// �Ự�ر�
	virtual void RecvCloseChat() = 0;
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

	// ��������״̬��Ϣ
	int SendTo_UpdateOnlineStatus(unsigned short status);

	// ���ͻ�ȡ���Ѷ����б��¼�
	int SendTo_GetShareList();

	// ���ͻ�ȡ�Ự�б��¼�
	int SendTo_GetListChatInfo();

	// ���ͻ�ȡ�����û�����Ϣ
	int SendTo_GetAllUserInfo();

	// ���ͻ�ȡĳ���û�����Ϣ
	int SendTo_GetUserInfo(unsigned long uid);

	int SendTo_GetWebUserInfo(unsigned long webuserid, const char *chatid, char *szMsg = "", unsigned int chatkefuid = 0);

	//��ȡ�Ự��Ϣ��Ϣ
	int SendTo_GetWebUserChatInfo(unsigned short gpid, unsigned long adminid, char *chatid);

	// ��ȡϵͳ���ö���
	CSysConfig* GetSysConfig();

	// ��ȡ������������ʵ��
	CChatObjects* GetChatObjects();

	// ��ȡ������Ϣ������ʵ��
	CMsgs* GetMsgs();

	// ��ͼ
	void ScreenCapture();

	static void CALLBACK TimerProc(string timeName, LPVOID pThis);

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

	void RecvComSendNormalChatidHisMsg(unsigned long senduid, unsigned long recvuid, COM_SEND_MSG& RecvInfo);

	int RecvComTransRequest(unsigned long senduid, COM_SEND_MSG& RecvInfo);

	int RecvComTransAnswer(unsigned long senduid, COM_SEND_MSG& RecvInfo);

	void RecvComSendWorkBillMsg(unsigned long senduid, unsigned long recvuid, char *msg, char* mobile);


	/***************     ��ϯ�ͷÿ���Ϣ������      *****************/

	int UnPack(CPackInfo *pPackInfo, char *buff, int len);

	int SendPackTo(CPackInfo *pPackInfo, unsigned long recvuid = 0, unsigned long recvsock = 0, HWND hWnd = NULL);

	CUserObject *AddUserObject(unsigned long id, char *sid, char *name, unsigned char status, unsigned long fd);

	CUserObject *GetUserObjectByUid(unsigned long id);

	CWebUserObject *GetWebUserObjectBySid(char *sid);

	CWebUserObject *GetWebUserObjectByUid(unsigned long uid);

	CWebUserObject *AddWebUserObject(char *sid, char *thirdid, char *name, char *scriptflag, char *url,
		unsigned char status, unsigned char floatauth);

	unsigned short GetPackSeq();

	int SendGetClkInfo(unsigned long   id, char *strid, unsigned short cmd, unsigned short cmdtype = 0, unsigned short type = 0);

	void TimerSolveAck();

	int SendPing();

	void SetOfflineStatus();

	void CloseAllSocket();


	/***************     ����ʹ�õķ��͵�����˵���Ϣ      *****************/

	int SendPingToVisitorServer();

	int SendBuffToVisitorServer(char *sbuff, int len);

	int SendAckEx(unsigned short seq, unsigned long uid = 0, unsigned long ip = 0, unsigned short port = 0);

	int SendAutoRespMsg(CWebUserObject *pWebUser, const char *msg, BOOL bClearTimer = true);

	//���Ϳ�ʼͨ�����ÿͽӴ�������
	int SendWebuserTalkBegin(CWebUserObject *pWebUser);

	//���ͽ���ͨ�������ÿͽӴ�������
	int SendWebuserTalkEnd(CWebUserObject *pWebUser);



	// ������Ϣ�еı����ַ�ת��
	void TransforFaceMsg(string& str);

	// ΢����Ϣ�Ľ���
	WxMsgBase* ParseWxMsg(CWebUserObject* pWebUser, COM_FLOAT_CHATMSG& recv);

	string GetMsgId();

	void SaveEarlyMsg(MSG_INFO *pMsgInfo);

	void SolveWebUserEarlyMsg(CWebUserObject *pWebUser);

	void SaveUserConfig();

public:
	int						m_nOnLineStatus;		// �û��Ƿ�����
	int						m_nOnLineStatusEx;

private:
	CLogin*					m_login;				// ��¼������
	CChatObjects*			m_chatObjects;			// ��Ϣ������	
	IBaseMsgs*				m_baseMsgs;				// ��Ϣ���սӿ�
	CSysConfigFile*			m_sysConfig;			// �û������ļ���
	INIT_CONF				m_initConfig;			// ��������ļ���
	CMySocket				m_socket;				// ��Ϣ����
	CMySocket				m_socketEx;				// �ÿͽӴ�����
	string					m_loginName;			// ��¼��
	string					m_password;				// ��¼����
	string					m_server;				// ��������ַ
	string					m_lastError;			// ��һ�δ�����Ϣ
	bool					m_isAutoLogin;			// �Ƿ��Զ���¼
	bool					m_isKeepPwd;			// �Ƿ��ס����
	bool					m_isLoginSuccess;		// �Ƿ��ѵ�¼
	int						m_port;					// �������˿�
	CUserObject				m_userInfo;				// ��¼�û�����Ϣ	
	unsigned short			m_usSrvRand;			// �������������
	unsigned short			m_usCltRand;			// �������е������
	MapUsers				m_mapUsers;				// Э������Ĵ洢����	
	CUserObject				m_CommUserInfo;			// �����û�
	int						m_myInfoIsGet;			// �Ƿ��ҵ���Ϣ�Ѿ���ȡ����	
	CTimerManager*			m_timers;				// ��ʱ��������
	int						m_nMyInfoIsGet;			// �Ƿ��ҵ���Ϣ�Ѿ���ȡ����
	int						m_nSendPing;			// ���������͵Ĵ���
	int						m_nSendPingFail;		// ����������ʧ�ܴ��� 
	int						m_nLoginToVisitor;		// 
	time_t					m_tResentVisitPackTime;	// 
	unsigned long			m_nNextInviteWebuserUid;// ����ķÿ���Ϣ
	unsigned long			m_nNextInviteUid;		// ����������û�
	WxTokens				m_mapTokens;			// ���ں�token�洢����
	MMutex					m_idLock;				// ������Ϣid����
	int						m_msgId;				// ��Ϣid������
	list<MSG_INFO*>			m_listEarlyMsg;			// ���滹δ��ʼ���ÿͶ���֮ǰ�յ�����Ϣ
};


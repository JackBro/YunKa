#pragma once

#include "login.h"
#include "timer.h"
#include "chat_visitor.h"
#include "../chat_common/comobject.h"
#include "../chat_common/comstruct.h"
#include "../chat_common/comfloat.h"

typedef map<string/*thirdId*/, string/*���ں�token*/> WxTokens; // ���ںŵ�thirdid��tokenһһ��Ӧ
typedef map<unsigned long, CUserObject*> MapUsers; // ������ϯ�û�
typedef map<char*, CWebUserObject*> MapWebUsers; // ����ÿ�

// ��¼��Ϣ�Ļص��ӿ�
class IHandlerLgoin
{
public:
	// ��¼�Ľ���,percent=100ʱ��ʾ��¼�ɹ�
	virtual void LoginProgress(int percent) = 0;
};

// ��¼��ͨ����Ϣ�Ļص��ӿ�
class IHandlerMsgs
{
public:
	
	// �յ�һ����ϯ�û�����Ϣ,������ʼ����ϯ�б�
	virtual void RecvOneUserInfo(CUserObject* pWebUser) = 0;

	// �յ�һ���½��ĻỰ��Ϣ
	virtual void RecvCreateChat(CWebUserObject* pWebUser) = 0;

	// �յ�һ���Ự��Ϣ
	virtual void RecvChatInfo(CWebUserObject* pWebUser) = 0;

	// �յ������û�������״̬
	virtual void RecvUserStatus(CUserObject* pUser) = 0;

	// ��ϯ������Ϣ
	virtual void RecvOnline(CUserObject* pUser) = 0;

	// ��ϯ������Ϣ
	virtual void RecvOffline(CUserObject* pUser) = 0;

	//************************************
	// Method:    RecvAcceptChat
	// Qualifier: ��ϯ���ܻỰ��֪ͨ��Ϣ
	// Parameter: pUser ���ܻỰ����ϯ����
	// Parameter: pWebUser �����ܵķÿͶ���
	//************************************
	virtual void RecvAcceptChat(CUserObject* pUser, CWebUserObject* pWebUser) = 0;

	//************************************
	// Method:    RecvCloseChat
	// Qualifier: �Ự�ر�֪ͨ��Ϣ
	// Parameter: pWebUser �Ự�ر���صķÿ�
	//************************************
	virtual void RecvCloseChat(CWebUserObject* pWebUser) = 0;

	// ��ȡ��һ�δ�����Ϣ
	virtual string GetLastError() = 0;

	//************************************
	// Method:    RecvOneMsg
	// Qualifier: �յ�һ����Ϣ
	// Parameter: pObj ����Ķ���������ϯ��web�ÿ͡�΢�ŷÿ�
	// Parameter: msgFrom ��Ϣ�ķ��������ͣ�������ϯ���ÿ�(΢�Ż�web)��Э������(��һ����ϯ)
	// Parameter: msgId ��Ϣ��Ψһid
	// Parameter: msgType ��Ϣ���ͣ�Ԥ֪��Ϣ(��Ҫ���web�ÿ�)����ͨ��Ϣ
	// Parameter: msgDataType ��Ϣ���������ͣ�����(��������)��������ͼƬ�����ꡢ��Ƶ��
	// Parameter: msgContent ��Ϣ�ľ�������
	// Parameter: msgTime �յ���Ϣ��ʱ��
	// Parameter: pAssistUser Э�����󣬵���ϢΪЭ��������ʱ����Ҫ�ò���
	// Parameter: msgContentWx ΢����Ϣ���������ֵ�΢����Ϣʱ����Ҫ�ò���
	// Parameter: msgExt Ԥ���Ĳ���
	//************************************
	virtual void RecvOneMsg(IBaseObject* pObj, int msgFrom, string msgId, int msgType, int msgDataType,	string msgContent, 
		string msgTime = "", CUserObject* pAssistUser = NULL, WxMsgBase* msgContentWx = NULL, string msgExt = "") = 0;
};

class CChatManager : public IBaseReceive
{
public:

	~CChatManager();

	// ��ȡͨ�Ź�����ʵ��������ģʽ
	static CChatManager* GetInstance();

	// ���ý��յ�¼��Ϣ�Ľӿ�
	void SetHandlerLogin(IHandlerLgoin* handlerLogin);

	// ���ý��յ�¼��ͨ����Ϣ�Ľӿ�
	void SetHandlerMsgs(IHandlerMsgs* handlerMsgs);

public:

	// ��ȡ��һ�ε�¼��Ϣ,�����߲��ù�������
	ListLoginedInfo GetPreLoginInfo();

	// ��ʼ��¼
	void StartLogin(string loginName, string password, bool isAutoLogin, bool isKeepPwd);

	// ��������״̬����Ϣ
	int SendTo_UpdateOnlineStatus(unsigned short status);

	// ���ͻ�ȡ���Ѷ����б����Ϣ
	int SendTo_GetShareList();

	// ���ͻ�ȡ�Ự�б����Ϣ
	int SendTo_GetListChatInfo();

	// ���ͻ�ȡ�����û�����Ϣ����Ϣ
	int SendTo_GetAllUserInfo();

	// ���ͻ�ȡĳ����ϯ��Ϣ����Ϣ
	int SendTo_GetUserInfo(unsigned long uid);

	// ���ͻ�ȡĳ���ÿ���Ϣ����Ϣ
	int SendTo_GetWebUserInfo(unsigned long webuserid, const char *chatid, char *szMsg = "", unsigned int chatkefuid = 0);

	// ���ͻ�ȡĳ���Ự��Ϣ����Ϣ
	int SendTo_GetWebUserChatInfo(unsigned short gpid, unsigned long adminid, char *chatid);

	// ��ͼ
	void ScreenCapture();
	
	// �˳��������˳�ʱ����
	void Exit();

private:
	CChatManager();
	friend class CLogin;
	friend class CMySocket;

	/***************     �̳нӿڵĺ���ʵ��    *****************/

	virtual void OnReceive(void* pHead, void* pData);


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
	int RecvSrvDown(PACK_HEADER packhead, char *pRecvBuff, int len);

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

	void TimerSolveAck();

	int SendPing();

	void SetOfflineStatus();

	void CloseAllSocket();


	/***************     ����ʹ�õķ��͵�����˵���Ϣ      *****************/

	int Send_GetInfo(unsigned long id, char *strid, unsigned short cmd, unsigned short cmdtype = 0, unsigned short type = 0);

	int SendAckEx(unsigned short seq, unsigned long uid = 0, unsigned long ip = 0, unsigned short port = 0);

	int SendAutoRespMsg(CWebUserObject *pWebUser, const char *msg, BOOL bClearTimer = true);

	//��ʼ���ܻỰ��Ϣ
	int SendStartRecvFloatMsg(unsigned short gpid, unsigned long adminid, char *chatid, unsigned short sLastMsgid);

	// ��ȡuserinfo��token
	void SendGetWxUserInfoAndToken(CWebUserObject* pWebUser);

	// ��ȡ΢���û���Ϣ
	int SendGetWxUserInfo(unsigned long webuserid, const char *chatid);

	// ��ȡ΢�Ź��ں�token
	int SendGetWxToken(unsigned long webuserid, const char *chatid);

	// ������Ϣ�еı����ַ�ת��
	void TransforFaceMsg(string& str);

	// ΢����Ϣ�Ľ���
	WxMsgBase* ParseWxMsg(CWebUserObject* pWebUser, COM_FLOAT_CHATMSG& recv);

	string GetMsgId();

	void SaveEarlyMsg(MSG_INFO *pMsgInfo);

	void SolveWebUserEarlyMsg(CWebUserObject *pWebUser);

	void SaveUserConfig();

	CWebUserObject *ChangeWebUserSid(CWebUserObject *pWebUser, char *sid, char *thirdid);

	void GetChatSysMsg(char* msg, CUserObject *pInviteUser, CWebUserObject *pWebUser, int type, CUserObject *pAcceptUser = NULL);

	// ��ʱ���¼�������
	static void CALLBACK TimerProc(string timeName, LPVOID pThis); 

public:
	int						m_nOnLineStatus;		// �û��Ƿ�����
	int						m_nOnLineStatusEx;
	bool					m_bExit;
	CUserObject				m_userInfo;				// ��¼�û�����Ϣ

private:
	CLogin*					m_login;				// ��¼������	
	IHandlerLgoin*			m_handlerLogin;			// ��¼��Ϣ���սӿ�
	IHandlerMsgs*			m_handlerMsgs;			// ͨ����Ϣ���սӿ�
	CSysConfigFile*			m_sysConfig;			// �û������ļ���
	INIT_CONF				m_initConfig;			// ��������ļ���
	CMySocket				m_socket;				// ��Ϣ����
	CChatVisitor*			m_vistor;				// �ÿͽӴ��������
	string					m_loginName;			// ��¼��
	string					m_password;				// ��¼����
	string					m_server;				// ��������ַ
	string					m_lastError;			// ��һ�δ�����Ϣ
	bool					m_isAutoLogin;			// �Ƿ��Զ���¼
	bool					m_isKeepPwd;			// �Ƿ��ס����
	bool					m_isLoginSuccess;		// �Ƿ��ѵ�¼
	int						m_port;					// �������˿�		
	unsigned short			m_usSrvRand;			// �������������
	unsigned short			m_usCltRand;			// �������е������
	MapUsers				m_mapUsers;				// Э������Ĵ洢����
	MapWebUsers				m_mapWebUsers;			// �ÿ͵Ĵ洢����
	CUserObject				m_CommUserInfo;			// �����û�
	int						m_myInfoIsGet;			// �Ƿ��ҵ���Ϣ�Ѿ���ȡ����	
	CTimerManager*			m_timers;				// ��ʱ��������
	int						m_nMyInfoIsGet;			// �Ƿ��ҵ���Ϣ�Ѿ���ȡ����
	int						m_nSendPing;			// ���������͵Ĵ���
	int						m_nSendPingFail;		// ����������ʧ�ܴ��� 
	int						m_nLoginToVisitor;		// 
	 
	unsigned long			m_nNextInviteWebuserUid;// ����ķÿ���Ϣ
	unsigned long			m_nNextInviteUid;		// ����������û�
	WxTokens				m_mapTokens;			// ���ں�token�洢����
	MMutex					m_idLock;				// ������Ϣid����
	int						m_msgId;				// ��Ϣid������
	list<MSG_INFO*>			m_listEarlyMsg;			// ���滹δ��ʼ���ÿͶ���֮ǰ�յ�����Ϣ
	int						m_nClientIndex;			// �ÿ͵����кţ�����
};


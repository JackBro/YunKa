#ifndef _COMOBJECT_EMC
#define _COMOBJECT_EMC

#include "comdef.h"
#include "tstring.h"
#include "comstruct.h"

#include <list>     
using namespace std;

//��ʼ����
class CSysConfigFile
{
public:
	CSysConfigFile();
	~CSysConfigFile();

	void ResetValue();
	bool LoadUserData(char *sFilename, unsigned short ver);
	bool LoadData(char *sFilename, unsigned short ver);
	bool SaveData(char *sFilename, unsigned short ver);
	void SetWndInitPos(bool bAlways);
	void DeleteAll();
	void DeleteAllLoginInfo();
	void DeleteAllAlertInfo();

	ALERT_INFO *SetAllDefaultAlertInfo(int type = -1);
	ALERT_INFO *CSysConfigFile::GetAlertInfo(int type);

public:
	char m_sConfigname[MAX_FILENAME_LEN];

public:
	//ͨ����¼�Ż�õ�¼�ַ���
	tstring GetStringFromUID(unsigned long uid);
	unsigned long GetUIDFromString(int type, tstring strID);

	//�鿴���û����б��е�λ��
	int GetLoginInfoIndex(int type, unsigned long uid, tstring strID);
	LOGIN_INFO* GetLoginInfo(int type, unsigned long uid, tstring sid);

	//����һ���µĶ���
	LOGIN_INFO *AddOneLoginInfo(unsigned long uid, tstring sid, tstring pass, tstring compid);
	LOGIN_INFO *AddOneLoginInfo(LOGIN_INFO *LoginInfo);

	//����һ����¼����������
	void AddServerAddress(tstring strServer, bool bMoveToTop = true);
	void CSysConfigFile::AddKeyWordSerchString(tstring strSearch, bool bMoveToTop);

	void CSysConfigFile::AddOrDeleteForbidWebuserSid(tstring sid, bool bAdd);
	bool CSysConfigFile::IsWebuserSidForbid(tstring sid);

	tstring CSysConfigFile::CombineFilterString();
	void CSysConfigFile::ParseFilterString(tstring strString);

	int nStartFlag, nEndFlag;

	bool bIsFirstRun;
	bool bFindSrv;

	bool bAutoSearchKeyword;

	bool bSavePass;			//�Ƿ񱣴�����
	bool bAutoLogin;		//�Ƿ��Զ���¼
	bool bHideLogin;		//�Ƿ������¼
	int nKeySendType;		//0 enter; 1 alt+enter

	int nClientCopyInfo;	//�汾��Ϣ 
	int nServerCopyInfo;	//�汾��Ϣ

	unsigned long dwLastUpdateTime;
	int nCopyCheckType;				//0 not check 1 auto 2 timer
	int nCopyCheckTimeMWD;				//0 month day 1 week 2 day
	int nCopyCheckTimeMonth, nCopyCheckTimeWeek, nCopyCheckTimeDay;

	int nCopyDownType;			//0 ��̨���� 1 ��ʾ����

	//��ʼ���ڴ�С������
	int nWndDock;	//���ڵ�ͣ��λ��
	int x, y, w, h;
	bool bThickMainWnd;
	int colorIndex;	//tree����ɫ

	int nInfoHeight;
	int nSendMsgHeight;
	int nPreSendMsgHeight;

	//����¼������
	int lastloginby;
	unsigned long lastloginuid;
	char lastloginstr[MAX_SID_LEN];
	char lastloginpass[MAX_PASSWORD_LEN];
	int lastloginstatus;
	char lastloginsstatus[MAX_STATUS_LEN];
	char lastlogincompid[MAX_STATUS_LEN];

	//����¼�ķ�������Ϣ
	int nConnectType;	//��������
	tstring strServer;
	int nServerPort;

	//�ÿͽӴ�
	tstring strVisitorServer;
	int nVisitorServerPort;

	tstring strRealServer;
	int nRealServerPort;

	int nProxyType;
	tstring strProxyServer;
	int nProxyPort;
	int nProxyLocalPort;
	tstring strProxyUser, strProxyPass;

	//�����¼���ĵ�¼��Ϣ
	list<LOGIN_INFO*> m_listLogin;

	//�������ѷ�ʽ
	list<ALERT_INFO*> m_listAlertInfo;

	//�����½���ķ�������ַ
	list<tstring> m_listServerAddress;

	//������������
	list<tstring> m_listKeyWordSearchString;

	//�������εķÿͱ�ʾ
	list<tstring> m_listForbidWebUserSid;

	int	 filtertype;

	FILTER_USERDEFINE infofilter;
	FILTER_USERDEFINE tailfilter;

	bool	m_bAutoResp;
	tstring	m_strWellcomeMsg;

	int		m_nUserTimeoutTime;
	tstring	m_strUserTimeoutMsg;

	int		m_nVisitorTimeoutTime;
	tstring	m_strVisitorTimeoutMsg;

	bool	m_bVisotorTimeoutClose;
	int		m_nVisitorTimeoutCloseTime;
	tstring	m_strVisitorTimeoutCloseMsg;

	bool	m_bAutoRespUnnormalStatus;
	tstring	m_strUnnormalStatusMsg[5];

	tstring	m_strInviteWords;
	tstring	m_strInviteTitle;

	//�����Զ�������
	list<tstring> m_listInviteWords;
	int		m_nInviteType;

	int m_nKeywordsSort;
	int m_nKeywordsUser;
	tstring m_strKeywordsFind;
};

class CBaseObject
{
public:
	CBaseObject(){};
	~CBaseObject(){};

	virtual bool Load(unsigned short ver)=0;
	virtual bool Save(unsigned short ver)=0;
};

class CUserObject : public CBaseObject
{
public:
	CUserObject();
	~CUserObject();

	bool Load(unsigned short ver);
	bool Save(unsigned short ver);

public:
	USER_INFO UserInfo;
	unsigned char talkstatus;

	int	m_nWaitTimer;		//���룬 ��Ϣ���͵ȴ�ʱ�Ķ�ʱ����

	unsigned char status;
	char sstatus[MAX_STATUS_LEN];

	unsigned long talkuid;
	unsigned long fd;	//����socket���ӣ�������Ϣת��

	tstring m_strMsgHtml;
	tstring m_strMsgHtmlEx;	//��һ�������ݣ����������ǰ���Ǹ��û�����ʾ�����ȴ��л������û�ʱ����ӵ�m_strMsgHtml��
	list<ONE_MSG_INFO> m_strMsgs;// ���������¼
	tstring m_strPreSendMsg;
	bool m_bConnected;

	bool m_bKeywordsChange;
	bool m_bKeywordsGet;

public:
	int	nTimer;  //��ʱ��

	int nVisitNum;
	int nTalkNum;
	int nLastVisitTime;

	int nFlashFirstOnline;			//��һ��������˸��־ 0 ������1 -> ��ʼ��

	unsigned long nbak;
	tstring sbak;

	bool bVisitMeAfterLogon;		//���û���������û�н�����
	bool bTalkMeAfterLogon;			//���û���������û�н�����

	int m_nFlag;					//������ʱʹ�ñ���
	bool m_bOpen;
	bool m_bReset;

	bool m_bFriend;					//�Ƿ��кö���
	bool m_bInnerTalk;				//�Ƿ��ڲ��Ի�
};

class IBaseReceive
{
public:
	virtual void OnReceive(void* pHead, void* pData) = 0;
};

#endif
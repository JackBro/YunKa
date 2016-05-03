#ifndef _COMOBJECT_EMC
#define _COMOBJECT_EMC

#include "comdef.h"
#include "string.h"
#include "comstruct.h"
#include "wx_obj.h"

#include <list>     
using namespace std;

class IBaseReceive
{
public:
	virtual void OnReceive(void* pHead, void* pData) = 0;
};

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
	string GestringFromUID(unsigned long uid);
	unsigned long GetUIDFromString(int type, string strID);

	//�鿴���û����б��е�λ��
	int GetLoginInfoIndex(int type, unsigned long uid, string strID);
	LOGIN_INFO* GetLoginInfo(int type, unsigned long uid, string sid);

	//����һ���µĶ���
	LOGIN_INFO *AddOneLoginInfo(unsigned long uid, string sid, string pass, string compid);
	LOGIN_INFO *AddOneLoginInfo(LOGIN_INFO *LoginInfo);

	//����һ����¼����������
	void AddServerAddress(string strServer, bool bMoveToTop = true);
	void CSysConfigFile::AddKeyWordSerchString(string strSearch, bool bMoveToTop);

	void CSysConfigFile::AddOrDeleteForbidWebuserSid(string sid, bool bAdd);
	bool CSysConfigFile::IsWebuserSidForbid(string sid);

	string CSysConfigFile::CombineFilterString();
	void CSysConfigFile::ParseFilterString(string strString);

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
	string strServer;
	int nServerPort;

	//�ÿͽӴ�
	string strVisitorServer;
	int nVisitorServerPort;

	string strRealServer;
	int nRealServerPort;

	int nProxyType;
	string strProxyServer;
	int nProxyPort;
	int nProxyLocalPort;
	string strProxyUser, strProxyPass;

	//�����¼���ĵ�¼��Ϣ
	list<LOGIN_INFO*> m_listLogin;

	//�������ѷ�ʽ
	list<ALERT_INFO*> m_listAlertInfo;

	//�����½���ķ�������ַ
	list<string> m_listServerAddress;

	//������������
	list<string> m_listKeyWordSearchString;

	//�������εķÿͱ�ʾ
	list<string> m_listForbidWebUserSid;

	int	 filtertype;

	FILTER_USERDEFINE infofilter;
	FILTER_USERDEFINE tailfilter;

	bool	m_bAutoResp;
	string	m_strWellcomeMsg;

	int		m_nUserTimeoutTime;
	string	m_strUserTimeoutMsg;

	int		m_nVisitorTimeoutTime;
	string	m_strVisitorTimeoutMsg;

	bool	m_bVisotorTimeoutClose;
	int		m_nVisitorTimeoutCloseTime;
	string	m_strVisitorTimeoutCloseMsg;

	bool	m_bAutoRespUnnormalStatus;
	string	m_strUnnormalStatusMsg[5];

	string	m_strInviteWords;
	string	m_strInviteTitle;

	//�����Զ�������
	list<string> m_listInviteWords;
	int		m_nInviteType;

	int m_nKeywordsSort;
	int m_nKeywordsUser;
	string m_strKeywordsFind;
};

class IBaseObject
{
public:
	IBaseObject(){};
	~IBaseObject(){};

	virtual bool Load(unsigned short ver)=0;
	virtual bool Save(unsigned short ver)=0;

	int m_nEMObType;
};

class CUserObject : public IBaseObject
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

	string m_strMsgHtml;
	string m_strMsgHtmlEx;	//��һ�������ݣ����������ǰ���Ǹ��û�����ʾ�����ȴ��л������û�ʱ����ӵ�m_strMsgHtml��
	list<ONE_MSG_INFO> m_strMsgs;// ���������¼
	string m_strPreSendMsg;
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
	string sbak;

	bool bVisitMeAfterLogon;		//���û���������û�н�����
	bool bTalkMeAfterLogon;			//���û���������û�н�����

	int m_nFlag;					//������ʱʹ�ñ���
	bool m_bOpen;
	bool m_bReset;

	bool m_bFriend;					//�Ƿ��кö���
	bool m_bInnerTalk;				//�Ƿ��ڲ��Ի�
};

class CWebUserObject : public IBaseObject
{
public:
	CWebUserObject();
	~CWebUserObject();

	//������Ϣ
	bool Load(unsigned short ver);
	bool Save(unsigned short ver);

	void AddCommonTalkId(unsigned long uid);

	bool IsExistCommonTalkId(unsigned long uid);

	bool IsOnline();

public:
	unsigned char	m_bNewComm;					//��Э��
	unsigned short  m_sNewSeq;					//��Э��,��ǰ���ܵ���Ϣid
	int				m_nIndex;					//���ڱ�ʾ�û�����������
	WEBUSER_INFO	info;
	WEBONLINE_INFO	onlineinfo;
	unsigned long	webuserid;
	unsigned long	floatadminuid;				//Ư�������Ա����
	int				nlangtype;					//�ַ���
	unsigned long	floatfromadminuid;			//�Ự��Դ��վ����Ա
	unsigned short	floatfromsort;				//�Ự����Ư����id
	char			chatid[MAX_CHATID_LEN + 1];	//����id��Ψһ����һ�λỰ
	unsigned short	gpid;

	//������Ϣ�����û���һ��������ʹ��
	unsigned long 	talkuid;					//�Ի����û� ��һ�ζԻ��û�
	unsigned long 	transferuid;				//Ҫת�ӵ��û�
	unsigned long 	inviteuid;					//������û���Ĭ��ΪSYS_WEBUSER
	unsigned long 	frominviteuid;				//����������û���

	unsigned char	cTalkedSatus;				//0û��ͨ������HASTALKED 1  ͨ����,INTALKING	2 ����ͨ����
	//CMapStringToOb m_ListUrlAndScriptFlagOb;	// �ж�web�û��Ƿ�����
	list<unsigned long> m_listCommonTalkId;		// ��ͬ�Ի��������ͷ��û��б�
	unsigned char	m_bNotResponseUser;			//1 �Լ�����Ӧ��ͷ���0�Լ���Ӧ��ͷ�
	char prevurl[MAX_URL_LEN];
	char prevurlhost[MAX_URL_LEN];
	char prevurlvar[MAX_URL_LEN];
	unsigned char	refuseinvite;				//�û��ܾ������룬 ���������Զ�����
	int	m_nWaitTimer;							//���룬 ��Ϣ���͵ȴ�ʱ�Ķ�ʱ����
	int m_resptimeoutmsgtimer;					//�ÿ�δӦ���Զ��ظ�ʱ��
	int m_resptimeoutclosetimer;				//�ÿͳ�ʱ��δ�ظ�����ʼ�Զ��رնԻ�
	int m_waitresptimeouttimer;					//�ͷ�δӦ���Զ��ظ�ʱ��
	//���������Զ�����
	unsigned long m_onlinetime;					//���ߵ�ʱ��
	time_t m_leaveTime;							// �������� [12/14/2010 SC]
	string m_strInfoHtml;
	string m_strTail;
	string m_strInfo;
	string m_strPreSendMsg;
	list<ONE_MSG_INFO> m_strMsgs;
	bool m_bConnected;							//���û��Ѿ����㽨�����ӣ�����ͨѶ��
	bool m_bIsGetInfo;							// �Ƿ��ѻ�ȡ����Ϣ
public:
	string m_strHistory;						//�ܵ���ʷ��¼����ʾ
	string m_strTotal;
	string m_strListMsg;
	string m_strListInfo;
	string m_strListTail;
	int	nTimer;									//��ʱ��
	int nVisitFrom;								//�ÿ���Դ
	int nVisitNum;
	int nTalkNum;
	int nLastVisitTime;
	unsigned long bak;
	int m_nFlag;								//������ʱʹ�ñ���
	bool m_bIsShow;								//�Ƿ���ʾ
	bool m_bIsFrWX;								//�Ƿ�����΢��
	string m_sWxAppid;							//�Ự��Դ��΢��appid
	bool GetNormalChatHisMsgSuccess;			//������ʷ��Ϣ
	time_t tGetNormalChatHismsgTime;

	WxUserInfo* m_pWxUserInfo;
};


#endif
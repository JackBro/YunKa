#if !defined _COMMFORM_EM
#define _COMMFORM_EM

//#define CMD_SRV_STATUS_FRDONLINE			0x006E	// 110 ֪ͨ���ҵ���������
//���������ң��ҵ���������
//CMD_SRV_INFORM_FRD_ONLINE
class SRV_STATUS_FRDONLINE:public CPackInfo
{
public:
	SRV_STATUS_FRDONLINE(unsigned short ver);
	~SRV_STATUS_FRDONLINE();

	bool unpack();  //read
	bool pack();  //write
	

	int GetStatus();
	void SetStatus(int status);

	int GetConnectType();
	void SetConnectType(int type);

	int GetClientType();
	void SetClientType(int type);

	int GetVideoType();
	void SetVideo(int type);
public:
	unsigned  int	uin;                  //�û���UIN

	char			nickname[MAX_USERNAME_LEN];						//�û�������
	unsigned  int	ip;                   //�û�������IP    
	unsigned  short port;                 //�û�������PORT

	unsigned short ver;
	unsigned short langtype;

	unsigned int status;               //�û�������״̬
	//0-3 ����״̬  
	//4-5 ���뷽ʽ
	//6-7 web/�ͻ����û�

	char	strstatus[MAX_STATUS_LEN+1];			//����״̬������

	unsigned short	sterminaltype;					//2 ������˾���

	unsigned short connecttype; //�������� �������汾>=3070֧��
	unsigned short mpid;        //�û�����mpid  �������汾>=3070֧��


	char   szRand[MAX_WEBCLIENID_LEN+1];  //rand����΢���û�id  mpver>=3073
	char   szThirdId[MAX_THIRDID_LEN+1];  //������id����΢�Ź��ں�id
};

//#define CMD_SRV_STATUS_FRDOFFLINE			0x0078	// 120 ֪ͨ���ҵ���������
//���������ң� �ҵ���������
class SRV_STATUS_FRDOFFLINE:public CPackInfo
{
public:
	SRV_STATUS_FRDOFFLINE(unsigned short ver);
	~SRV_STATUS_FRDOFFLINE();

	bool unpack();  //read
	bool pack();  //write
	

public:

	unsigned  int	uin;     //�����û���UIN����

	unsigned short	externid;					//2 ������˾���
	unsigned short	bak;
};

//#define CMD_SRV_SEARCH_FAIL					0x0096	// 150 ��ѯ����ʧ��
class SRV_SEARCH_FAIL:public CPackInfo
{
public:	
	SRV_SEARCH_FAIL(unsigned short ver);
	~SRV_SEARCH_FAIL();

	bool unpack();  //read
	bool pack();  //write
	

public:
	unsigned short		seq;   //���ز�ѯ��

	unsigned short      type;             //0 user is not exit
	char				reason[MAX_RESPONSE_LEN];		//255  �ܾ�����������
};

//#define CMD_SRV_UPDATE_FAIL					0x00D2	// 210 ����ʧ��
class SRV_UPDATE_FAIL:public CPackInfo
{
public:
	SRV_UPDATE_FAIL(unsigned short ver);
	~SRV_UPDATE_FAIL();

	bool unpack();  //read
	bool pack();  //write
	

public:
	unsigned short	seq;
	unsigned short	type;

	unsigned int	uin;

	unsigned short	deny;
	char			reason[MAX_RESPONSE_LEN];
};

//#define CMD_SRV_STATUS_USER_FORM					0x01A4	// 420 �û�����״̬
class SRV_STATUS_USER_FORM:public CPackInfo
{
public:
	SRV_STATUS_USER_FORM(unsigned short version);
	~SRV_STATUS_USER_FORM();

	int GetOnlineStatus();
	int GetConnectType();
	int GetUserType();

	bool unpack();  //read
	bool pack();  //write
	

public:
	unsigned short	type;	//״̬���ͱ���
    unsigned int	uin;			//�Ż��Ǹ��û���״̬
	
private:
	unsigned int	status;			//״̬
	/*
	�μ�USER_INFO ��onlineflag
		0 - 1   �û���������(4)��		0�� ֱ�ӽ��� 1�� ���������룻 2���������
		4 - 5  �û����				0�� ��ע���û��� 1 ע��ͻ�;  2����ʱ�û�
		8 - 12  �û�״̬(16)				0 �û�����״̬��1�����ߣ�2�����ߣ�3����ȥһ�£�
									4���벻Ҫ����
		  16      �Ƿ���Ƶ�ر�/��		0 û�У�   1 ��
	*/
public:
	char strstatus[MAX_STATUS_LEN];  //����״̬����

	unsigned int	ip;	//���ӣ����Ϊ���ߣ� �ӵ�ַ
	unsigned short	port;		//���ӣ����Ϊ���ߣ� �Ӷ˿�
};

//�ͻ��˵İ��Ĵ���
//old
//#define CMD_CLT_SEND_ALLMSG					0x03F2  //  1010 ����Ⱥ����Ϣ
//����Ⱥ������
class CLT_SEND_ALLMSG:public CPackInfo
{
public:
	CLT_SEND_ALLMSG(unsigned short ver);
	~CLT_SEND_ALLMSG();

	bool unpack();  //read
	bool pack();  //write

public:
    unsigned short			num;			//�����ߵ�UIN�ĸ���
	unsigned int listuin[MAX_LIST_UIN];	//�����ߵ�UIN

	//��Ϣ�ڲ���uin��Ч
	MSG_INFO	msg;

//	unsigned short langtype; 
//	unsigned short bak; 

private:
	unsigned maxnum;
};

//#define CMD_CLT_GET_CLT_INFO				0x0460  // 1120 ��ø�������
//�û���ѯ��������
//CMD_CLT_GET_CLT_INFO
class CLT_GET_CLT_INFO:public CPackInfo
{
public:
	CLT_GET_CLT_INFO(unsigned short ver);
	~CLT_GET_CLT_INFO();

	bool unpack();  //read
	bool pack();  //write

public:
	unsigned short	seq;             //��ѯ��
	unsigned int   uin;                 //Ҫ��ѯ���˵�UIN����
	unsigned short  type; 
};


//#define CMD_CLT_UPDATE_STATUS				0x04D8  // 1240 ����״̬
//��������״̬
class CLT_UPDATE_STATUS:public CPackInfo
{
public:
	CLT_UPDATE_STATUS(unsigned short ver);
	~CLT_UPDATE_STATUS();

	bool unpack();  //read
	bool pack();  //write

	
public:
	unsigned short seq;
	unsigned short type ; 
	/* ����
			0	����״̬
			2   ��Ƶ״̬
			10  ȫ����־λ��Ϣ
			11	commstatus
			12	extstatus
			13	onlinestatus
	*/

	unsigned int commstatus;
	unsigned int extstatus;
	unsigned int onlinestatus;

	char strstatus[MAX_STATUS_LEN];  //����״̬����������
 };

//#define CMD_CLT_UPDATE_INFO					0x0500  // 1280 �����û�����
//�����û�����
class CLT_UPDATE_INFO:public CPackInfo
{
public:
	CLT_UPDATE_INFO(unsigned short ver);
	~CLT_UPDATE_INFO();

	bool unpack();  //read
	bool pack();  //write

	

public:
	unsigned short seq;

	unsigned short type;//0  �û���Ϣ  1  ������Ϣ  2ȫ��

	CUserInfo userinfo;                     //Ҫ���µ��û���Ϣ
	CTradeInfo	tradeinfo;
};

//#define CMD_CLT_UPDATE_PW					0x0514  // 1300 ��������
//��������
class CLT_UPDATE_PW:public CPackInfo
{
public:
	CLT_UPDATE_PW(unsigned short ver);
	~CLT_UPDATE_PW();

	bool unpack();  //read
	bool pack();  //write

	

public:

	unsigned short seq;
	char	pw[MAX_PASSWORD_LEN];
};

//#define CMD_CLT_UPDATE_LOGINSTRING			0x051E  // 1310 ���µ�¼�������ַ���
//���µ�½������
class CLT_UPDATE_LOGINSTRING:public CPackInfo
{
public:
	CLT_UPDATE_LOGINSTRING(unsigned short ver);
	~CLT_UPDATE_LOGINSTRING();

	bool unpack();  //read
	bool pack();  //write

	

public:
	unsigned short seq;
	char	strid[MAX_SID_LEN];
};



//#define CMD_COM_FILE_SEND					0x0F8C  // 3980 �ͻ���������ĳһ���ļ�
class COM_FILE_SEND:public CPackInfo
{
public:
	COM_FILE_SEND(unsigned short version);
	~COM_FILE_SEND();

	bool unpack();  //read
	bool pack();  //write

	
public:
	unsigned short	seq;						//2 ���к�

	unsigned int	recvuin;					//4 ����UIN����
	unsigned int	senduin;					//4 �����ߵ�UIN����
	char			sendname[MAX_USERNAME_LEN];	//20 �����ߵ�����

	unsigned short	sendtoserver;				//2
	unsigned int	random;						//4 ����Ψһ��

	char			strip[MAX_IPNUM_LEN];		//20 �����ߵĵ�ַ
	unsigned short	port;						//2

	unsigned int	localip;					//2
	unsigned short	localport;					//2 �����ߵĶ˿�

	unsigned short	filetotal;					//2 �ļ�����Ŀ,Ŀ¼Ϊһ���ļ�
	unsigned int	filesize;					//4

	unsigned short	filenum;					//2
	FILE_INFO		m_ListFile[MAX_LIST_FILE];	//5 * 125

	unsigned int	startpos;					//4 ��ʼ��λ��, 0��ʾΪ��һ�η���, �Ը��ļ�ֻ�Ե�һ����Ч
	unsigned short	isauth;						//2 �Ƿ������֤  0  û�����ӣ�1 ����
	char			strauth[MAX_RESPONSE_LEN];	//255 ��֤��Ϣ 

private:
	unsigned short	maxfilenum;

};

class COM_VIDEOAUDIOTEXT_FORM:public CPackInfo
{
public :
	COM_VIDEOAUDIOTEXT_FORM(unsigned short version);
	~COM_VIDEOAUDIOTEXT_FORM();

	bool unpack();  //read
	bool pack();  //write

	
	void SetBuff(int len, char *pBuff);
public :
	unsigned int recvip[4];	//	recvip[0]   recvip[1] ������ip recvip[2] ����ip recvip [3] ����ip
	unsigned short recvport[4];	//

	unsigned short seq; //���
	unsigned short num;	//�ð��ķְ�����
	unsigned short index;		//�ð��ķְ����

	char sBuff[BUFFMAXLEN];		//����

public:
	bool sendtoserver;
	int len;

	unsigned int uin;
	unsigned short port;

	char	strip[MAX_IPNUM_LEN];
};

#endif
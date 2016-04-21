#ifndef _COMMDEF_COMMMSG
#define _COMMDEF_COMMMSG

//��ʱ��������
#define TIMER_SYSTEMMSG		1	//ϵͳ��Ϣ������˸��״̬����ͼ��
#define TIMER_TRAYMSG		2	//��Ϣ������˸��������ͼ��
#define TIMER_AFTERINIT		3	//������ʾ�����ϵ��ã��Ա��ⴰ�ڵȴ�
#define TIMER_COMMMAXTIME	4	//ͨѶ����ʱ������
#define TIMER_SENDUDP		5	//����UDP����ʱ����
#define TIMER_NORMAL		6	//�������Ķ�ʱ��������ping mutiping idleoffline
#define TIMER_EXIT			7	//�˳�ϵͳ�Ķ�ʱ��
#define TIMER_POPUPMENU		9	//����˵�����
#define TIMER_MEETINGPING	10	//�����û���ͣ�ķ�������Ϣ��������
#define TIMER_STAERTVIDEO	11	//������Ƶ���Ա��ⴰ�ڵȴ�
#define TIMER_TOPWND		12	//��ǰ�洰����ʧʱ��
#define TIMER_PROXY			14	//�������������
#define TIMER_LOGINOFF_TO	15	//�ǳ����ٵ�½��ע��
#define TIMER_AUTO_HIDE		16	//���߽�ʱ�Զ�����
#define TIMER_CONNECTMAXTIME	17	//һ����������ʱ������
#define TIMER_FIRSTONLINE	18	//��һ���û�������˸����
#define TIMER_SENDVIDEO		19	//���һ��Ķ�ʱ��������ping mutiping idleoffline
#define TIMER_AUTOLOGIN		20	//�Զ���¼
#define TIMER_DOWNLISTEN	50	//�ļ����ط����������ȴ�ʱ�䶨ʱ��
#define TIMER_VISITOR_ACCEPT 80
#define TIMER_TRANS_TIMEOUT 81 //ת�ӻỰ��ʱ���� gxl
#define TIMER_REFRESH_WEBPAGE 82 // ˢ����Ϣ��¼��ҳ��
#define TIMER_MAILCHECK		60	//�ʼ���ⶨʱ�� 60 -- 70  

//��ʱ�����
#define TIME_NORMALTIME			500
//һ��ͨѶ����ʱ��
#define TIME_COMMMAXTIME		8000
//���ڳ�ʼ�������ϵ��õ�ʱ��
#define TIME_AFTERINIT			100  
//ping��ʱ����
#define TIME_PING				40000 
//relogo��ʱ����
#define TIME_AUTOLOGON			30000 
//mutiping��ʱ����
#define TIME_MUTIPING			5000  
//�رջ������ݻ�ȡ�̵߳Ķ�ʱ��
#define TIME_MEETINGCLOSE		3000
//һЩ��ʾ���ڵ�ͣ��ʱ��
#define TIME_TIPBOX				3000

//�Զ����ߵĶ�ʱ��, 20��һ�Σ�ÿһ�μ�ʱ�������3������1���Ӻ����ߣ�������
#define TIME_IDLEOFFLINE		20000	

#define TIME_PROXY				30000
//�������Ϣ


//socket��ʧ��
#define	WM_SOCKET_RECVFAIL			(WM_USER + 10)	
#define	WM_SOCKET_RECVSUCC			(WM_USER + 20)
#define	WM_SOCKET_CLOSE				(WM_USER + 30)
#define WM_CLOSE_WND				(WM_USER + 40)

//��½�ɹ���
#define	WM_COMM_LOGINSUCC			(WM_USER + 60)

#define WM_HTML_TOOLBAR				 (WM_USER + 70)	
#define WM_TOOLBAR_MSG				 (WM_USER + 80)	

//�ÿͽӴ�socket
#define	WM_SOCKETEX_RECVFAIL			(WM_USER + 11)	
#define	WM_SOCKETEX_RECVSUCC			(WM_USER + 21)
#define	WM_SOCKETEX_CLOSE				(WM_USER + 31)

//
#define WM_TRANSFERFILE_PROGRESS	(WM_USER + 80)
#define WM_TRANSFERFILE_FINISH		(WM_USER + 90)
#define WM_TRANSFERFILE_GETSESSION	(WM_USER + 100)

#define WM_PARSE_XMLDATA			(WM_USER + 110)

#define WM_KEYWORD_EDIT				(WM_USER + 120)

#define	WM_NAVIGATE_BEFORE			(WM_USER + 130)
#define WM_NAVIGATE_BEFORE2			(WM_USER + 135)
//#define WM_INPLACE_NOTIFY			(WM_USER + 140)
#define WM_CHECKFALSHITEM			(WM_USER + 140)
#define WM_GETTIPSINFO_TREE			(WM_USER + 150)
#define WM_TREECOLOR_CHANGE			(WM_USER + 160)
/*
//ϵͳ��Ϣ������˸��״̬����ͼ��
#define WM_OPENSYSTEM			(WM_USER + 200)
#define WM_LOGIN_SERVER			(WM_USER + 210)
*/
//���յ����������󣬷��ص�����
#define WM_RESPONSE_DATA		(WM_USER + 220)
/*
//��������
#define WM_SEND_DATA			(WM_USER + 230)
//���յ���Ӧ������ʾ�����ͳɹ�
#define WM_SEND_SUCCESS			(WM_USER + 240)
//��������ʧ��
#define WM_SEND_FAIL			(WM_USER + 250)
//���հ�
#define WM_RECV_DATA			(WM_USER + 260)
//���յ�������Ϣ�İ�(�ӷ�������ȡ)
#define	WM_UPDATE_USERINFO		(WM_USER + 270)
//���յ��޸���Ϣ�İ�(�޸ĵ�������)
#define	WM_CHANGE_USERINFO		(WM_USER + 280)

//�ڹ����ļ��������иı乲���ļ�����ʾ
#define	WM_CHANGE_SHAREFILE		(WM_USER + 200)
//�ļ������з������ĸı�
#define	WM_DOWNFILE_SORT		(WM_USER + 210)
//�ļ������н��ȸı�
#define	WM_DOWNFILE_PROGRESS	(WM_USER + 220)
//�ļ��б���ɾ��
#define	WM_DOWNFILE_DELETET		(WM_USER + 230)

//����Ϣ�������иı�ĳһ�����ѵ���ʾ
#define	WM_CHANGE_UIN_MANAGER	(WM_USER + 250)
//����Ϣ�������иı�ĳһ�������ʾ
#define	WM_CHANGE_GROUP_MANAGER	(WM_USER + 260)
//����Ϣ�������иı�ĳһ�����������ʾ
#define	WM_MSGMEMO_CHANGE		(WM_USER + 270)
//����Ϣ�������иı�ĳһ��ѡ�������ʾ
#define	WM_MSGITEM_CHANGE		(WM_USER + 280)
//����Ϣ�������иı���
#define	WM_MSGITEM_CHANGECOUMNE	(WM_USER + 290)
*/
//���̷��͵���Ϣ
#define	WM_MSG_NOTIFYICON		(WM_USER + 300)
/*
//��Ϣ��������Ĺ�����
#define	WM_MSG_TALKMSGTOOLBAR	(WM_USER + 310)
*/
//ѡ����һ��ͼ��
#define	WM_MSG_ICONFACE			(WM_USER + 310)
#define	WM_MSG_KILLFOCUS		(WM_USER + 320)

/*
//����ı���Ϣ
#define	WM_MSG_FONT				(WM_USER + 321)
//�ֺŸı����Ϣ
#define	WM_MSG_SIZE				(WM_USER + 322)
//��ɫ�ı����Ϣ
#define	WM_MSG_COLOR			(WM_USER + 323)
//ת����Ϣ
#define	WM_MSG_RESEND			(WM_USER + 324)
//ת������
#define	WM_MSG_TRANSFERCLIENT	(WM_USER + 325)
//���ͽ����ļ�����Ϣ
#define	WM_MSG_SENDSCREEN		(WM_USER + 326)

//���������Ϣ
#define	WM_MOURSEKEYBOARDE_ACTIVE	(WM_USER + 350)

//���������Ϣ
#define	WM_TOPWND_RESULT		(WM_USER + 360)	//��ǰ�洰�ڵ�ѡ�ж���

//hyperlink����Ϣ
#define	WM_HYPERLINK_SEND				(WM_USER + 500)

//�ⲿ��Ӷ����ϵ�˵ĵ���Ϣ
#define	WM_ADDFRD_BYEXT					(WM_USER + 600)
*/
//tabctrl�������Ϣ
#define	WM_TABCTRL_LBUTTONDOWN			(WM_USER + 700)
//em tab window�������Ϣ
#define	WM_EMTAB_DOWN					(WM_USER + 710)

//em mis 
#define	WM_EMMIS_DOWN					(WM_USER + 740)
#define WM_EMMIS_DROPDOWN				(WM_USER + 741)
#define	WM_EMMIS_OUTOFWND				(WM_USER + 742)

/*
#define	WM_VOLUME_MUTE					(WM_USER + 742)
#define	WM_VOLUME_SIZE					(WM_USER + 743)

#define WM_RECORD_RESULT				(WM_USER + 750)
#define WM_RECORD_START					(WM_USER + 751)
#define WM_RECORD_BLOCK					(WM_USER + 752)
#define WM_RECORD_STOP					(WM_USER + 753)

#define WM_RECORD_VIDEO					(WM_USER + 755)
#define WM_RECORD_AUDIO					(WM_USER + 756)

//change face�������Ϣ
#define	WM_CHANGE_FACE					(WM_USER + 800)

//tips�Ի������
#define	WM_DELETE_TIPS					(WM_USER + 810)

#define WM_MSG_TALKFUNC					(WM_USER + 820)
#define WM_MSG_EDITLINK					(WM_USER + 830)

#define WM_PTPCONNECT_SUCCESS			(WM_USER + 850)

#define WM_VIDEO_ATTR					(WM_USER + 860)

#define WM_FULLWNDTITLE_BTN				(WM_USER + 870)

#define WM_WND_HIDE						(WM_USER + 880)
#define WM_WND_SHOW						(WM_USER + 881)

//������Ϣ
#define WM_MEETING_DOWN					(WM_USER + 890)
//�û���Ϣ
#define WM_USER_DOWN					(WM_USER + 900)

#define WM_EXSTATIC_DOWN				(WM_USER + 910)

#define WM_SENDRECVVIDEO_FAIL			(WM_USER + 930)
*/
//#define WM_CLOSE_USER					(WM_USER + 950)

//��������������Ӱ��Ļظ�
//#define WM_CONNECT_SERVER				(WM_USER + 960)

//xp�༭������Ϣ
#define WM_XPEDIT						(WM_USER + 970)

//#define WM_MOUSE_DBCLICK				(WM_USER + 980)
//�������û��뿪��������������ʱ������Ϣ�������Ժ��ټӣ�
//#define WM_TALK_USERLEAVE				(WM_USER + 990)

#define WM_USERTREEITEM_SELECT			(WM_USER + 1000)
#define WM_USERTREEITEM_DBCLICK			(WM_USER + 1001)

#define ID_TRAY_EXIT_EX                 32950

#endif
#ifndef _CHAT_VISITOR_H_
#define _CHAT_VISITOR_H_

#include "socket.h"

class CChatManager;

class CChatVisitor : public IBaseReceive
{
public:
	CChatVisitor();
	~CChatVisitor();

public:
	// ���Ϳ�ʼͨ�����ÿͽӴ�������
	int SendWebuserTalkBegin(CWebUserObject *pWebUser);

	// ���ͽ���ͨ�������ÿͽӴ�������
	int SendWebuserTalkEnd(CWebUserObject *pWebUser);

	int SendPingToVisitorServer();

	int SendBuffToVisitorServer(char *sbuff, int len);

	int ConnectToVisitorServer();

	int LoginToVisitorServer();

	void SolveVisitorSystem(char *pInitBuff);

	void SolveVisitorSCRIPTMSG(char *pInitBuff);

	//�ÿͲ�ͬ��Ի�
	void SolveVisitorSCRIPTMSGApplyFail(char *pInitBuff);

	//���ķÿ�����
	void SolveVisitorSCRIPTMSGModiName(char *pInitBuff);

	//��ʼͨ��
	void SolveVisitorSCRIPTMSGTalkBegin(char *pInitBuff);

	//����ͨ��
	void SolveVisitorSCRIPTMSGTalkEnd(char *pInitBuff);

	void SolveVisitorSystemAdmin(char *pInitBuff);

	//�ÿ�����
	void SolveVisitorSystemUp(char *pInitBuff);

	void SetVisitorOffline(CWebUserObject *pWebUser);

	//�ÿ�����
	void SolveVisitorSystemDown(char *pInitBuff);

	//systemset
	void SolveVisitorSystemSystenset(char *pInitBuff);

	//stoprecvmsg
	void SolveVisitorSystemStopRecvMsg(char *pInitBuff);

	//alreadyapply
	void SolveVisitorSystemAlreadyApply(char *pInitBuff);

private:
	/***************     �̳нӿڵĺ���ʵ��    *****************/

	virtual void OnReceive(void* pHead, void* pData);

public:
	CChatManager*	m_manager;
	CMySocket		m_socketEx;				// �ÿͽӴ�����
	time_t			m_tResentVisitPackTime;	//

private:
	friend class CMySocket;

private:
	
};

#endif
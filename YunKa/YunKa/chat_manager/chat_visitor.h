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
	//���Ϳ�ʼͨ�����ÿͽӴ�������
	int SendWebuserTalkBegin(CWebUserObject *pWebUser);

	//���ͽ���ͨ�������ÿͽӴ�������
	int SendWebuserTalkEnd(CWebUserObject *pWebUser);

	int SendPingToVisitorServer();

	int SendBuffToVisitorServer(char *sbuff, int len);

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
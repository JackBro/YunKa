#pragma once

#include "login.h"

class CChatObjects :public IBaseReceive
{
public:
	CChatObjects();
	~CChatObjects();

	friend class CMySocket;

	// ���ո��û��Ի�

	// �ܾ��˶Ի�

	// ���ĸ��û�����
private:
	virtual void OnReceive(void* pHead, void* pData);

public:
	CLogin* m_login;
};


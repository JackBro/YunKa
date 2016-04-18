#pragma once

#include "comm_struct.h"
#include "sys_config.h"
#include "chat_objects.h"
#include "msgs.h"
#include "login.h"

class CChatManager
{
public:
	CChatManager();
	~CChatManager();

	static CChatManager* GetInstance()
	{
		static CChatManager instance;
		return &instance;
	}

public:

	CLogin* GetLogin();

	CSysConfig* GetSysConfig();			// ��ȡϵͳ���ö���

	CChatObjects* GetChatObjects();		// ��ȡ������������ʵ��

	CMsgs* GetMsgs();					// ��ȡ������Ϣ������ʵ��

	void ScreenCapture();				// ��ͼ

private:
	CLogin* m_login;
	CChatObjects* m_chatObjects;
	bool m_isLoginSuccess;
};


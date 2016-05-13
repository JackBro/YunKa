#ifndef _MAIN_FRAME_H_
#define _MAIN_FRAME_H

#pragma once
#include <map>
#include "small_menu.h"

#include "chat_manager.h"
#include "cef_browser/client_handler.h"

#include "rich_edit_util.h"
#include "IImageOle.h"
#include "face_list.h"
#include "face_sel_dlg.h"



// ��HWND��ʾ��CControlUI����
class CWndUI : public CControlUI
{
public:
	CWndUI() : m_hWnd(NULL){}

	virtual void SetVisible(bool bVisible = true)
	{
		__super::SetVisible(bVisible);
		::ShowWindow(m_hWnd, bVisible);
		//SetBkImage(_T("E:\\WeiBo_3\\client\\cppprj\\common\\media\\bin\\debug\\SkinRes\\videobg.bmp"));
	}

	virtual void SetInternVisible(bool bVisible = true)
	{
		__super::SetInternVisible(bVisible);
		::ShowWindow(m_hWnd, bVisible);
	}

	void SetPos(RECT rc)
	{
		__super::SetPos(rc);
		::SetWindowPos(m_hWnd, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	BOOL Attach(HWND hWndNew)
	{
		if (!::IsWindow(hWndNew))
		{
			return FALSE;
		}

		m_hWnd = hWndNew;
		return TRUE;
	}

	HWND Detach()
	{
		HWND hWnd = m_hWnd;
		m_hWnd = NULL;
		return hWnd;
	}

	HWND GetHWND()
	{
		return m_hWnd;
	}

public:
	HWND m_hWnd;
};





class CMainFrame : public WindowImplBase, public IHandlerMsgs
{
public:

	CMainFrame(CChatManager * manager);
	~CMainFrame();

public:

	LPCTSTR GetWindowClassName() const;
	virtual void OnFinalMessage(HWND hWnd);
	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
	virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);




	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam);

	DWORD GetBkColor();
	void SetBkColor(DWORD dwBackColor);


	virtual void OnClick(TNotifyUI& msg);
	virtual void OnSelectChanged(TNotifyUI &msg);
	virtual void OnItemClick(TNotifyUI &msg);
	void OnItemRbClick(TNotifyUI &msg);
	virtual void OnTimer(TNotifyUI &msg);
	virtual void OnHeaderClick(TNotifyUI& msg);
	virtual void OnMouseEnter(TNotifyUI& msg);
	virtual void OnItemSelect(TNotifyUI &msg);

public:    //��������Ϣ�ص�
	// �յ�һ����ϯ�û�����Ϣ,������ʼ����ϯ�б�
	virtual void RecvOneUserInfo(CUserObject* pWebUser){}

	// �յ�һ���½��ĻỰ��Ϣ
	virtual void RecvCreateChat(CWebUserObject* pWebUser) {}

	// �յ�һ���Ự��Ϣ
	virtual void RecvChatInfo(CWebUserObject* pWebUser) {}

	// �յ������û�������״̬
	virtual void RecvUserStatus(CUserObject* pUser) {}

	// ��ϯ������Ϣ
	virtual void RecvOnline(CUserObject* pUser) {}

	// ��ϯ������Ϣ
	virtual void RecvOffline(CUserObject* pUser) {}

	virtual void RecvAcceptChat(CUserObject* pUser, CWebUserObject* pWebUser) {}

	virtual void RecvCloseChat(CWebUserObject* pWebUser) {}

	// ��ȡ��һ�δ�����Ϣ
	virtual string GetLastError() { return ""; }

	virtual void RecvOneMsg(IBaseObject* pObj, int msgFrom, string msgId, int msgType, int msgDataType, string msgContent,
		string msgTime = "", CUserObject* pAssistUser = NULL, WxMsgBase* msgContentWx = NULL, string msgExt = "") {}



public:
	//�Լ�����Ĳ�������
	void OnBtnFont(TNotifyUI& msg);
	void OnBtnFace(TNotifyUI& msg);
	void OnBtnScreen(TNotifyUI& msg);

	void OnFaceCtrlSel(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnBtnSendMessage(TNotifyUI& msg);

	BOOL _RichEdit_InsertFace(CRichEditUI * pRichEdit, LPCTSTR lpszFileName, int nFaceId, int nFaceIndex);


protected:

	void Notify(TNotifyUI& msg);
	void OnPrepare(TNotifyUI& msg);
	void OnExit(TNotifyUI& msg);

public:
	CChatManager* m_manager;


private:

	CSmallMenu m_frameSmallMenu;


	CButtonUI * m_pFontBtn, *m_pFaceBtn, *m_pScreenBtn, *pSendMsgBtn;
	CFaceSelDlg m_faceSelDlg;
	CFaceList  m_faceList;

	//CRichEditUI2    *m_pSendEdit;


	CRichEditUI* m_pSendEdit;


	HandlerInfo m_pListMsgHandler; // ��Ϣ�б�

	CDuiString m_sendMsgString;
	HWND m_hMainWnd;


};















#endif
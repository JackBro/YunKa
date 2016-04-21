// MDuiTest.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "YunKa.h"
#include "login_wnd.h"
#include "main_frame.h"
#include "path.h"
#include "utils.h"



Gdiplus::GdiplusStartupInput g_gdiplusStartupInput;
ULONG_PTR g_gdiplusToken;


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	wstring strFileName = ZYM::CPath::GetAppPath() + _T("ImageOleCtrl.dll");

	BOOL bRet = DllRegisterServer(strFileName.c_str());	// ע��COM���
	if (!bRet)
	{
		::MessageBox(NULL, _T("COM���ע��ʧ�ܣ�Ӧ�ó����޷���ɳ�ʼ��������"), _T("��ʾ"), MB_OK);
		return 0;
	}

	HRESULT hr = ::OleInitialize(NULL);
	if (FAILED(hr))
		return 0;

	GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL);	// ��ʼ��GDI+
	HMODULE hRichEditDll = ::LoadLibrary(_T("Riched20.dll"));	// ����RichEdit�ؼ�DLL

	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("SkinRes"));

	HRESULT Hr = ::CoInitialize(NULL);
	if (FAILED(Hr)) return 0;

#if 1
	CLoginWnd* pLoginFrame = new CLoginWnd();
#else


#endif

	pLoginFrame->Create(NULL, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
	pLoginFrame->CenterWindow();
	int result = pLoginFrame->ShowModal();

	if (result == 1)
	{
		//CMicUpWnd * pMainFram = new CMicUpWnd();
		//if (pMainFram == NULL) { return 0; }
		//pMainFram->Create(NULL, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
		//pMainFram->CenterWindow();
		//pMainFram->ShowWindow(true);
		//pMainFram->ShowModal();




		CMainFrame *pWndFrame = new CMainFrame();
		pWndFrame->Create(NULL, _T(""), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES);
		pWndFrame->CenterWindow();
		pWndFrame->ShowModal();

	}
	else
	{
		//��¼ʧ��
	}


	CPaintManagerUI::MessageLoop();

	::CoUninitialize();


	if (hRichEditDll != NULL)					// ж��RichEdit�ؼ�DLL
		::FreeLibrary(hRichEditDll);

	Gdiplus::GdiplusShutdown(g_gdiplusToken);	// ����ʼ��GDI+
	::OleUninitialize();

	return 0;
}



// DuilibPreview.cpp : 定义应用程序的入口点。
//

#include "StdAfx.h"
#include "DuilibPreview.h"
#include "MainWnd.h"
#define DBGMSG(x) MessageBox(0, x, 0 ,0);


#include <CommDlg.h>
extern "C" _declspec(dllexport) int OpenFileDlg(LPOPENFILENAMEA p)
{
	return GetOpenFileNameA(p);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	int nPathLen = _tcslen(lpCmdLine);
	if (!nPathLen)
	{
		MessageBox(0, _T("DuilibPreview.exe [Xml PathName]"), 0, MB_USERICON);
		return 0;
	}
	if (lpCmdLine[nPathLen - 1]  == '\"')
	{
		lpCmdLine[nPathLen - 1] = 0;
		lpCmdLine++;
	}

	CDuiString strDir(lpCmdLine);

	int nPos = strDir.ReverseFind('\\');
	if (nPos >= 0)
	{
		strDir.Delete(nPos, strDir.GetLength() - nPos);
	}
	SetCurrentDirectory(strDir);

	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(strDir);
	//DBGMSG(_T("xxx"));
	CMainWnd dlg(lpCmdLine);
	dlg.Create(NULL, NULL, UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE| WS_EX_TOPMOST);
	dlg.CenterWindow();
	dlg.ShowModal();
	return 0;
}


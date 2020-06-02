#pragma once
#include <xstring>
#include <list>

typedef struct _ATTR
{
	CDuiString sName;
	CDuiString sKey;
	CDuiString sValue;
}ATTR;
typedef std::list<ATTR>	CAttrList;
// 
// void PrintString(LPCTSTR str, LPCTSTR title)
// {
// 	MessageBox(0, str, title, 0);
// }


void ParsePreAttrs(LPCTSTR p, CAttrList &attrs)
{
	p = SkipSpace(p);
	while (_tcsncmp(p, _T("<!--"), 4) == 0)
	{
		p += 4;
		p = SkipSpace(p);
		if ((_tcsnicmp(p, _T("preattr"), 7) != 0))
		{
			return;
		}
		ATTR attr;
		p += 7;
		p = SkipSpace(p);

		attr.sName.Empty();
		while (*p != ':')
		{
			attr.sName += *p;
			p++;
		}
		//attr.sKey.TrimRight();
		p++;

		p = SkipSpace(p);
		attr.sKey.Empty();
		while (*p != '=')
		{
			attr.sKey += *p;
			p++;
		}
		//attr.sKey.TrimRight();
		p++;

		p = SkipSpace(p);
		attr.sValue.Empty();
		while (_tcsncmp(p, _T("-->"), 3) != 0)
		{
			attr.sValue += *p;
			p++;
		}
		//attr.sValue.TrimRight();
		attrs.push_front(attr);

		p += 3;
		p = SkipSpace(p);
	}
}

CDuiString GetParentFile(LPCTSTR lpszFile, CAttrList& attrs)
{
	CDuiBuffer buf;
	if (!DuiReadFileData(lpszFile, buf))
	{
		return _T("");
	}
	if (buf.GetSize() < 3)
	{
		return _T("");
	}
	LPTSTR pstrXML = NULL;
#ifdef UNICODE
	if (buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF)
	{
		//UTF-8

		DWORD nWide = ::MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)(LPVOID)buf, buf.GetSize(), NULL, 0 );

		pstrXML = static_cast<LPTSTR>(malloc((nWide + 1)*sizeof(TCHAR)));
		::MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)(LPVOID)buf + 3, buf.GetSize() - 3, pstrXML, nWide );
		pstrXML[nWide] = _T('\0');

	}
	else if (buf[0] == 0xFE && buf[1] == 0xFF)
	{
		//utf-16
		pstrXML = static_cast<LPTSTR>((LPVOID)buf) + 1;
	}
	else
	{
		//ascii
		DWORD nWide = ::MultiByteToWideChar( CP_ACP, 0, (LPCSTR)(LPVOID)buf, buf.GetSize(), NULL, 0 );
		pstrXML = static_cast<LPTSTR>(malloc((nWide + 1)*sizeof(TCHAR)));
		::MultiByteToWideChar( CP_ACP, 0, (LPCSTR)(LPVOID)buf, buf.GetSize(), pstrXML, nWide );
		pstrXML[nWide] = _T('\0');
	}
#else
	if (buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF)
	{
		//UTF-8
		std::string s = DuiUtf8ToAcp((LPCSTR)(&buf[3]));
		pstrXML = static_cast<LPTSTR>(malloc(s.length() * sizeof(TCHAR)));
		memcpy(pstrXML, s.c_str(), s.length());
		pstrXML[s.length()] = _T('\0');

	}
	else if (buf[0] == 0xFE && buf[1] == 0xFF)
	{
		//utf-16
		std::string s = DuiUtf16ToAscii((LPCWSTR)(&buf[2]));
		pstrXML = static_cast<LPTSTR>(malloc(s.length() * sizeof(TCHAR)));
		memcpy(pstrXML, s.c_str(), s.length());
		pstrXML[s.length()] = _T('\0');
	}
	else
	{
		//ascii
		pstrXML = static_cast<LPTSTR>((LPVOID)buf);
	}
#endif
	CDuiString sFile(lpszFile);
	LPCTSTR p = SkipSpace(pstrXML);
	if (_tcsncmp(p, _T("<!--"), 4) == 0)
	{
		p += 4;
		p = SkipSpace(p);
		if (_tcsnicmp(p, _T("preview"), 7) == 0)
		{
			p += 7;
			p = SkipSpace(p);
			CDuiString strTemp;
			while (_tcsncmp(p, _T("-->"), 3) != 0)
			{
				strTemp += *p;
				p++;
			}
			p += 3;
			sFile = strTemp;
			ParsePreAttrs(p, attrs);
			sFile = GetParentFile(sFile, attrs);
		}
		else
		{
			ParsePreAttrs(pstrXML, attrs);
		}
	}
	if (pstrXML != buf)
	{
		free(pstrXML);
	}
	return sFile;
}

class CFloatWnd : public CWindowWnd
{
public:
	LPCTSTR GetWindowClassName(void) const
	{
		return _T("FloatWnd");
	}
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		if (uMsg == WM_CREATE)
		{
			return OnCreate(uMsg, wParam, lParam, bHandled); 
		}
		if (uMsg == WM_LBUTTONDOWN)
		{
			if (m_hWndCallback)
			{
				::SendMessage(m_hWndCallback, 0x2222, 0, 0);
				return 0;
			}
		}
		if (uMsg == WM_NCHITTEST)
		{
			return HTCLIENT;
		}
		if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes))
			return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		HWND hParent = GetParent(m_hWnd);
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		DWORD dwNewStyle = WS_OVERLAPPED | WS_VISIBLE| WS_SYSMENU |WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
		dwNewStyle&=styleValue;//按位与将旧样式去掉
		::SetWindowLong(*this, GWL_STYLE, dwNewStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		RECT rcClient;
		::GetClientRect(*this, &rcClient);
		::SetWindowPos(*this, NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, \
			rcClient.bottom - rcClient.top, SWP_FRAMECHANGED|SWP_NOZORDER);
		//::SetWindowPos(*this, NULL, rcClient.left, rcClient.top, 300,100, SWP_FRAMECHANGED);

		m_pm.Init(m_hWnd);
		m_pm.SetInitSize(300, 100);

		m_pLblFloat = new CLabelUI();
		//m_pLblFloat->SetBkColor(0xFF00FF00);
		m_pLblFloat->SetTextColor(0xFFFF0000);
		//m_pLblFloat->SetFloat(true);
		m_pLblFloat->SetAutoCalcWidth(true);
		CContainerUI* pRoot = new CVerticalLayoutUI();
		pRoot->Add(m_pLblFloat);
		pRoot->SetBkColor(/*0xFFFFFFFF*/0xFF000000);
		m_pm.AttachDialog(pRoot);
		return 0;
	}
	void SetText(LPCTSTR lpszText)
	{
		if (_tcslen(lpszText) == 0)
		{
			ShowWindow(false);
			return;
		}
		ShowWindow(true, false);
		RECT rc = {0, 0, 9999, 9999};
		CRenderEngine::DrawText(m_pm.GetPaintDC(), &m_pm, rc, lpszText, 0, _T(""), DT_CALCRECT | DT_SINGLELINE);
		m_pLblFloat->SetText(lpszText);
		SetWindowPos(m_hWnd, HWND_TOPMOST, rc.left, rc.top, rc.right, rc.bottom + 3, SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
		//RedrawWindow(m_hWnd, NULL, NULL, 0);
	}
	void SetPos(int x, int y)
	{
		SetWindowPos(m_hWnd, HWND_NOTOPMOST, x, y, 0, 0, SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);
	}
	CPaintManagerUI m_pm;
	CLabelUI* m_pLblFloat;
	HWND	m_hWndCallback;
};

CControlUI* CALLBACK __FindControlFromPoint(CControlUI* pThis, LPVOID pData)
{
	LPPOINT pPoint = static_cast<LPPOINT>(pData);
	return ::PtInRect(&pThis->GetPos(), *pPoint) ? pThis : NULL;
}
class CMainWnd :
	public WindowImplBase
{
public:
	CMainWnd(LPCTSTR lpszXmlName)
	{
		m_pOldHover = nullptr;
		CDuiString sFilePath(lpszXmlName);
		
		/*
		解析文件预览时需要设置的属性,文件最开始采用如下形式定义预览属性
		<!--preview SettingsView.xml-->
		<!--PreAttr tab_XXX:selectedid=1-->
		<!--PreAttr opt_XXX:selected=true-->
		<!--preattr preview:MouseTips=false-->

		preview：表示定义当前文件被包含的父文件路径，因为如果当前文件是被Include的，则无法直接预览，需要预览父文件
		PreAttr：表示需要预设的控件属性，PreAttr 控件名:属性名=属性值
		PreAttr preview:MouseTips=false表示不启用预览时候的鼠标悬停提示
		详情请阅读InitWindow开始部分代码
		*/
		sFilePath = GetParentFile(sFilePath, m_attrs);
		lpszXmlName = sFilePath;
		TCHAR szDir[MAX_PATH] = {0};
		TCHAR szFile[MAX_PATH] = {0};
		_tcscpy_s(szDir, MAX_PATH, lpszXmlName);
		TCHAR* p = _tcsrchr(szDir, _T('\\'));
		if (p)
		{
			_tcscpy_s(szFile, MAX_PATH, p + 1);
			p[0] = 0;
		}
		else
		{
			szDir[0] = 0;
			_tcscpy_s(szFile, MAX_PATH, lpszXmlName);
		}
		m_strResourcePath = szDir;
		m_strSkinFile = szFile;
		m_bEnableMouseTips = true;
	}
	~CMainWnd()
	{
		for (std::map<CDuiString, CDuiBuffer*>::iterator itor = m_mapFiles.begin(); itor != m_mapFiles.end(); itor++)
		{
			delete itor->second;
		}
	}
	BOOL IsLoadSkinSuccess()
	{
		return m_bLoadSkinSuccess;
	}
protected:
	CAttrList m_attrs;
	CDuiString m_strResourcePath;
	CDuiString m_strSkinFile;
	CFloatWnd m_wndFloat;
	CPaintManagerUI m_pmFloat;
	std::map<CDuiString, CDuiBuffer*> m_mapFiles;
	BOOL m_bLoadSkinSuccess;
	CControlUI* m_pOldHover;
	bool m_bEnableMouseTips;
	virtual void InitWindow()
	{
		if (GetParent(m_hWnd))
		{
			m_PaintManager.SetBackgroundTransparent(false);
		}
		
		for (CAttrList::iterator attr = m_attrs.begin(); attr != m_attrs.end(); attr++)
		{
			if (attr->sName.CompareNoCase(_T("preview")) == 0)
			{
				if (attr->sKey.CompareNoCase(_T("MouseTips")) == 0)
				{
					m_bEnableMouseTips = attr->sValue.CompareNoCase(_T("true")) == 0;
				}
				continue;
			}
			CControlUI* pCtl = GetDlgItem(attr->sName);
			if (pCtl)
			{
				pCtl->SetAttribute(attr->sKey, attr->sValue);
			}
		}
		if (m_bEnableMouseTips)
		{
			m_wndFloat.Create(m_hWnd, NULL, UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE|WS_EX_TOOLWINDOW);
			m_wndFloat.ShowWindow(false, false);
			SetTimer(m_hWnd, 0, 1000, NULL);
			m_wndFloat.m_hWndCallback = m_hWnd;
		}

	}
	virtual LPCTSTR GetWindowClassName() const
	{
		return _T("CDuiPreviewDlg");
	}
	virtual CDuiString GetResourcePath()
	{
		return m_strResourcePath;
	}

	virtual CDuiString GetSkinFile()
	{
		return m_strSkinFile;
	}

	virtual CDuiString GetSkinFolder()
	{
		return m_strResourcePath;
	}
	virtual UILIB_RESOURCETYPE GetResourceType() const
	{
		return UILIB_FILE;
	}
	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam)
	{
		if (wParam == VK_ESCAPE)
		{
			Close();
			return TRUE;
		}
		return WindowImplBase::ResponseDefaultKeyEvent(wParam);
	}
	void OnLoadSkinFinished()
	{
		//修改WindowImplBase代码在加载xml成功后回调该函数
		if (GetParent(m_hWnd))
		{
			CShadowUI* pShadow = m_PaintManager.GetShadow();
			if (pShadow)
			{
				pShadow->ShowShadow(false);
			}
		}
		m_bLoadSkinSuccess = TRUE;
	}

	void OnLoadSkinFailed()
	{
		//修改WindowImplBase代码在加载xml失败后回调该函数
		//此函数在WindowImplBase中弹出加载资源失败对话框处回调
		m_bLoadSkinSuccess = FALSE;
	}

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		static DWORD dwBorderColor = 0;
		static DWORD dwBorderSize = 0;
		static RECT rcBorderSize = {0};
		static CDuiString strForeColor;
		if (uMsg == 0x2222 || (m_pOldHover && WM_LBUTTONDOWN == uMsg && (::GetKeyState(VK_CONTROL) & 0xFF00)))
		{
			//拦截，发送控件选中消息
			HWND hParent = GetParent(m_hWnd);
			if (hParent)
			{
				::SendMessage(hParent, 0x2222, (WPARAM)m_pOldHover->GetXmlFile(), (LPARAM)m_pOldHover->GetXmlOffset());
				return 0;
			}

		}
		LRESULT lRes = WindowImplBase::HandleMessage(uMsg, wParam, lParam);
		if (!m_bEnableMouseTips)
		{
			return lRes;
		}
		if (WM_MOUSEMOVE == uMsg || WM_TIMER == uMsg)
		{
			POINT pt; //= { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			::GetCursorPos(&pt);
			//ClientToScreen(m_hWnd, &pt);
			RECT rc;
			GetWindowRect(m_hWnd, &rc);
			if (!PtInRect(&rc, pt))
			{
				if (m_pOldHover)
				{
					m_pOldHover->SetBorderColor(dwBorderColor);
					if (dwBorderSize)
					{
						m_pOldHover->SetBorderSize(dwBorderSize);
					}
					else
					{
						m_pOldHover->SetBorderSize(rcBorderSize);
					}
					//pOldHover->SetAttribute(_T("ForeColor"), strForeColor);
				}
				m_pOldHover = nullptr;
				m_wndFloat.ShowWindow(false, false);
				return lRes;
			}
		}

		if (WM_NCHITTEST == uMsg && (::GetKeyState(VK_CONTROL) & 0xFF00))
		{
			lRes = HTCLIENT;
		}
		if (WM_NCHITTEST != uMsg && WM_MOVING != uMsg)
		{
			return lRes;
		}
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		static POINT ptLast = {0};
		::ScreenToClient(*this, &pt);
		if (WM_MOVING == uMsg)
		{
			pt = ptLast;
		}
		ptLast = pt;
		CControlUI* pNewHover = m_PaintManager.GetRoot()->FindControl(__FindControlFromPoint, &pt, UIFIND_VISIBLE | UIFIND_TOP_FIRST);
		RECT rcCtrl = {0};
		if (pNewHover)rcCtrl = pNewHover->GetPos();
		if (m_pOldHover != pNewHover && pNewHover)
		{
			CDuiString sTips = pNewHover->GetClass();
			CDuiString sName = pNewHover->GetName();
			if (sName.GetLength())
			{
				sTips += _T(":");
				sTips += sName;
			}
			CDuiString sPos;
			sPos.SmallFormat(_T("(%d,%d,%d,%d)[%d,%d]"),
				rcCtrl.left, rcCtrl.top, rcCtrl.right, rcCtrl.bottom,
				rcCtrl.right - rcCtrl.left, rcCtrl.bottom - rcCtrl.top);
			sTips += sPos;
			CDuiString sFile = pNewHover->GetXmlFile();
			if (sFile.GetLength())
			{
				std::map<CDuiString, CDuiBuffer*>::iterator itor = m_mapFiles.find(sFile);
				CDuiBuffer* pBuf;
				if (itor == m_mapFiles.end())
				{
					pBuf = new CDuiBuffer();
					DuiReadResourceFileData(sFile, *pBuf);
					m_mapFiles[sFile] = pBuf;
				}
				else
				{
					pBuf = itor->second;
				}
				DWORD dwCtrlOff = pNewHover->GetXmlOffset();
				int nLine = 1;
				LPBYTE lpFileData = *pBuf;
				for (DWORD i = 0; i < pBuf->GetSize() && i < dwCtrlOff; i++)
				{
					if (lpFileData[i] == '\n')
					{
						nLine++;
					}
				}
				sFile.SmallFormat(_T(" %s(%d):%d"), (LPCTSTR)sFile, nLine, dwCtrlOff);
			}
			sTips += sFile;
			m_wndFloat.SetText(sTips);
		}
		RECT rcTips;
		GetClientRect(m_wndFloat, &rcTips);
		POINT p = {rcCtrl.left, rcCtrl.top - rcTips.bottom};
		if (rcCtrl.top < 0)
		{
			rcCtrl.top = rcCtrl.bottom;
		}
		ClientToScreen(m_hWnd, &p);
		m_wndFloat.SetPos(p.x, p.y);
		if (m_pOldHover != pNewHover)
		{
			if (m_pOldHover)
			{
				m_pOldHover->SetBorderColor(dwBorderColor);
				if (dwBorderSize)
				{
					m_pOldHover->SetBorderSize(dwBorderSize);
				}
				else
				{
					m_pOldHover->SetBorderSize(rcBorderSize);
				}
				//pOldHover->SetAttribute(_T("ForeColor"), strForeColor);
			}
			if (pNewHover)
			{
				dwBorderColor = pNewHover->GetBorderColor();
				dwBorderSize = pNewHover->GetBorderSize();
				rcBorderSize.left = pNewHover->GetLeftBorderSize();
				rcBorderSize.top = pNewHover->GetTopBorderSize();
				rcBorderSize.right = pNewHover->GetRightBorderSize();
				rcBorderSize.bottom = pNewHover->GetBottomBorderSize();
				//strForeColor = pNewHover->GetAttribute(_T("ForeColor"));
				if (strForeColor.IsEmpty())
				{
					strForeColor = _T("#00000000");
				}

				pNewHover->SetBorderColor(0xFFFF0000);
				pNewHover->SetBorderSize(1);
				//pNewHover->SetAttribute(_T("ForeColor"), _T("#22000000"));
			}
			else
			{
				m_wndFloat.ShowWindow(false, false);
			}
			m_pOldHover = pNewHover;
		}
		return lRes;
	}
};


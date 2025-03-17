
// ImGuiMfcDialog.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CImGuiMfcApp:
// See ImGuiMfcDialog.cpp for the implementation of this class
//

class CImGuiMfcApp : public CWinApp
{
public:
	CImGuiMfcApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	AFX_INLINE int ScaleDpi(int iVal) const { return MulDiv(iVal, m_dpi, 96); }
	AFX_INLINE int GetDpi() const { return m_dpi; }
	__declspec(property(get = GetDpi))	int DPI;

private:
	int	m_dpi;

	DECLARE_MESSAGE_MAP()
};

extern CImGuiMfcApp theApp;

namespace App {
	enum class ThemeStyle { Light, Dark };

	bool IsLightTheme();
	bool SetTitleBarDarkMode(HWND hWnd, bool bDarkMode);
	void SetupImGuiStyle(App::ThemeStyle theme, float fAlpha = 1.0f);
}

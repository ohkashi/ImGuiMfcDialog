// ImGuiDialog.cpp : implementation file
//

#include "pch.h"
#include "ImGuiMfcApp.h"
#include "ImGuiDialog.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"
#include <GL/GL.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Data stored per platform window
struct WGL_WindowData { HDC hDC; };

// Data
static HGLRC			g_hRC;
static WGL_WindowData	g_MainWindow;
static int				g_Width;
static int				g_Height;

bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data);
void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data);
//void ResetDeviceWGL();

CStringA utf8(LPCWSTR u16)
{
	int len = (int)wcslen((const wchar_t*)u16);
	int count = WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)u16, len, NULL, 0, NULL, NULL);
	CHAR szTemp[256] = { '\0' };
	ASSERT(count < sizeof(szTemp));
	WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)u16, len, szTemp, count, NULL, NULL);
	return szTemp;
}


// CImGuiDialog dialog

IMPLEMENT_DYNAMIC(CImGuiDialog, CDialog)

CImGuiDialog::CImGuiDialog(UINT nIDTemplate, CWnd* pParent)
	: CDialog(nIDTemplate, pParent), m_bDarkMode(false), m_bImGuiInited(false), m_bRenderOk(false)
{
	m_bDarkMode = !App::IsLightTheme();
	BackColor = ImColor(IM_COL32_WHITE);
}

CImGuiDialog::~CImGuiDialog()
{
}

void CImGuiDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CImGuiDialog, CDialog)
	ON_WM_DESTROY()
	ON_WM_SETTINGCHANGE()
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()

// CImGuiDialog message handlers

BOOL CImGuiDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Initialize OpenGL
	HWND hWnd = GetSafeHwnd();
	if (!CreateDeviceWGL(hWnd, &g_MainWindow))
	{
		CleanupDeviceWGL(hWnd, &g_MainWindow);
		EndDialog(0);
		return FALSE;
	}
	wglMakeCurrent(g_MainWindow.hDC, g_hRC);

	CRect rect;
	GetClientRect(rect);
	App::SetTitleBarDarkMode(hWnd, m_bDarkMode);

	TCHAR szClassName[100], szText[256];
	int radio_grp_idx = -1;
	CtrlType preType = CtrlType::None;
	HWND hChild = ::GetWindow(hWnd, GW_CHILD);
	while (hChild) {
		::GetClassName(hChild, szClassName, _countof(szClassName));
		ImCtrl ctrl;
		ctrl.handle = hChild;
		ctrl.id = ::GetDlgCtrlID(hChild);
		ctrl.style = GetWindowLong(hChild, GWL_STYLE);
		ctrl.exStyle = GetWindowLong(hChild, GWL_EXSTYLE);
		ctrl.type = GetCtrlType(szClassName, ctrl.style);
		ASSERT(ctrl.type != CtrlType::None);
		switch (ctrl.type) {
		case CtrlType::CheckBox:
			ctrl.state.checked = false;
			break;
		case CtrlType::RadioButton:
			if (ctrl.type != preType) {
				ASSERT(radio_grp_idx < (int)_countof(m_iRadioState) - 1);
				m_iRadioState[++radio_grp_idx] = 0;
			}
			ctrl.state.pIndex = &m_iRadioState[radio_grp_idx];
			break;
		}
		::GetWindowRect(hChild, ctrl.rect);
		ScreenToClient(ctrl.rect);
		::GetWindowText(hChild, szText, _countof(szText));
		strcpy_s(ctrl.text, _countof(ctrl.text), (LPCSTR)utf8(szText));
		m_vtCtrls.emplace_back(ctrl);
		preType = ctrl.type;
		hChild = ::GetWindow(hChild, GW_HWNDNEXT);
	}
	for (auto& item : m_vtCtrls) {
		::DestroyWindow(item.handle);
		m_mapCtrlId[item.id] = &item;
	}

	InitializeImGui(hWnd);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

CImGuiDialog::ImCtrl* CImGuiDialog::GetDlgItem(UINT nID) const
{
	auto it = m_mapCtrlId.find(nID);
	if (it != m_mapCtrlId.end())
		return it->second;
	return nullptr;
}

CImGuiDialog::CtrlType CImGuiDialog::GetCtrlType(LPCTSTR lpszClassName, DWORD dwStyle)
{
	if (_tcscmp(lpszClassName, WC_BUTTON) == 0) {
		DWORD dwType = dwStyle & BS_TYPEMASK;
		if (dwType == BS_CHECKBOX || dwType == BS_AUTOCHECKBOX)
			return CtrlType::CheckBox;
		else if (dwType == BS_RADIOBUTTON || dwType == BS_AUTORADIOBUTTON)
			return CtrlType::RadioButton;
		else if (dwType == BS_GROUPBOX)
			return CtrlType::GroupBox;
		return CtrlType::PushButton;
	} else if (_tcscmp(lpszClassName, WC_EDIT) == 0) {
		return CtrlType::Edit;
	} else if (_tcscmp(lpszClassName, WC_STATIC) == 0) {
		return CtrlType::Static;
	} else if (_tcscmp(lpszClassName, WC_LISTBOX) == 0) {
		return CtrlType::ListBox;
	} else if (_tcscmp(lpszClassName, WC_SCROLLBAR) == 0) {
		return CtrlType::ScrollBar;
	} else if (_tcscmp(lpszClassName, WC_COMBOBOX) == 0) {
		return CtrlType::ComboBox;
	}
	return CtrlType::None;
}

void CImGuiDialog::InitializeImGui(HWND hWnd)
{
	//ImGui_ImplWin32_EnableDpiAwareness();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
	//io.FontGlobalScale = theApp.DPI / 96.0f;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	App::SetupImGuiStyle(m_bDarkMode ? App::ThemeStyle::Dark : App::ThemeStyle::Light);
	ImGuiStyle& style = ImGui::GetStyle();
	BackColor = style.Colors[ImGuiCol_WindowBg];

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_InitForOpenGL(hWnd);
	ImGui_ImplOpenGL3_Init();

	CFont* pFont = GetFont();
	LOGFONT lf;
	pFont->GetLogFont(&lf);
	m_fontHeight = theApp.ScaleDpi(abs(lf.lfHeight));

	LANGID lid = GetUserDefaultUILanguage();
	if (lid == 0x412) {
		m_fontHeight += 2;
		io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Malgun.ttf",
			(float)m_fontHeight, NULL, io.Fonts->GetGlyphRangesKorean());
	} else {
		io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", (float)m_fontHeight);
	}
	m_bImGuiInited = true;
}

bool CImGuiDialog::RenderImpl(HDC hDC)
{
	// Our state
	static bool show_demo_window = true;
	static bool show_another_window = false;

	CRect rect;
	GetClientRect(rect);

	if (!m_bImGuiInited) {
		if (hDC) {
			CDC* pDC = CDC::FromHandle(hDC);
			pDC->FillSolidRect(rect, m_bDarkMode ? RGB(0, 0, 0) : RGB(255, 255, 255));
			CDC::DeleteTempMap();
		}
		return false;
	}

	ImGuiIO& io = ImGui::GetIO();

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	const auto wndSize = ImVec2((float)rect.Width(), (float)rect.Height());
	const char* wnd_name = "Controls";
	ImGui::Begin(wnd_name, nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove);
	ImGui::SetWindowPos(ImVec2(0, 0));
	ImGui::SetWindowSize(wndSize);
	CStringA str;
	int radio_idx = 0;
	
	bool pre_bool;
	CtrlType preType = CtrlType::None;
	const auto& style = ImGui::GetStyle();
	for (auto& item : m_vtCtrls) {
		float x = (float)item.rect.left;
		float y = (float)item.rect.top;
		float cx = (float)item.rect.Width();
		float cy = (float)item.rect.Height();
		ImGui::SetCursorPos(ImVec2(x, y));
		switch (item.type) {
		case CtrlType::CheckBox:
			pre_bool = item.state.checked;
			ImGui::Checkbox(item.text, &item.state.checked);
			if (item.state.checked != pre_bool)
				PostMessage(WM_COMMAND, MAKEWPARAM(item.id, item.state.checked ? BST_CHECKED : BST_UNCHECKED), (LPARAM)0);
			break;
		case CtrlType::RadioButton:
			radio_idx = (item.type != preType) ? 0 : radio_idx;
			if (ImGui::RadioButton(item.text, item.state.pIndex, radio_idx++))
				PostMessage(WM_COMMAND, MAKEWPARAM(item.id, BN_CLICKED), (LPARAM)0);
			break;
		case CtrlType::GroupBox:
		{
			auto g = ImGui::GetWindowDrawList();
			g->AddRect(ImVec2(x, y), ImVec2(x + cx, y + cy),
				ImColor(style.Colors[ImGuiCol_Border]), style.FrameRounding, ImDrawFlags_RoundCornersAll, 1.5f);
			auto txt_size = ImGui::CalcTextSize(item.text);
			x += (float)theApp.ScaleDpi(8);
			y -= m_fontHeight / 2.0f;
			g->AddRectFilled(ImVec2(x, y), ImVec2(x + txt_size.x, y + txt_size.y), ImColor(style.Colors[ImGuiCol_WindowBg]));
			g->AddText(ImVec2(x, y), ImColor(style.Colors[ImGuiCol_Text]), item.text);
		}
		break;
		case CtrlType::PushButton:
			if (ImGui::Button(item.text, ImVec2(cx, cy)))
				PostMessage(WM_COMMAND, MAKEWPARAM(item.id, BN_CLICKED), (LPARAM)0);
			break;
		case CtrlType::Edit:
			str.Format("##%u", item.id);
			ImGui::PushItemWidth(cx);
			ImGui::InputText((const char*)str, item.text, _countof(item.text));
			ImGui::PopItemWidth();
			break;
		case CtrlType::Static:
			ImGui::Text(item.text);
			break;
		case CtrlType::ListBox:
			break;
		case CtrlType::ScrollBar:
			break;
		case CtrlType::ComboBox:
			str.Format("##%u", item.id);
			ImGui::PushItemWidth(cx);
			ImGui::Combo((const char*)str, &item.state.selected, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");
			ImGui::PopItemWidth();
			break;
		}
		preType = item.type;
	}
	ImGui::End();
	OnBeforeImGuiRender(io, wndSize);
	ImGui::Render();

	// Clear the back buffer
	// Rendering
	glViewport(0, 0, g_Width, g_Height);
	glClearColor(BackColor.x, BackColor.y, BackColor.z, BackColor.w);
	glClear(GL_COLOR_BUFFER_BIT);
	auto pDrawData = ImGui::GetDrawData();
	OnRender(pDrawData, wndSize);
	ImGui_ImplOpenGL3_RenderDrawData(pDrawData);

	// Present
	::SwapBuffers(g_MainWindow.hDC);
	if (!m_bRenderOk) {
		m_bRenderOk = true;
		ShowWindow(SW_SHOW);
	}
	return true;
}

void CImGuiDialog::OnDestroy()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceWGL(GetSafeHwnd(), &g_MainWindow);
	wglDeleteContext(g_hRC);

	CDialog::OnDestroy();
}

LRESULT CImGuiDialog::OnKickIdle(WPARAM wParam, LPARAM lParam)
{
	RenderImpl(NULL);
	return TRUE;
}

void CImGuiDialog::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDialog::OnSettingChange(uFlags, lpszSection);

	m_bDarkMode = !App::IsLightTheme();
	App::SetTitleBarDarkMode(GetSafeHwnd(), m_bDarkMode);
}

// Helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
	HDC hDc = ::GetDC(hWnd);
	PIXELFORMATDESCRIPTOR pfd = { 0 };
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;

	const int pf = ::ChoosePixelFormat(hDc, &pfd);
	if (pf == 0)
		return false;
	if (::SetPixelFormat(hDc, pf, &pfd) == FALSE)
		return false;
	::ReleaseDC(hWnd, hDc);

	data->hDC = ::GetDC(hWnd);
	if (!g_hRC)
		g_hRC = wglCreateContext(data->hDC);
	return true;
}

void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
	wglMakeCurrent(nullptr, nullptr);
	::ReleaseDC(hWnd, data->hDC);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CImGuiDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(GetSafeHwnd(), message, wParam, lParam))
		return TRUE;

	switch (message) {
	case WM_ERASEBKGND:
		RenderImpl((HDC)wParam);
		return TRUE;

	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED) {
			g_Width = LOWORD(lParam);
			g_Height = HIWORD(lParam);
		}
		break;

	case WM_WINDOWPOSCHANGING:
		if (!m_bRenderOk) {
			auto pWndPos = reinterpret_cast<WINDOWPOS*>(lParam);
			pWndPos->flags &= ~SWP_SHOWWINDOW;
		}
		break;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

BOOL CImGuiDialog::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message) {
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
		if (ImGui_ImplWin32_WndProcHandler(GetSafeHwnd(), pMsg->message, pMsg->wParam, pMsg->lParam))
			return TRUE;
		break;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

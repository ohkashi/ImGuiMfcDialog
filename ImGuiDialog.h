#pragma once
#include "imgui.h"
#include <vector>
#include <unordered_map>


// CImGuiDialog dialog

class CImGuiDialog : public CDialog
{
	DECLARE_DYNAMIC(CImGuiDialog)

public:
	explicit CImGuiDialog(UINT nIDTemplate, CWnd* pParent = nullptr);
	virtual ~CImGuiDialog();

	enum class CtrlType {
		None, CheckBox, RadioButton, GroupBox, PushButton, Edit, Static, ListBox, ScrollBar, ComboBox };

	struct ImCtrl {
		ImCtrl() { state.selected = -1; }
		CtrlType	type;
		UINT		id;
		HWND		handle;
		DWORD		style;
		DWORD		exStyle;
		CRect		rect;
		char		text[256];
		union {
			bool	checked;
			int		selected;
			int*	pIndex;
		} state;
	};

	ImCtrl* GetDlgItem(UINT nID) const;

	ImVec4	BackColor;

protected:
	CtrlType GetCtrlType(LPCTSTR lpszClassName, DWORD dwStyle);
	void InitializeImGui(HWND hWnd);
	bool RenderImpl(HDC hDC);

	bool	m_bDarkMode;
	bool	m_bImGuiInited;
	int		m_fontHeight;

	virtual void OnBeforeImGuiRender(ImGuiIO& io, const ImVec2& client_size) {}
	virtual void OnRender(ImDrawData* pDrawData, const ImVec2& client_size) {}

private:
	bool	m_bRenderOk;
	int		m_iRadioState[10];
	std::vector<ImCtrl>	m_vtCtrls;
	std::unordered_map<UINT, ImCtrl*>	m_mapCtrlId;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg LRESULT OnKickIdle(WPARAM wParam, LPARAM lParam);
};

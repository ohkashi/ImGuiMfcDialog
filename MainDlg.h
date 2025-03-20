
// MainDlg.h : header file
//

#pragma once
#include <GL/glew.h>
#include <GL/GL.h>


// CMainDlg dialog
class CMainDlg : public CImGuiDialog
{
// Construction
public:
	CMainDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAIN_DLG };
#endif

protected:
	virtual void OnBeforeImGuiRender(ImGuiIO& io, const ImVec2& client_size) override;
	virtual void OnRender(ImDrawData* pDrawData, const ImVec2& client_size) override;
	GLuint AddShader(const char* shaderCode, GLenum shaderType);
	GLuint CompileShader(const char* vsCode, const char* fsCode);

private:
	GLuint	VAO;	// 아래 모든 작업을 하나의 state로 저장할 OpenGL Object ID (vertex array)
	GLuint	VBO;	// vertex buffer object ID (vertex buffer)
	GLuint	shader;
	GLuint	uniformModel;
	float	curAngle = 0.0f;
	const float toRadians = 3.14159265f / 180.0f;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnAppAbout();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedRadio(UINT nID);
};

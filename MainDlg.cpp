
// MainDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "ImGuiMfcApp.h"
#include "MainDlg.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CMainDlg dialog

CMainDlg::CMainDlg(CWnd* pParent /*=nullptr*/) : CImGuiDialog(IDD_MAIN_DLG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CImGuiDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMainDlg, CImGuiDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_QUERYDRAGICON()
	ON_WM_SETTINGCHANGE()
	ON_COMMAND(ID_APP_ABOUT, &CMainDlg::OnAppAbout)
	ON_BN_CLICKED(IDC_CHECK1, &CMainDlg::OnBnClickedCheck1)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO1, IDC_RADIO3, &CMainDlg::OnBnClickedRadio)
END_MESSAGE_MAP()


// CMainDlg message handlers

BOOL CMainDlg::OnInitDialog()
{
	SetMenu(IDR_MAINFRAME);
	CImGuiDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	auto pCheck = GetDlgItem(IDC_CHECK1);
	pCheck->state.checked = !App::IsLightTheme();
	auto pSlider = GetDlgItem(IDC_SLIDER1);
	pSlider->state.progress = 0;

	if (glewInit() != GLEW_OK) {
		ASSERT(FALSE);
		return FALSE;
	}

	float vertices[] = {
		// 위치              // 컬러
		0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // 우측 하단
		-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // 좌측 하단
		0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // 위 
	};

	const char* vertex_shader = R"(
		#version 450

		layout (location = 0) in vec3 pos;
		layout (location = 1) in vec3 aColor;

		out vec3 ourColor;
		uniform mat4 model;

		void main() {
			gl_Position = model * vec4(pos, 1.0);
			ourColor = aColor;
		})";
	const char* fragment_shader = R"(
		#version 450

		in vec3 ourColor;
		out vec4 fragColor;

		void main() {
			fragColor = vec4(ourColor, 1.0);
		})";

	shader = CompileShader(vertex_shader, fragment_shader);

	glGenVertexArrays(1, &VAO);	// VAO 생성
	glBindVertexArray(VAO);		// VAO를 OpenGL context에 연결(bind)

	glGenBuffers(1, &VBO);		// pram:(만들고자 하는 버퍼의 갯수, 버퍼의 ID 포인터)
	glBindBuffer(GL_ARRAY_BUFFER, VBO);	// param:(target, 버퍼의 ID)
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);	// parm:(target, 복사할 데이터 크기, 복사할 데이터, 힌트)

	// 위치 attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// 컬러 attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0); // ID 대신 0을 넣으면 unbind
	glBindVertexArray(0);

	uniformModel = glGetUniformLocation(shader, "model");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMainDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CImGuiDialog::OnSysCommand(nID, lParam);
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMainDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMainDlg::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CImGuiDialog::OnSettingChange(uFlags, lpszSection);

	auto pCtrl = GetDlgItem(IDC_CHECK1);
	pCtrl->state.checked = m_bDarkMode;
	OnBnClickedCheck1();
}

void CMainDlg::OnAppAbout()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

void CMainDlg::OnOK()
{
	TRACE("CMainDlg::OnOK()\n");

	//CImGuiDialog::OnOK();
}

void CMainDlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

	CImGuiDialog::OnCancel();
}

void CMainDlg::OnBnClickedCheck1()
{
	const auto pCtrl = GetDlgItem(IDC_CHECK1);
	m_bDarkMode = pCtrl->state.checked;
	App::SetupImGuiStyle(m_bDarkMode ? App::ThemeStyle::Dark : App::ThemeStyle::Light);
	ImGuiStyle& style = ImGui::GetStyle();
	BackColor = style.Colors[ImGuiCol_WindowBg];
	App::SetTitleBarDarkMode(m_hWnd, m_bDarkMode);
}

void CMainDlg::OnBnClickedRadio(UINT nID)
{
	const auto& pCtrl = GetDlgItem(nID);
	TRACE("Radio%d: state = %d\n", nID - IDC_RADIO1 + 1, *pCtrl->state.pIndex);
}

void CMainDlg::OnBeforeImGuiRender(ImGuiIO& io, const ImVec2& client_size)
{
	static float f = 0.0f;
	static int counter = 0;

	static float progress = 0.0f, progress_dir = 1.0f;
	progress += progress_dir * 0.4f * io.DeltaTime;
	if (progress >= +1.1f) { progress = +1.1f; progress_dir *= -1.0f; }
	if (progress <= -0.1f) { progress = -0.1f; progress_dir *= -1.0f; }

	curAngle += 75.0f * io.DeltaTime;
	if (curAngle >= 360)
		curAngle -= 360;

	/*ImGui::Begin(u8"안녕, 세상아!");
	//ImGui::SetWindowPos(ImVec2(0, 0));
	ImGui::Text("This is some useful text.");

	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
	ImGui::ColorEdit3("clear color", (float*)&BackColor);

	if (ImGui::Button("Button"))
		counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);
	*/
	auto pProgress = GetDlgItem(IDC_PROGRESS1);
	pProgress->state.progress = progress;
	auto pSlider = GetDlgItem(IDC_SLIDER1);
	auto pStatic = GetDlgItem(IDC_STATIC_SLIDER);
	sprintf_s(pStatic->text, _countof(pStatic->text), "%d%%", (int)pSlider->state.progress);
	ImGui::Begin("Footer", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs);
	float cy = (float)m_fontHeight + theApp.ScaleDpi(21);
	ImGui::SetWindowPos(ImVec2((float)theApp.ScaleDpi(4), client_size.y - cy));
	ImGui::SetWindowSize(ImVec2(client_size.x, cy));
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();
}

void CMainDlg::OnRender(ImDrawData* pDrawData, const ImVec2& client_size)
{
	glUseProgram(shader);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, curAngle * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glUseProgram(0);
}

GLuint CMainDlg::AddShader(const char* shaderCode, GLenum shaderType)
{
	GLuint new_shader = glCreateShader(shaderType);

	const GLchar* code[1];
	code[0] = shaderCode;

	glShaderSource(new_shader, 1, code, NULL);

	GLint result = 0;
	GLchar err_log[1024] = { 0 };

	glCompileShader(new_shader);
	glGetShaderiv(new_shader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(new_shader, sizeof(err_log), NULL, err_log);
		TRACE("Error compiling the %d shader: '%s'\n", shaderType, err_log);
		return 0;
	}
	return new_shader;
}

GLuint CMainDlg::CompileShader(const char* vsCode, const char* fsCode)
{
	GLuint vs, fs;

	GLuint shader = glCreateProgram();
	if (!shader) {
		TRACE("Error: Cannot create shader program.");
		return 0;
	}

	vs = AddShader(vsCode, GL_VERTEX_SHADER);
	fs = AddShader(fsCode, GL_FRAGMENT_SHADER);
	glAttachShader(shader, vs);  // Attach shaders to the program for linking process.
	glAttachShader(shader, fs);

	GLint result = 0;
	GLchar err_log[1024] = { 0 };

	glLinkProgram(shader);  // Create executables from shader codes to run on corresponding processors.
	glGetProgramiv(shader, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shader, sizeof(err_log), NULL, err_log);
		TRACE("Error linking program: '%s'\n", err_log);
		return 0;
	}
	return shader;
}

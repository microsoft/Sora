
// DebugPlotViewer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "DebugPlotViewer.h"
#include "MainFrm.h"

#include "DebugPlotViewerDoc.h"
#include "DebugPlotViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDebugPlotViewerApp

BEGIN_MESSAGE_MAP(CDebugPlotViewerApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CDebugPlotViewerApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CDebugPlotViewerApp construction

CDebugPlotViewerApp::CDebugPlotViewerApp()
{
	m_bHiColorIcons = TRUE;

	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("DebugPlotViewer.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CDebugPlotViewerApp object

CDebugPlotViewerApp theApp;
#define SHARED_SINGLE_INSTANCE_MUTEX_NAME L"|Sora|DebugPlot|single_instance_mutex"

// CDebugPlotViewerApp initialization

BOOL CALLBACK BringInstanceToTopCallBack(
  __in  HWND hwnd,
  __in  LPARAM lParam
)
{
	LPDWORD pPid = (LPDWORD)lParam;
	DWORD wndPid;
	::GetWindowThreadProcessId(hwnd, &wndPid);
	if (*pPid == wndPid)
	{
		if (GetWindowLong(hwnd,GWL_STYLE) & WS_VISIBLE)
		{
			::ShowWindow(hwnd, TRUE);
			::SetWindowPos(hwnd, ::GetForegroundWindow(), 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOSIZE);
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CDebugPlotViewerApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.

	SharedGlobalData * sharedGlobalData = SharedGlobalData::Instance();
	if (!sharedGlobalData)
		return FALSE;

	HANDLE hMutex = CreateMutex(NULL, TRUE, SHARED_SINGLE_INSTANCE_MUTEX_NAME);
	if ( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		::CloseHandle(hMutex);
		DWORD pid = sharedGlobalData->sharedStruct->viewPid;
		::EnumWindows(BringInstanceToTopCallBack, (LPARAM)&pid);
		SharedGlobalData::Clean();
		return FALSE;
	}
	else
	{
		DWORD pid = ::GetCurrentProcessId();
		sharedGlobalData->Lock(INFINITE);
		sharedGlobalData->sharedStruct->viewPid = pid;
		sharedGlobalData->Unlock();
	}

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Sora DebugPlotViewer Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	// RichEdit
	AfxInitRichEdit();

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CDebugPlotViewerDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CDebugPlotViewerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}

int CDebugPlotViewerApp::ExitInstance()
{
	// GDI+
	Gdiplus::GdiplusShutdown(m_gdiplusToken);

	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CDebugPlotViewerApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CDebugPlotViewerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CDebugPlotViewerApp customization load/save methods

void CDebugPlotViewerApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_CHANNEL_EXPLORER);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_TEXT_CHANNEL);
}

void CDebugPlotViewerApp::LoadCustomState()
{
}

void CDebugPlotViewerApp::SaveCustomState()
{
}

// CDebugPlotViewerApp message handlers




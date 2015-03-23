
// DbgPlotViewer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "DbgPlotViewer.h"
#include "MainFrm.h"

#include "DbgPlotViewerDoc.h"
#include "DbgPlotViewerView.h"

#include "Event.h"
#include "TaskQueue.h"
#include "SharedNameManagement.h"
#include "CrashDump.h"


using namespace std;
using namespace SoraDbgPlot::SharedObj;
using namespace SoraDbgPlot::Task;
using namespace SoraDbgPlot::Event;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDbgPlotViewerApp

BEGIN_MESSAGE_MAP(CDbgPlotViewerApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CDbgPlotViewerApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CDbgPlotViewerApp construction

CDbgPlotViewerApp::CDbgPlotViewerApp()
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
	SetAppID(_T("DbgPlotViewer.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

#ifdef ENABLE_CRASH_DUMP
	_bEnableCrashDump = true;
#else
	_bEnableCrashDump = false;
#endif

}

// The one and only CDbgPlotViewerApp object

CDbgPlotViewerApp theApp;
#define SHARED_SINGLE_INSTANCE_MUTEX_NAME L"|Sora|DebugPlot|single_instance_mutex"

static BOOL SetPrivilege(
	HANDLE hToken,          // access token handle
	LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
	BOOL bEnablePrivilege   // to enable or disable privilege
	) 
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if ( !LookupPrivilegeValue( 
		NULL,            // lookup privilege on local system
		lpszPrivilege,   // privilege to lookup 
		&luid ) )        // receives LUID of privilege
	{
		char msg[64];
		sprintf(msg, "LookupPrivilegeValue error: %u\n", GetLastError());
		OutputDebugStringA(msg); 
		return FALSE; 
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// Enable the privilege or disable all privileges.

	if ( !AdjustTokenPrivileges(
		hToken, 
		FALSE, 
		&tp, 
		sizeof(TOKEN_PRIVILEGES), 
		(PTOKEN_PRIVILEGES) NULL, 
		(PDWORD) NULL) )
	{ 
		char msg[64];
		sprintf(msg, "AdjustTokenPrivileges error: %u\n", GetLastError());
		OutputDebugStringA(msg); 
		return FALSE; 
	} 

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

	{
		OutputDebugStringA("The token does not have the specified privilege. \n");
		return FALSE;
	} 

	return TRUE;
}

static BOOL SetDebugPrivilege()
{
	HANDLE hToken = NULL;
	BOOL succ = ::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	if (succ == TRUE)
	{
		BOOL succ = SetPrivilege(hToken, SE_DEBUG_NAME, TRUE);
		return succ;
	}
	else
	{
		::OutputDebugStringA("OpenProcessToken failed");
		return FALSE;
	}
}

// CDbgPlotViewerApp initialization

BOOL CDbgPlotViewerApp::InitInstance()
{
	::OutputDebugString(L"DebugPlot Viewer started\n");

	SECURITY_ATTRIBUTES sa;
	CreateDACLWithAllAccess(&sa);

	HANDLE hMutex = CreateMutex(&sa, TRUE, SHARED_SINGLE_INSTANCE_MUTEX_NAME);
	if ( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		::CloseHandle(hMutex);
		::AfxMessageBox(L"Instance already running");
		_bInitialized = false;
		LocalFree(sa.lpSecurityDescriptor);
		return FALSE;
	}
	else
	{
		_bInitialized = true;
		LocalFree(sa.lpSecurityDescriptor);
	}

	if (SetDebugPrivilege() == FALSE)
	{
		::AfxMessageBox(L"Please run DebugPlot Viewer as administrator");
		_bInitialized = false;
		return FALSE;
	}

	SetCrashDumpHandler();

	ClearLog();
	InitAppObj();

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
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
	SetRegistryKey(_T("Sora DebugPlot Viewer v2.0"));
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
	Gdiplus::GdiplusStartup(&_gdiplusToken, &gdiplusStartupInput, NULL);

	CoInitialize(NULL);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CDbgPlotViewerDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CDbgPlotViewerView));
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

int CDbgPlotViewerApp::ExitInstance()
{
	//TODO: handle additional resources you may have added

	if (_bInitialized)
	{
		CoUninitialize();

		// GDI+
		Gdiplus::GdiplusShutdown(_gdiplusToken);

		AfxOleTerm(FALSE);

		DeiniteAppObj();
	}

	return CWinAppEx::ExitInstance();
}

// CDbgPlotViewerApp message handlers


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
void CDbgPlotViewerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CDbgPlotViewerApp customization load/save methods

void CDbgPlotViewerApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	//GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	//GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_CHANNEL_EXPLORER);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_TEXT_CHANNEL);
}

void CDbgPlotViewerApp::LoadCustomState()
{
}

void CDbgPlotViewerApp::SaveCustomState()
{
}

// CDbgPlotViewerApp message handlers


bool CDbgPlotViewerApp::DeleteDirRecursive(const std::wstring& dir, bool bDir, bool bContent)
{
	wstring directoryname = dir;
	if(dir.at(directoryname.size()-1) !=  '\\') directoryname += '\\';

	if (bContent)
	{
		WIN32_FIND_DATAW fdata;
		HANDLE dhandle;
		//BUG 1: Adding a extra \ to the directory name..
		directoryname += L"*";
		dhandle = FindFirstFileW(directoryname.c_str(), &fdata);
		//BUG 2: Not checking for invalid file handle return from FindFirstFileA
		if( dhandle != INVALID_HANDLE_VALUE )
		{
			// Loop through all the files in the main directory and delete files & make a list of directories
			while(true)
			{
				if(FindNextFileW(dhandle, &fdata))
				{
					std::wstring     filename = fdata.cFileName;
					if(filename.compare(L"..") != 0)
					{
						//BUG 3: caused by BUG 1 - Removing too many characters from string.. removing 1 instead of 2
						std::wstring filelocation = directoryname.substr(0, directoryname.size()-1) + filename;

						// If we've encountered a directory then recall this function for that specific folder.

						//BUG 4: not really a bug, but spurious function call - we know its a directory from FindData already, use it.
						if( (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)  
							DeleteFileW(filelocation.c_str());
						else 
							DeleteDirRecursive(filelocation, true, true);
					}
				} else if(GetLastError() == ERROR_NO_MORE_FILES)    break;
			}
			directoryname = directoryname.substr(0, directoryname.size()-2);
			//BUG 5: Not closing the FileFind with FindClose - OS keeps handles to directory open.  MAIN BUG
			FindClose( dhandle );
		}
	}
	if (bDir)
	{
		HANDLE DirectoryHandle;
		DirectoryHandle = CreateFileW(directoryname.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL);
		//BUG 6: Not checking CreateFileA for invalid handle return.
		if( DirectoryHandle != INVALID_HANDLE_VALUE )
		{

			bool DeletionResult = (RemoveDirectoryW(directoryname.c_str()) != 0)?true:false;
			CloseHandle(DirectoryHandle);
			return DeletionResult;
		}
		else
		{
			return true;
		}
	}

	return true;
}

void CDbgPlotViewerApp::SetCrashDumpHandler()
{
	if (_bEnableCrashDump)
		SetCrashDump();
}

void CDbgPlotViewerApp::ClearLog()
{
	DWORD requiredLen = ::GetCurrentDirectory(0, NULL);
	if (requiredLen == 0)
		return;

	const wchar_t * dir = L"\\logs";

	requiredLen += (wcslen(dir) + 1);

	wchar_t * path = new wchar_t[requiredLen];
	DWORD ret = ::GetCurrentDirectory(requiredLen, path);

	if (ret == 0)
	{
		delete [] path;
		return;
	}

	wcscat(path, dir);
	DeleteDirRecursive(path, true, true);

	if (path)
		delete [] path;
}

void CDbgPlotViewerApp::InitAppObj()
{
	_serialGen = make_shared<SoraDbgPlot::Common::SharedSerialNumGenerator>(
		SharedNameManager::GetSerialNumGeneratorName()
		);
	_taskQueue = make_shared<TaskQueue>();
	channelManager = make_shared<SharedChannelManager>();
	objLookup = make_shared<ObjLookup>();
	plotWindowOpenedContainer = make_shared<PlotWindowOpenedContainer>();
	plotOperationDriver = make_shared<PlotOperationDriver>();
	controlPanelDriver = make_shared<ControlPanelDriver>();
	playPauseController = make_shared<PlotWndPlayPauseController>();

	RouteMessage_SharedObj();
	RouteMessage_TreeUpdate();
	RouteMessage_OpenOperation();
	RouteMessage_SaveLoad();
	RouteMessage_PlayPause();
}

void CDbgPlotViewerApp::DeiniteAppObj()
{

	objLookup->Clear();
	channelManager->Clear();
	plotWindowOpenedContainer->Clear();

	channelManager.reset();
	objLookup.reset();
	plotWindowOpenedContainer.reset();
	plotOperationDriver.reset();
	controlPanelDriver.reset();
	playPauseController.reset();
	currentObject.reset();
	_taskQueue.reset();

	while(1)
	{
		int numQueue = TaskQueue::WaitAndClean(1000);

#ifdef _DEBUG
		//int numHandler = SoraDbgPlot::Event::EventHandlerCount();
		//TRACE0("Waiting for task queue tasks\n");
		//TRACE1("Task queue count: %d\n", numQueue);
		//TRACE1("Event handler unreleased: %d\n", numHandler);
		//TRACE1("Task num in queue: %d\n", SoraDbgPlot::Task::TaskQueue::MonitorTaskInQueue());
		//TRACE1("Task num to run: %d\n", SoraDbgPlot::Task::TaskQueue::MonitorTaskCntToRun());
		//TRACE1("Task num runnning: %d\n", SoraDbgPlot::Task::TaskQueue::MonitorTaskCntRunning());
		//TRACE1("Task queue op num: %d\n", SoraDbgPlot::Task::TaskQueue::MonitorTaskQueueOpCnt());
#endif		

		if (numQueue == 0)
			break;
	}
	_serialGen.reset();

	ClearLog();

	::OutputDebugString(L"DebugPlot Viewer exited\n");
}

void CDbgPlotViewerApp::RouteMessage_SaveLoad()
{
	auto lu = objLookup;
	auto pl = plotWindowOpenedContainer;
	auto pod = theApp.plotOperationDriver;
	auto playPauseController = theApp.playPauseController;

	lu->EventPlotWindowLoaded.Subscribe([pod](const void * sender, const ObjLookup::PlotWindowLoadedEvent & e){
		pod->AddPlotWnd(e._plotWnd);
	});

	lu->EventPlotWindowLoaded.Subscribe([playPauseController](const void * sender, const ObjLookup::PlotWindowLoadedEvent & e){
		playPauseController->AddProcessPlotWnd(e._process, e._plotWnd);
	});

	lu->EventPlotWindowLoaded.Subscribe([pl](const void * sender, const ObjLookup::PlotWindowLoadedEvent & e){
		pl->AddPlotWindow(e._plotWnd, e._rect);
	});

	lu->EventChannelLoaded.Subscribe([pl](const void * sender, const ObjLookup::ChannelLoadedEvent & e){
		pl->AddChannel(e._plotWnd, e._channel, e._rect);
	});
}

void CDbgPlotViewerApp::RouteMessage_OpenOperation()
{
	auto lu = objLookup;
	auto cm = channelManager;
	auto pl = plotWindowOpenedContainer;
	auto pod = theApp.plotOperationDriver;
	auto playPauseController = theApp.playPauseController;

	pl->EventCloseChannel.Subscribe([lu](const void * sender, const PlotWindowOpenedContainer::CloseChannelEvent & e){
		lu->CloseChannel(e._channel);
	});

	pl->EventAddPlotWndRequest.Subscribe([lu, pl](const void * sender, const PlotWindowOpenedContainer::AddPlotWndParam & e){
		auto succ = make_shared<bool>();
		auto process = e._process;
		auto channel = e._channel;
		auto point = e._point;

		auto pl2 = pl;
		lu->OpenChannel(channel, [pl2, process, channel, point](bool succ){
			if (succ)
				pl2->AddPlotWindow(process, channel, point);
		});
	});

	pl->EventAddChannelRequest.Subscribe([lu, pl](const void * sender, const PlotWindowOpenedContainer::AddChannelRequestEvent & e){
		auto succ = make_shared<bool>();
		auto channel = e._channel;
		auto plotWnd = e._plotWnd;
		auto rect = e._rect;
		lu->CloseChannel(channel);

		auto pl2 = pl;
		lu->OpenChannel(channel, [pl2, plotWnd, channel, rect](bool succ){
			pl2->AddChannel(plotWnd, channel, rect);
		});
	});

	pl->EventClosePlotWnd.Subscribe([pod](const void * sender, const PlotWindowOpenedContainer::ClosePlotWndEvent & e){
		pod->RemovePlotWnd(e._plotWnd);
	});

	pl->EventClosePlotWnd.Subscribe([lu](const void * sender, const PlotWindowOpenedContainer::ClosePlotWndEvent & e){
		lu->RemovePlotWnd(e._plotWnd);
	});

	pl->EventClosePlotWnd.Subscribe([playPauseController](const void * sender, const PlotWindowOpenedContainer::ClosePlotWndEvent & e){
		playPauseController->RemovePlotWnd(e._plotWnd);
	});

	pl->EventPlotWndAdded.Subscribe([pod](const void * sender, const PlotWindowOpenedContainer::PlotWndAddedParam & e){
		pod->AddPlotWnd(e._plotWnd);
	});

	pl->EventPlotWndAdded.Subscribe([playPauseController](const void * sender, const PlotWindowOpenedContainer::PlotWndAddedParam & e){
		playPauseController->AddProcessPlotWnd(e._process, e._plotWnd);
	});

	pl->EventPlotWndAdded.Subscribe([lu](const void * sender, const PlotWindowOpenedContainer::PlotWndAddedParam & e){
		lu->AddPlotWnd(e._plotWnd, e._process);
	});
}

void CDbgPlotViewerApp::RouteMessage_TreeUpdate()
{
	//auto lu = objLookup;

	//lu->EventProcessChannelTreeChanged.Subscribe([](const void * sender, const ObjLookup::ProcessChannelTree & tree){
	//	TRACE0("Ps-Ch tree changed\n");
	//	for (auto iterPsChMap = tree.begin(); iterPsChMap != tree.end(); ++iterPsChMap)
	//	{
	//		auto process = iterPsChMap->first;
	//		TRACE2("process [%d] [%s]\n", process->Pid(), process->Name().c_str());
	//		auto channelMap = iterPsChMap->second;
	//		for (auto iterChannel = channelMap.begin(); iterChannel != channelMap.end(); ++iterChannel)
	//		{
	//			TRACE2("channel [%d] [%s]\n", (*iterChannel)->Pid(), (*iterChannel)->Name().c_str());
	//		}
	//	}
	//});
}

void CDbgPlotViewerApp::RouteMessage_SharedObj()
{
	auto lu = objLookup;
	auto cm = channelManager;
	auto pl = plotWindowOpenedContainer;

	cm->EventDiscoverdProcess.Subscribe([lu](const void * sender, const vector<shared_ptr<SharedProcess> > & vec){
		TRACE0("New shared process discoved\n");
		lu->AddSharedProcess(vec);
	});

	cm->EventProcessClosed.Subscribe([lu](const void * sender, const vector<shared_ptr<SharedProcess> > & vec){
		TRACE0("Shared Process closed\n");
		lu->RemoveSharedProcess(vec);
	});

	cm->EventDiscoverdChannel.Subscribe([lu](const void * sender, const vector<shared_ptr<SharedChannel> > & vec){
		TRACE0("New shared channel discovered\n");
		lu->AddSharedChannel(vec);
	});

	cm->EventChannelClosed.Subscribe([lu](const void * sender, const vector<shared_ptr<SharedChannel> > & vec){
		TRACE0("Shared channel closed\n");
		lu->RemoveSharedChannel(vec);
	});

	// control panel related

	auto controlPanelDriver = theApp.controlPanelDriver;

	lu->EventProcessOpenedAttatched.Subscribe([controlPanelDriver](const void * sender, const std::shared_ptr<ProcessOpened> & process){
		controlPanelDriver->AddProcessOpened(process);
	});

	lu->EventProcessOpenedDeattatched.Subscribe([controlPanelDriver](const void * sender, const std::shared_ptr<ProcessOpened> & process){
		controlPanelDriver->RemoveProcess(process);
	});

	lu->EventProcessOpenedAttatched.Subscribe([pl](const void * sender, const std::shared_ptr<ProcessOpened> & process){
		pl->PrcessAttatchDetatch(process, true);
	});

	lu->EventProcessOpenedDeattatched.Subscribe([pl](const void * sender, const std::shared_ptr<ProcessOpened> & process){
		pl->PrcessAttatchDetatch(process, false);
	});
}

void CDbgPlotViewerApp::RouteMessage_PlayPause()
{
	auto theObjLookup = this->objLookup;
	auto thePlayPauseController = this->playPauseController;

	theObjLookup->EventPauseProcess.Subscribe([thePlayPauseController](const void * sender, const std::shared_ptr<ProcessOpened> & process){
		thePlayPauseController->PauseProcess(process);
	});
}

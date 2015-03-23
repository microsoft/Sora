
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include <algorithm>
#include "DebugPlotViewer.h"

#include "MainFrm.h"
#include "Logger.h"
#include "Debug.h"
#include "SharedObjManager.h"
#include "Constant.h"
#include "ChannelBufferImpl.h"
#include "SeriesLine.h"
#include "SharedSerialNumGenerator.h"
#include "SharedNameManagement.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_WM_SETTINGCHANGE()
	ON_MESSAGE(WM_APP, OnApp)
	ON_WM_TIMER()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	_serialGen = new SoraDbgPlot::Common::SharedSerialNumGenerator(
		SharedNameManager::GetSerialNumGeneratorName()
		);

	// TODO: add member initialization code here
	hReadingThread = 0;
	stopFlag = 0;
	timeIntervalMs = 15;
	hWaitableTimer = ::CreateWaitableTimer(NULL, FALSE, NULL);

	_channelBufferReadable = ChannelBuffer::OpenForRead(
		SoraDbgPlot::Common::GLOBAL_BUFFER_NAME,
		SoraDbgPlot::Common::GLOBAL_BUFFER_BLOCK_NUM,
		SoraDbgPlot::Common::GLOBAL_BUFFER_BLOCK_SIZE);
}

CMainFrame::~CMainFrame()
{
	KillReadingThread();
	CloseHandle(hWaitableTimer);
	SharedObjManager::Clean();
	SharedGlobalData::Clean();
	ASSERT(this->registeredWnd.size() == 0);
	delete _channelBufferReadable;
	delete _serialGen;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;
	// set the visual manager and style based on persisted value
	//OnApplicationLook(theApp.m_nAppLook);
	CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	// Allow user-defined toolbars operations:
	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

#ifdef ENABLE_STATUSBAR
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
#endif

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);


	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Load menu item image (not placed on any standard toolbars):
	CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);

	// create docking windows
	if (!CreateDockingWindows())
	{
		TRACE0("Failed to create docking windows\n");
		return -1;
	}

	CRect rectInitialDock;
	m_wndPlayControl.EnableDocking(CBRS_ALIGN_ANY);
	rectInitialDock.SetRect(0, 0, 80, 80);
	DockPane(&m_wndPlayControl, AFX_IDW_DOCKBAR_BOTTOM, &rectInitialDock);

	m_wndChannelExplorer.EnableDocking(CBRS_ALIGN_ANY);
	rectInitialDock.SetRect(0, 0, 200, 200);
	DockPane(&m_wndChannelExplorer, AFX_IDW_DOCKBAR_LEFT, &rectInitialDock);

	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	rectInitialDock.SetRect(0, 0, 300, 300);
	m_wndProperties.DockToWindow(&m_wndChannelExplorer, CBRS_ALIGN_BOTTOM, &rectInitialDock);

	m_wndAutoLayout.EnableDocking(CBRS_ALIGN_ANY);
	rectInitialDock.SetRect(0, 0, 50, 50);
	DockPane(&m_wndAutoLayout, AFX_IDW_DOCKBAR_BOTTOM, &rectInitialDock);

#ifdef ENABLE_OUTPUT_PANEL
	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndOutput);
#endif

	// Enable toolbar and docking window menu replacement
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// enable quick (Alt+drag) toolbar customization
	CMFCToolBar::EnableQuickCustomization();

	if (CMFCToolBar::GetUserImages() == NULL)
	{
		// load user-defined toolbar images
		if (m_UserImages.Load(_T(".\\UserImages.bmp")))
		{
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}

	// enable menu personalization (most-recently used commands)
	// TODO: define your own basic commands, ensuring that each pulldown menu has at least one basic command.
	CList<UINT, UINT> lstBasicCommands;

	lstBasicCommands.AddTail(ID_FILE_NEW);
	lstBasicCommands.AddTail(ID_FILE_OPEN);
	lstBasicCommands.AddTail(ID_FILE_SAVE);
	lstBasicCommands.AddTail(ID_FILE_PRINT);
	lstBasicCommands.AddTail(ID_APP_EXIT);
	lstBasicCommands.AddTail(ID_EDIT_CUT);
	lstBasicCommands.AddTail(ID_EDIT_PASTE);
	lstBasicCommands.AddTail(ID_EDIT_UNDO);
	lstBasicCommands.AddTail(ID_APP_ABOUT);
	lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
	lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2003);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_VS_2005);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLUE);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_SILVER);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLACK);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_AQUA);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_WINDOWS_7);
	lstBasicCommands.AddTail(ID_SORTING_SORTALPHABETIC);
	lstBasicCommands.AddTail(ID_SORTING_SORTBYTYPE);
	lstBasicCommands.AddTail(ID_SORTING_SORTBYACCESS);
	lstBasicCommands.AddTail(ID_SORTING_GROUPBYTYPE);

	CMFCToolBar::SetBasicCommands(lstBasicCommands);

	//HANDLE curProcess = ::GetCurrentProcess();
	//::SetProcessAffinityMask(curProcess, 0x1);	// run in core 0

    FILETIME ft;
    SYSTEMTIME st;
	LARGE_INTEGER dueTime;

    GetSystemTime(&st);              // Gets the current system time
    SystemTimeToFileTime(&st, &ft); 
	
	dueTime.LowPart = ft.dwLowDateTime;
	dueTime.HighPart = ft.dwHighDateTime;

	::SetWaitableTimer(hWaitableTimer, &dueTime, 15, NULL, NULL, FALSE);
	StartReadingThread();

	enableTimerFlag = true;
	SetTimer(TIMER_CONTROLLER, 100, 0);
	SetTimer(TIMER_UPDATE_TREE, 1000, 0);
	SetTimer(TIMER_UPDATE_GRAPH, 30, 0);

	closeFlag = false;

	return 0;
}

void CMainFrame::StartReadingThread()
{
	if (!hReadingThread)
	{
		stopFlag = 0;
		hReadingThread = (HANDLE)_beginthread(ThreadReadData, 0, this);
	}
}

void CMainFrame::KillReadingThread()
{
	if (hReadingThread)
	{
		stopFlag = 1;
		::WaitForSingleObject(hReadingThread, INFINITE);
		hReadingThread = 0;
	}
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

BOOL CMainFrame::CreateDockingWindows()
{
	BOOL bNameValid;

	// Create output window

#ifdef ENABLE_OUTPUT_PANEL
	CString strOutputWnd;
	bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
	ASSERT(bNameValid);
	if (!m_wndOutput.Create(strOutputWnd, this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Output window\n");
		return FALSE; // failed to create
	}

	Logger::wnd = &m_wndOutput;
#endif

	// Create channel view
	CString strChannelView;
	bNameValid = strChannelView.LoadString(IDS_CHANNEL_VIEW);

	if (!m_wndChannelExplorer.Create(strChannelView, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_CHANNEL_VIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT| CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create File View window\n");
		return FALSE; // failed to create
	}

	// Create play control view
	CString strPlayControlView;
	bNameValid = strPlayControlView.LoadString(IDS_PLAY_CONTROL_VIEW);

	if (!m_wndPlayControl.Create(strPlayControlView, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_PLAY_CONTROL_VIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create File View window\n");
		return FALSE; // failed to create
	}

	// Create properties window
	CString strPropertiesWnd;
	bNameValid = strPropertiesWnd.LoadString(IDS_PROPERTIES_WND);
	ASSERT(bNameValid);
	if (!m_wndProperties.Create(strPropertiesWnd, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Properties window\n");
		return FALSE; // failed to create
	}
	
	// Create properties window
	CString strAutoLayoutWnd;
	bNameValid = strAutoLayoutWnd.LoadString(IDS_AUTOLAYOUT_WND);
	ASSERT(bNameValid);
	if (!m_wndAutoLayout.Create(strAutoLayoutWnd, this, CRect(0, 0, 200, 200), TRUE, ID_AUTOLAYOUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_TOP | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Auto Layout Panel\n");
		return FALSE; // failed to create
	}

	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hChannelViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_FILE_VIEW_HC : IDI_FILE_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndChannelExplorer.SetIcon(hChannelViewIcon, FALSE);

	HICON hPlayControlIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndPlayControl.SetIcon(hPlayControlIcon, FALSE);

	HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndProperties.SetIcon(hPropertiesBarIcon, FALSE);

	HICON hAutoScaleIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_AUTOLAYOUT), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndAutoLayout.SetIcon(hAutoScaleIcon, FALSE);


#ifdef ENABLE_OUTPUT_PANEL
	HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndOutput.SetIcon(hOutputBarIcon, FALSE);
#endif
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// base class does the real work

	if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}


	// enable customization button for all user toolbars
	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != NULL)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}

	return TRUE;
}

void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CFrameWndEx::OnSettingChange(uFlags, lpszSection);
#ifdef ENABLE_OUTPUT_PANEL
	m_wndOutput.UpdateFonts();
#endif
}

void __cdecl ThreadReadData(PVOID param)
{
	CMainFrame * frame = (CMainFrame *)param;
	SharedObjManager * sharedObjManager = SharedObjManager::Instance();

	do {
		std::vector<SeriesProp *> copyOfSeries;
		std::vector<DebuggingProcessProp *> copyOfProcess;
		while(1)
		{
			if (frame->stopFlag)
				break;

			WaitForSingleObject(frame->hWaitableTimer, INFINITE);

			// lock the tree and to the fast work
			sharedObjManager->Lock();
			// copy process out
			std::for_each(sharedObjManager->allProcessObjs.begin(), sharedObjManager->allProcessObjs.end(), [&copyOfProcess] (std::pair<wstring, DebuggingProcessProp *> pair)
			{
				DebuggingProcessProp * processProp = pair.second;
				processProp->IncRefCount();
				copyOfProcess.push_back(processProp);
			});

			// copy series out
			std::for_each(sharedObjManager->allSeriesObjs.begin(), sharedObjManager->allSeriesObjs.end(), [&copyOfSeries] (std::pair<wstring, SeriesProp *> pair) {
				SeriesProp * seriesProp = pair.second;
				seriesProp->IncRefCount();
				copyOfSeries.push_back(seriesProp);
			});
			sharedObjManager->Unlock();

			// update r/w index for series replay buffer
			std::for_each(copyOfProcess.begin(), copyOfProcess.end(), [] (DebuggingProcessProp * process)
			{
				process->PlayAFrame();
			});

			// new way of update channel data
			frame->_channelBufferReadable->ReadData(CMainFrame::ReadDataFunction, frame);

			// maybe slow work: update actual data for each series
			std::for_each(copyOfSeries.begin(), copyOfSeries.end(), [&frame] (SeriesProp * seriesProp) {
				if (seriesProp->isActive)
				{
					ASSERT(seriesProp->GetRefCount());

					//seriesProp->UpdateView();
				}
				seriesProp->DecRefCount();
			});
			copyOfSeries.resize(0);

			// do:	1, set event for plotter-viewer syncronization
			//		2, decrement reference count
			std::for_each(copyOfProcess.begin(), copyOfProcess.end(), [] (DebuggingProcessProp * process)
			{
				if (process->ShouldUpdateReplayBuffer())
					process->SetEventForPlotter();
				process->DecRefCount();
			});
			copyOfProcess.resize(0);
		}

	} while(0);

	_endthread();
}

void __stdcall CMainFrame::ReadDataFunction(const char * buffer, int size, void * userData, __int32 userId)
{
	SharedObjManager * sharedObjManager = SharedObjManager::Instance();

	auto iter = sharedObjManager->seriesMapForReading.find(userId);
	if (iter != sharedObjManager->seriesMapForReading.end())
	{
		iter->second->WriteData(buffer, size);
	}
}

LRESULT CMainFrame::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case CMD_PLAY_PAUSE_ALL:
		this->GetActiveView()->PostMessage(WM_APP, wParam, lParam);
		break;

	case CMD_NEW_DOCUMENT:		// multicast this message to view, property panel, control panel
		this->m_wndProperties.PostMessage(WM_APP, wParam, lParam);
		this->GetActiveView()->PostMessage(WM_APP, wParam, lParam);
		this->m_wndChannelExplorer.PostMessage(WM_APP, wParam, lParam);
		break;
	case CMD_CLEAR_DOCUMENT:
		this->m_wndPlayControl.SendMessage(WM_APP, wParam, lParam);
		break;
	case CMD_CHANGE_PROPERTY:
		this->m_wndProperties.PostMessage(WM_APP, wParam, lParam);
		break;
	case CMD_CHANGE_PROPERTY_VALUE:
		this->m_wndProperties.PostMessage(WM_APP, wParam, lParam);
		break;
	case CMD_UPDATE_CONTROLLER:
		this->m_wndPlayControl.PostMessage(WM_APP, wParam, lParam);
		break;
	case CMD_UPDATE_PROPERTY_PANEL:
		this->m_wndProperties.PostMessage(WM_APP, wParam, lParam);
		break;
	case CMD_UPDATE_CHANNEL_TREE:
		this->m_wndChannelExplorer.PostMessage(WM_APP, wParam, lParam);
		break;
	case CMD_GET_PLOT_WND_AREA_WND:
		this->GetActiveView()->SendMessage(WM_APP, wParam, lParam);
		break;
	case CMD_MAIN_FRAME_ENABLE_TIMER:
		do {
			bool enable = (lParam != 0);
			this->enableTimerFlag = enable;
			if (enable)
				StartReadingThread();
			else
				KillReadingThread();
		} while(0);
		break;
	case CMD_NOTIFY_RELEASED:
		{
			ObjProp * prop = (ObjProp *)lParam;
			if (this->m_wndProperties.GetCurrentObj() == prop)
			{
				::AfxGetMainWnd()->PostMessage(WM_APP, CMD_CHANGE_PROPERTY, (LPARAM)0);		
			}
		}
		break;
	case CMD_UPDATE_BITMAP:
		{
			MsgUpdateBmp * msg = (MsgUpdateBmp *)lParam;
			if (registeredWnd.find(msg->wnd) != registeredWnd.end())
			{
				msg->wnd->PostMessage(WM_APP, wParam, lParam);
			}
			else
			{
				delete msg->bmp;
				delete msg;
			}
		}
		break;
	case CMD_REGISTER_WND:
		this->registeredWnd.insert((CWnd*)lParam);
		break;
	case CMD_UNREGISTER_WND:
		{
			CWnd * wnd = (CWnd *)lParam;
			set<CWnd *>::iterator iter;
			iter = this->registeredWnd.find(wnd);
			if (iter != this->registeredWnd.end())
				this->registeredWnd.erase(iter);
		}
		break;
	case CMD_UPDATE_PLOT_WND:
		this->GetActiveView()->PostMessage(WM_APP, wParam, lParam);
		break;
	case CMD_AUTO_LAYOUT:
		this->GetActiveView()->PostMessage(WM_APP, wParam, lParam);
		break;
	}
	return 0;
}



void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default


	if (enableTimerFlag)
	{
		switch (nIDEvent)
		{
		case TIMER_CONTROLLER:
			this->m_wndPlayControl.PostMessage(WM_APP, CMD_CONTROLLER_TIMER, 0);
			break;
		case TIMER_UPDATE_TREE:
			this->m_wndChannelExplorer.SendMessage(WM_APP, CMD_UPDATE_TREE_TIMER, 0);
			break;
		case TIMER_UPDATE_GRAPH:
			this->GetActiveView()->SendMessage(WM_APP, CMD_UPDATE_GRAPH_TIMER, 0);
			//this->plotWndArea.plotWndAreaProp->PlotGraphToScreen();
			break;
		}
	}

	CFrameWndEx::OnTimer(nIDEvent);
}

void CMainFrame::OnClose()
{
	if (!closeFlag)		// do some clean up job
	{
		closeFlag = true;
		this->GetActiveView()->SendMessage(WM_APP, CMD_RESET_VIEW, 0);
		this->PostMessage(WM_CLOSE, 0, 0);
	}
	else
	{
		CFrameWndEx::OnClose();
	}
}

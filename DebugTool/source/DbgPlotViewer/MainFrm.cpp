
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "DbgPlotViewer.h"

#include "MainFrm.h"

#include <memory>
#include "SharedChannelManager.h"
#include "GlobalObjLookup.h"
#include "ChannelTreeCtrl.h"
#include "AutoLayoutSlider.h"

using namespace std;

#define WMME_EXE_TASKQUEUE (WM_APP+1)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WMME_MAINFRAME_REAL_CLOSE WM_APP + 32

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_WM_SETTINGCHANGE()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_MESSAGE(WMME_MAINFRAME_REAL_CLOSE, OnRealClose)
	ON_COMMAND(ID_PLAY_PAUSE_ALL, &CMainFrame::OnPlayPauseAll)
	ON_MESSAGE(WMME_EXE_TASKQUEUE, OnExeTaskQueue)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

shared_ptr<SoraDbgPlot::SharedObj::SharedChannelManager> g_sharedManager;
shared_ptr<ObjLookup> g_objLookup;

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	_playPauseState = STATE_PLAY;
	_bTimeEnabled = false;
}

CMainFrame::~CMainFrame()
{
	_pvTaskQueue.Execute(true);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	// set the visual manager used to draw all user interface elements
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));

	// set the visual style to be used the by the visual manager
	CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);

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

	if (!_playControlToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,CRect(1,1,1,1), IDR_MAINFRAME) ||
		!_playControlToolBar.LoadToolBar(IDR_TOOLBAR_PLAY_CONTROL))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strPlayToolBarName;
	bNameValid = strPlayToolBarName.LoadString(IDS_TOOLBAR_PLAY_CONTROL);
	ASSERT(bNameValid);
	_playControlToolBar.SetWindowText(strPlayToolBarName);

	// Allow user-defined toolbars operations:
	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	_playControlToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);
	DockPane(&_playControlToolBar);

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

	DockDockpanels();

	// Enable toolbar and docking window menu replacement

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

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
	lstBasicCommands.AddTail(ID_SORTING_SORTALPHABETIC);
	lstBasicCommands.AddTail(ID_SORTING_SORTBYTYPE);
	lstBasicCommands.AddTail(ID_SORTING_SORTBYACCESS);
	lstBasicCommands.AddTail(ID_SORTING_GROUPBYTYPE);

	CMFCToolBar::SetBasicCommands(lstBasicCommands);

	m_wndToolBar.ResetAllImages();
	_playControlToolBar.LoadBitmapW(IDR_MAINFRAME_256);
	_playControlToolBar.LoadBitmap(IDB_BITMAP_PLAY_ALL);
	
    m_wndToolBar.RedrawWindow();
	_playControlToolBar.RedrawWindow();
	UINT indexb = _playControlToolBar.CommandToIndex(ID_PLAY_PAUSE_ALL);
	CMFCToolBarButton *button = _playControlToolBar.GetButton(indexb);
	button->EnableWindow(true);
	_playControlToolBar.SendMessage(TB_ENABLEBUTTON, ID_PLAY_PAUSE_ALL, (LPARAM)MAKELONG(TRUE, 0));

	InitWorkWhenCreated();

	EnableTimer(true);

	return 0;
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
	// channel explorer()
	_channelExplorerPanel = make_shared<DockableWndContainer>();
	if (!_channelExplorerPanel->Create(L"Channel Explorer", this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_CHANNEL_EXPLORER, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create channel explorer panel\n");
		return FALSE; // failed to create
	}

	auto processChannelTree = make_shared<ChannelTreeCtrl>();
	_channelExplorerPanel->AddChild(processChannelTree);

	CRect rectDummy;
	rectDummy.SetRectEmpty();
	_channelViewImages.Create(IDB_CHANNEL, 16, 0, RGB(255, 0, 255));
	DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS;
	if (!processChannelTree->Create(dwViewStyle, rectDummy, _channelExplorerPanel.get(), 1))
	{
		return -1;
	}
	processChannelTree->SetImageList(&_channelViewImages, TVSIL_NORMAL);

	theApp.objLookup->EventProcessChannelTreeChanged.Subscribe([processChannelTree](const void * sender, const map<shared_ptr<ProcessOpened>, set<shared_ptr<ChannelOpened> > > & tree){
		processChannelTree->UpdateTreeData(tree);
	});

	
	// auto scale
	_autoLayoutPanel = make_shared<DockableWndContainer>();
	if (!_autoLayoutPanel->Create(L"Auto Layout", this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_AUTOLAYOUT, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create auto layout panel\n");
		return FALSE; // failed to create
	}

	auto autoLayoutSlider = make_shared<AutoLayoutSlider>();
	_autoLayoutPanel->AddChild(autoLayoutSlider);

	auto pl = theApp.plotWindowOpenedContainer;
	autoLayoutSlider->EventAutoScale.Subscribe([pl](const void * sender, const int pos){
		int i = 10;
		TRACE1("Auto 1 scale: %d\n", pos);
		pl->AutoLayout(pos);
	});

	if (!autoLayoutSlider->Create(NULL, L"Auto Layout Slider", WS_VISIBLE | WS_CHILD, rectDummy, _autoLayoutPanel.get(), 1, 0))
		return -1;

	// property panel
	_propertyPanel = make_shared<DockableWndContainer>();
	_propertyPanel->SetBgColor(RGB(255, 255, 255));
	if (!_propertyPanel->Create(L"Property", this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_PROPERTIES, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create auto Property Panel\n");
		return FALSE; // failed to create
	}

	auto propertyPanel = _propertyPanel;
	processChannelTree->EventObjSelected.Subscribe([propertyPanel](const void * sender, const shared_ptr<PropObject> & e){

		theApp.currentObject = e;
		propertyPanel->RemoveAllChilds();
		auto propertyPage = e->GetPropertyPage();
		if (propertyPage == 0)
			return;

		propertyPanel->AddChild(propertyPage);
		CRect rectDummy;
		rectDummy.SetRectEmpty();
		propertyPage->Create(WS_CHILD | WS_VISIBLE, rectDummy, propertyPanel.get(), 1);
		propertyPanel->UpdateLayout();
	});

	processChannelTree->EventCloseChannel.Subscribe([pl](const void * sender, const shared_ptr<ChannelOpened> & e){
		pl->CloseChannel(e);
	});

	theApp.plotWindowOpenedContainer->EventBringToFront.Subscribe([propertyPanel](const void * sender, const shared_ptr<PlotWindowOpened> & e){

		theApp.currentObject = e;
		propertyPanel->RemoveAllChilds();
		auto propertyPage = e->GetPropertyPage();
		if (propertyPage == 0)
			return;

		propertyPanel->AddChild(propertyPage);
		CRect rectDummy;
		rectDummy.SetRectEmpty();
		propertyPage->Create(WS_CHILD | WS_VISIBLE, rectDummy, propertyPanel.get(), 1);
		propertyPanel->UpdateLayout();		
	});
	
	auto playPauseController = theApp.playPauseController;
	theApp.plotWindowOpenedContainer->EventProcessSelected.Subscribe([playPauseController](const void * sender, const shared_ptr<ProcessOpened> & e){
		playPauseController->SelectProcess(e);
	});

	//auto controllerDriver = theApp.controlPanelDriver;
	//theApp.plotWindowOpenedContainer->EventBringToFront.Subscribe([controllerDriver](const void * sender, const  & e){
	//	
	//});

	theApp.plotWindowOpenedContainer->EventClosePlotWnd.Subscribe([propertyPanel](const void * sender, const PlotWindowOpenedContainer::ClosePlotWndEvent & e){
		if (theApp.currentObject == e._plotWnd)
		{
			theApp.currentObject = 0;
			propertyPanel->RemoveAllChilds();
			if (propertyPanel->m_hWnd)
				propertyPanel->Invalidate(1);
		}
	});

	// control panel
	_controlPanel = make_shared<DockableWndContainer>();
	if (!_controlPanel->Create(L"Control Panel", this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_CONTROL_PANEL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create auto Control Panel\n");
		return FALSE; // failed to create
	}

	auto controlPanelChildWnd = theApp.controlPanelDriver->GetControlWnd();
	_controlPanel->AddChild(controlPanelChildWnd);
	rectDummy.SetRectEmpty();
	controlPanelChildWnd->Create(NULL, L"Control Panel wnds", WS_CHILD | WS_VISIBLE, rectDummy, _controlPanel.get(), 1);

	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}


BOOL CMainFrame::DockDockpanels()
{
	_channelExplorerPanel->EnableDocking(CBRS_ALIGN_ANY);
	DockPane(_channelExplorerPanel.get());

	_autoLayoutPanel->EnableDocking(CBRS_ALIGN_ANY);
	DockPane(_autoLayoutPanel.get());

	_propertyPanel->EnableDocking(CBRS_ALIGN_ANY);
	DockPane(_propertyPanel.get());

	_controlPanel->EnableDocking(CBRS_ALIGN_ANY);
	DockPane(_controlPanel.get());

	return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hChannelExplorerIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_ICON_CHANNEL_EXPLORER : IDI_ICON_CHANNEL_EXPLORER), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	_channelExplorerPanel->SetIcon(hChannelExplorerIcon, FALSE);

	HICON hAutoLayoutIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_ICON_AUTO_LAYOUT : IDI_ICON_AUTO_LAYOUT), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	_autoLayoutPanel->SetIcon(hAutoLayoutIcon, FALSE);

	HICON hControlPanelIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_ICON_CONTROL_PANEL : IDI_ICON_CONTROL_PANEL), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	_controlPanel->SetIcon(hControlPanelIcon, FALSE);

	HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_ICON_PROPERTIES : IDI_ICON_PROPERTIES), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	_propertyPanel->SetIcon(hPropertiesBarIcon, FALSE);
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

void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CFrameWndEx::OnSettingChange(uFlags, lpszSection);
	//m_wndOutput.UpdateFonts();
}

void CMainFrame::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	this->DeinitWorkWhenClosed();
	HWND hWnd = this->m_hWnd;
	if (hWnd)
		::PostMessage(hWnd, WMME_MAINFRAME_REAL_CLOSE, 0, 0);
	//this->PostMessage(WMME_MAINFRAME_REAL_CLOSE, 0, 0);

#if 0
	CFrameWndEx::OnClose();
#endif
}

void CMainFrame::InitWorkWhenCreated()
{
	auto cm = theApp.channelManager;
	_timerHandlers.push_back(make_pair<int, function<void(void)> >(15, [cm](){
		cm->Update();
	}));

	_timerHandlers.push_back(make_pair<int, function<void(void)> >(2000, [cm](){
		cm->DoGarbageCollection();
	}));

	auto pod = theApp.plotOperationDriver;
	_timerHandlers.push_back(make_pair<int, function<void(void)> >(30, [pod](){
		pod->Plot();
	}));

	auto controllerDriver = theApp.controlPanelDriver;
	auto objLookup = theApp.objLookup;
	_timerHandlers.push_back(make_pair<int, function<void(void)> >(30, [controllerDriver, objLookup](){
		controllerDriver->Update();
		objLookup->PollPauseEvent();
	}));

	
#ifdef _DEBUG
	_timerHandlers.push_back(make_pair<int, function<void(void)> >(3000, [](){
		//int numHandler = SoraDbgPlot::Event::EventHandlerCount();
		//int numQueue = SoraDbgPlot::Task::TaskQueue::MonitorQueueNum();
		//TRACE0("Waiting for task queue tasks\n");
		//TRACE1("Task queue count: %d\n", numQueue);
		//TRACE1("Event handler unreleased: %d\n", numHandler);
		//TRACE1("Task num in queue: %d\n", SoraDbgPlot::Task::TaskQueue::MonitorTaskInQueue());
		//TRACE1("Task num to run: %d\n", SoraDbgPlot::Task::TaskQueue::MonitorTaskCntToRun());
		//TRACE1("Task num runnning: %d\n", SoraDbgPlot::Task::TaskQueue::MonitorTaskCntRunning());
		//TRACE1("Task queue op num: %d\n", SoraDbgPlot::Task::TaskQueue::MonitorTaskQueueOpCnt());
	}));
#endif	

	for (unsigned int i = 0; i < _timerHandlers.size(); i++)
	{
		SetTimer(i, _timerHandlers[i].first, 0);
	}
}

void CMainFrame::DeinitWorkWhenClosed()
{
	for (unsigned int i = 0; i < _timerHandlers.size(); i++)
	{
		KillTimer(i);
	}
}


void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (_bTimeEnabled)
		_timerHandlers[nIDEvent].second();

	CFrameWndEx::OnTimer(nIDEvent);
}

LRESULT CMainFrame::OnRealClose(WPARAM wParam, LPARAM lParam)
{
	CFrameWndEx::OnClose();
	return 0;
}
void CMainFrame::OnPlayPauseAll()
{
	_playControlToolBar.ResetAllImages();
	if(_playPauseState == STATE_PAUSE)
	{
		_playPauseState = STATE_PLAY;
		m_wndToolBar.LoadBitmap(IDR_MAINFRAME_256);
        _playControlToolBar.LoadBitmap(IDB_BITMAP_PLAY_ALL);
		
        m_wndToolBar.RedrawWindow();
	    _playControlToolBar.RedrawWindow();

		theApp.playPauseController->PlayPauseSelectedProcess(false);

		return;
	}
	else
	{
		_playPauseState = STATE_PAUSE;
		m_wndToolBar.LoadBitmapW(IDR_MAINFRAME_256);
		_playControlToolBar.LoadBitmap(IDB_BITMAP_PAUSE_ALL);
		
        m_wndToolBar.RedrawWindow();
	    _playControlToolBar.RedrawWindow();

		theApp.playPauseController->PlayPauseSelectedProcess(true);

		return;
	}
}

void CMainFrame::EnableTimer(bool bEnable)
{
	_bTimeEnabled = bEnable;
}

LRESULT CMainFrame::OnExeTaskQueue(WPARAM wParam, LPARAM lParam)
{
	_pvTaskQueue.Execute(false);

	return 0;
}

void CMainFrame::FlushMsg()
{
	const int WM_MESSAGE_FLUSH = WM_APP + 3;
	unsigned long lastSid = SoraDbgPlot::Task::TaskQueue::Sid();

	HWND hWnd = this->m_hWnd;
	if (hWnd)
	{
		BOOL succ = ::PostMessage(hWnd,WM_MESSAGE_FLUSH, 0, 0);
		if (succ == FALSE)
			return;
	}
	else
		return;

	int matchCount = 0;

	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_MESSAGE_FLUSH)
		{
			unsigned long sid = SoraDbgPlot::Task::TaskQueue::Sid();
			unsigned long numTask = SoraDbgPlot::Task::TaskQueue::RemainingTask();
			if ( (sid == lastSid) && numTask == 0)
			{
				matchCount++;
				if (matchCount == 2)
					break;
			}
			else
				matchCount = 0;

			if (matchCount == 0)
				lastSid = SoraDbgPlot::Task::TaskQueue::Sid();

			HWND hWnd = this->m_hWnd;
			if (hWnd)
			{
				BOOL succ = ::PostMessage(hWnd, WM_MESSAGE_FLUSH, 0, 0);
				if (succ == FALSE)
					break;
			}
			else
				break;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}



void CMainFrame::OpenDocument(const CString & path)
{
	this->EnableTimer(false);
	this->EnableWindow(false);
	this->FlushMsg();
	theApp.objLookup->ClearPlotWindows();
	this->FlushMsg();
	theApp.objLookup->Load(path);
	this->FlushMsg();
	this->EnableTimer(true);
	this->EnableWindow(true);
}

void CMainFrame::SaveDocument(const CString & path)
{
	this->EnableTimer(false);
	this->EnableWindow(false);
	this->FlushMsg();
	theApp.objLookup->Save(path);
	this->FlushMsg();
	this->EnableTimer(true);
	this->EnableWindow(true);
}

void CMainFrame::ClearDocument()
{
	this->EnableTimer(false);
	this->EnableWindow(false);
	this->FlushMsg();
	theApp.objLookup->ClearPlotWindows();
	this->FlushMsg();
	this->EnableTimer(true);
	this->EnableWindow(true);
}

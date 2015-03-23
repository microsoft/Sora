
// DbgPlotViewer.h : main header file for the DbgPlotViewer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include <memory>

#include "SharedSerialNumGenerator.h"
#include "SharedChannelManager.h"
#include "GlobalObjLookup.h"
#include "PlotWindowOpenedContainer.h"
#include "PlotOperationDriver.h"
#include "ControlPanelDriver.h"
#include "PlotWndPlayPauseController.h"

// CDbgPlotViewerApp:
// See DbgPlotViewer.cpp for the implementation of this class
//

class CDbgPlotViewerApp : public CWinAppEx
{
public:
	CDbgPlotViewerApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
public:
	std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannelManager> channelManager;
	std::shared_ptr<ObjLookup> objLookup;
	std::shared_ptr<PlotWindowOpenedContainer> plotWindowOpenedContainer;
	std::shared_ptr<PlotOperationDriver> plotOperationDriver;
	std::shared_ptr<PropObject> currentObject;
	std::shared_ptr<ControlPanelDriver> controlPanelDriver;
	std::shared_ptr<PlotWndPlayPauseController> playPauseController;

private:
	bool DeleteDirRecursive(const std::wstring &path, bool bDir, bool bContent);
	void ClearLog();
	void SetCrashDumpHandler();
	void InitAppObj();
	void DeiniteAppObj();
	void RouteMessage_SharedObj();
	void RouteMessage_TreeUpdate();
	void RouteMessage_OpenOperation();
	void RouteMessage_SaveLoad();
	void RouteMessage_PlayPause();

	bool _bEnableCrashDump;

	std::shared_ptr<SoraDbgPlot::Common::SharedSerialNumGenerator> _serialGen;

	std::shared_ptr<SoraDbgPlot::Task::TaskQueue> _taskQueue;

	bool _bInitialized;

private:
	ULONG_PTR _gdiplusToken;	// GDI+
};

extern CDbgPlotViewerApp theApp;

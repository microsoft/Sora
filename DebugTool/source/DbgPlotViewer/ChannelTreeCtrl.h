#pragma once

#include "ChannelOpened.h"
#include "ProcessOpened.h"
#include "PropObject.h"
#include "PassiveTaskQueue.h"
#include "ChannelAddable.h"
#include "Event.h"

// ChannelTreeCtrl
#define WMME_EXE_TASKQUEUE (WM_APP+1)

class ChannelTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(ChannelTreeCtrl)

public:
	ChannelTreeCtrl();
	virtual ~ChannelTreeCtrl();

	void UpdateTreeData(const std::map<std::shared_ptr<ProcessOpened>, std::set<std::shared_ptr<ChannelOpened> > > & tree);

	SoraDbgPlot::Event::Event<bool> EventClosed;
	SoraDbgPlot::Event::Event<std::shared_ptr<PropObject> > EventObjSelected;

	SoraDbgPlot::Event::Event<std::shared_ptr<ChannelOpened> > EventCloseChannel;

private:
	bool ShowFileDialog(CString & filename);

private:
	void UpdateTreeView();
	std::shared_ptr<ChannelAddable> FindTarget(CPoint & pointOut);


protected:
	DECLARE_MESSAGE_MAP()

private:
	std::map<std::shared_ptr<ProcessOpened>, std::set<std::shared_ptr<ChannelOpened> > > _processChannelTree;

	std::shared_ptr<ChannelOpened> _channelDragged;

	bool _bDragging;
	std::shared_ptr<ChannelAddable> _lastObj;
	std::shared_ptr<ChannelOpened> _contextMenuObj;

	void HighlightWindow(CWnd * wnd, bool highlight);
public:
	afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnClose();
	afx_msg LRESULT OnExeTaskQueue(WPARAM, LPARAM);

private:
	PassiveTaskQueue _pvTaskQueue;
public:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	virtual void PostNcDestroy();
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult);	afx_msg void OnCloseChannel();
	afx_msg void OnChannelSaveSelection();
	afx_msg void OnChannelSaveAll();
	afx_msg void OnTextChannelExport();
	afx_msg void OnTextChannelClose();
};



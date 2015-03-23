
// DebugPlotViewerDoc.h : interface of the CDebugPlotViewerDoc class
//


#pragma once

#include <vector>
#include "AppMessage.h"
#include "SeriesObj.h"
#include "PlotWndArea.h"

#import <msxml6.dll>
#include <tchar.h>
//HRESULT CreateAndAddElementNode(IXMLDOMDocument *pDom, PCWSTR wszName, PCWSTR wszNewline, IXMLDOMNode *pParent, IXMLDOMElement **ppElement = NULL);
class CDebugPlotViewerDoc : public CDocument
{
protected: // create from serialization only
	CDebugPlotViewerDoc();
	DECLARE_DYNCREATE(CDebugPlotViewerDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CDebugPlotViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS

public:
	PlotWndAreaProp plotWndAreaProp;

	//std::vector<CWnd *> observers;

	void DBG_PrintChannelTree();
	void DBG_PrintWndTree();

	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();

	void ResetDocumentState();

	void createSeries(PlotWndProp *pPlotWnd,MSXML2::IXMLDOMNodeListPtr pls, DebuggingProcessProp *pProcess);	
	SeriesProp * MatchSeries(const CString& name, DebuggingProcessProp * process, const type_info& type);
	SeriesProp * MatchSeriesText(const CString& name, DebuggingProcessProp * process, bool isLog);

	std::set<DebuggingProcessProp *> matchedProcess;
	std::set<SeriesProp *> matchedSeries;

	std::set<DebuggingProcessProp *> newProcess;
	std::set<SeriesProp *> newSeries;
};

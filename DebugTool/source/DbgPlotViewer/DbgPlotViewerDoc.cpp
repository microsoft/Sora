
// DbgPlotViewerDoc.cpp : implementation of the CDbgPlotViewerDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "DbgPlotViewer.h"
#endif

#include "MainFrm.h"
#include "DbgPlotViewerDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDbgPlotViewerDoc

IMPLEMENT_DYNCREATE(CDbgPlotViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CDbgPlotViewerDoc, CDocument)
END_MESSAGE_MAP()


// CDbgPlotViewerDoc construction/destruction

CDbgPlotViewerDoc::CDbgPlotViewerDoc()
{
	// TODO: add one-time construction code here

}

CDbgPlotViewerDoc::~CDbgPlotViewerDoc()
{
}

BOOL CDbgPlotViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	
	((CMainFrame*)AfxGetMainWnd())->ClearDocument();

	return TRUE;
}




// CDbgPlotViewerDoc serialization

void CDbgPlotViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CDbgPlotViewerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CDbgPlotViewerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CDbgPlotViewerDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CDbgPlotViewerDoc diagnostics

#ifdef _DEBUG
void CDbgPlotViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDbgPlotViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CDbgPlotViewerDoc commands


BOOL CDbgPlotViewerDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// TODO: Add your specialized code here and/or call the base class

	BOOL succ = CDocument::OnSaveDocument(lpszPathName);

	((CMainFrame*)AfxGetMainWnd())->SaveDocument(lpszPathName);	

	return succ;
}


BOOL CDbgPlotViewerDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  Add your specialized creation code here
	((CMainFrame*)AfxGetMainWnd())->OpenDocument(lpszPathName);

	return TRUE;
}

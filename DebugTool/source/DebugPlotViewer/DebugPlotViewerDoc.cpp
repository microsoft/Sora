
// DebugPlotViewerDoc.cpp : implementation of the CDebugPlotViewerDoc class
//

//#import <msxml6.dll>
#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "DebugPlotViewer.h"
#endif

#include "DebugPlotViewerDoc.h"

#include <propkey.h>
#include <algorithm>
#include <tchar.h>

#include "AppMessage.h"
#include "Logger.h"
#include "SharedObjManager.h"
#include "SeriesLine.h"
#include "SeriesText.h"
#include "SeriesDots.h"
#include "SeriesLog.h"
#include "SeriesSpectrum.h"
#include "BitmapTypePlotWndProp.h"
#include "PlotWndPropLine.h"
#include "PlotWndPropDots.h"
#include "PlotWndPropText.h"
#include "PlotWndPropSpectrum.h"
#include "PlotWndPropLog.h"
#include "HelperFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//#define CHK_HR(stmt)        do { hr=(stmt); if (FAILED(hr)) goto CleanUp; } while(0)
//
//
//#define CHK_ALLOC(p)        do { if (!(p)) { hr = E_OUTOFMEMORY; goto CleanUp; } } while(0)
//
//
//#define SAFE_RELEASE(p)     do { if ((p)) { (p)->Release(); (p) = NULL; } } while(0)



IMPLEMENT_DYNCREATE(CDebugPlotViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CDebugPlotViewerDoc, CDocument)
END_MESSAGE_MAP()


//HRESULT AppendChildToParent(IXMLDOMNode *pChild, IXMLDOMNode *pParent)
//{
//    HRESULT hr = S_OK;
//    IXMLDOMNode *pChildOut = NULL;
//    CHK_HR(pParent->appendChild(pChild, &pChildOut));
//
//CleanUp:
//    SAFE_RELEASE(pChildOut);
//    return hr;
//}
//
//
//HRESULT CreateElement(IXMLDOMDocument *pXMLDom, PCWSTR wszName, IXMLDOMElement **ppElement)
//{
//    HRESULT hr = S_OK;
//    *ppElement = NULL;
//
//    BSTR bstrName = SysAllocString(wszName);
//    CHK_ALLOC(bstrName);
//    CHK_HR(pXMLDom->createElement(bstrName, ppElement));
//
//CleanUp:
//    SysFreeString(bstrName);
//    return hr;
//}


// CDebugPlotViewerDoc construction/destruction
//HRESULT CreateAndAddTextNode(IXMLDOMDocument *pDom, PCWSTR wszText, IXMLDOMNode *pParent)
//{
//    HRESULT hr = S_OK;    
//    IXMLDOMText *pText = NULL;
//
//    BSTR bstrText = SysAllocString(wszText);
//    CHK_ALLOC(bstrText);
//
//    CHK_HR(pDom->createTextNode(bstrText, &pText));
//    CHK_HR(AppendChildToParent(pText, pParent));
//
//CleanUp:
//    SAFE_RELEASE(pText);
//    SysFreeString(bstrText);
//    return hr;
//}

SeriesProp * CDebugPlotViewerDoc::MatchSeriesText(const CString& name, DebuggingProcessProp * process, bool isLog)
{
	TextSeriesProp * textSeriesProp = (TextSeriesProp *)this->MatchSeries(name, process, typeid(TextSeriesProp));
	if (textSeriesProp == 0)
		return 0;

	if (isLog == textSeriesProp->isLog)
		return textSeriesProp;
	else
		return 0;
}

// Helper function to create and append a CDATA node to a parent node.
SeriesProp * CDebugPlotViewerDoc::MatchSeries(const CString& name, DebuggingProcessProp * process, const type_info& type)
{
	wstring keyName;

	SharedObjManager * sharedObjManager = SharedObjManager::Instance();

	keyName.append(process->moduleName);
	keyName.append(L"|");
	keyName.append(name);

	std::pair<std::multimap<wstring, SeriesProp *>::iterator, std::multimap<wstring, SeriesProp *>::iterator> range;
	range = sharedObjManager->allSeriesObjs.equal_range(keyName);

	std::multimap<wstring, SeriesProp *>::iterator iter;
	for (iter = range.first; iter != range.second; iter++)
	{
		SeriesProp * series = (*iter).second;
		std::set<SeriesProp *>::iterator iterMatchedSeries;
		iterMatchedSeries = matchedSeries.find(series);
		if (
			(series->GetProcess() == process) && 
			(iterMatchedSeries == matchedSeries.end()) &&
			(typeid(*series) == type)
			)
		{
			ASSERT(series->isOpened == false);
			matchedSeries.insert(series);
			return series;
		}
	}
	
	return 0;
}

void CDebugPlotViewerDoc::createSeries(PlotWndProp *pPlotWnd,MSXML2::IXMLDOMNodeListPtr pls, DebuggingProcessProp *pProcess)
{
	MSXML2::IXMLDOMNodePtr pss =NULL;
	MSXML2::IXMLDOMNodePtr ps2 =NULL;
	LineSeriesProp *pSeriesline=NULL;
	DotsSeriesProp *pSeriesdots=NULL;
	TextSeriesProp *pSeriestext=NULL;
	SpectrumSeriesProp * pSeriesSpectrum=NULL;
	int j=0;
	int tlx=0;
	int tly=0;
	int brx=0;
	int bry=0;

	CString cs;

	for(long j=0;j<pls->length;j++)
	{
		pss=pls->item[j];
		ps2=pss->selectSingleNode(L"SType");
		if(ps2->text==_bstr_t(typeid(LineSeriesProp).name()))
		{
			ps2=pss->selectSingleNode(L"SName");
			cs.Format(_T("%S"),(LPCSTR)ps2->text);
			pSeriesline = (LineSeriesProp *)MatchSeries(cs, pProcess, typeid(LineSeriesProp));
			if (!pSeriesline)
			{
				pSeriesline=new LineSeriesProp;
				//pSeriesline->type=TYPE_LINE;
				pSeriesline->name=cs;
				pSeriesline->SetProcess(pProcess);
				pProcess->AddSeries(pSeriesline);
				newSeries.insert(pSeriesline);
			}

			//ps2=pss->selectSingleNode(L"colorIsSet");
			//if(ps2->text==(_bstr_t)(L"0"))
			//	pSeriesline->colorIsSet=0;
			//else
			//	pSeriesline->colorIsSet=1;

			ps2=pss->selectSingleNode(L"color");
			pSeriesline->color=(atol)((LPCSTR)ps2->text);
			ps2=pss->selectSingleNode(L"subRectTLX");
			tlx=atoi((LPCSTR)(ps2->text));
			ps2=pss->selectSingleNode(L"subRectTLY");
			tly=atoi((LPCSTR)(ps2->text));
			ps2=pss->selectSingleNode(L"subRectBRX");
			brx=atoi((LPCSTR)(ps2->text));
			ps2=pss->selectSingleNode(L"subRectBRY");
			bry=atoi((LPCSTR)(ps2->text));
			pSeriesline->rect.SetRect(tlx,tly,brx,bry);
			pSeriesline->SetPlotWndProp(pPlotWnd);
			pPlotWnd->AddSeries(pSeriesline);
		}

		else if(ps2->text==_bstr_t(typeid(DotsSeriesProp).name()))
		{
			ps2=pss->selectSingleNode(L"SName");
			cs.Format(_T("%S"),(LPCSTR)ps2->text);
			pSeriesdots = (DotsSeriesProp *)MatchSeries(cs, pProcess, typeid(DotsSeriesProp));

			if (!pSeriesdots)
			{
				pSeriesdots=new DotsSeriesProp;
				//pSeriesdots->type=TYPE_DOTS;
				pSeriesdots->name=cs;
				pSeriesdots->SetProcess(pProcess);
				pProcess->AddSeries(pSeriesdots);
				newSeries.insert(pSeriesdots);
			}
			
			//ps2=pss->selectSingleNode(L"colorIsSet");
			//if(ps2->text==(_bstr_t)(L"0"))
			//	pSeriesdots->colorIsSet=0;
			//else
			//	pSeriesdots->colorIsSet=1;
					
			ps2=pss->selectSingleNode(L"color");
			pSeriesdots->color=(atol)((LPCSTR)ps2->text);

			ps2=pss->selectSingleNode(L"subRectTLX");
			tlx=atoi((LPCSTR)(ps2->text));
			ps2=pss->selectSingleNode(L"subRectTLY");
			tly=atoi((LPCSTR)(ps2->text));
			ps2=pss->selectSingleNode(L"subRectBRX");
			brx=atoi((LPCSTR)(ps2->text));
			ps2=pss->selectSingleNode(L"subRectBRY");
			bry=atoi((LPCSTR)(ps2->text));
			pSeriesdots->rect.SetRect(tlx,tly,brx,bry);
			pSeriesdots->SetPlotWndProp(pPlotWnd);
			pPlotWnd->AddSeries(pSeriesdots);
		}
		else if (ps2->text == _bstr_t(typeid(LogSeriesProp).name()))
		{
			ps2 = pss->selectSingleNode(L"SName");
			cs.Format(_T("%S"),(LPCSTR)ps2->text);

			LogSeriesProp * pSeriesLog = (LogSeriesProp *)MatchSeries(cs, pProcess, typeid(LogSeriesProp));
			if (!pSeriesLog)
			{
				pSeriesLog = new LogSeriesProp;
				pSeriesLog->name = cs;
				pSeriesLog->SetProcess(pProcess);
				pProcess->AddSeries(pSeriesLog);
				newSeries.insert(pSeriesLog);
			}

			ps2=pss->selectSingleNode(L"color");
			pSeriesLog->SetColor((atol)((LPCSTR)ps2->text));

			//if (pSeriesLog->smSeriesInfo)
			//{
			//	SharedSeriesInfo * sharedSeriesInfo = (SharedSeriesInfo *)pSeriesLog->smSeriesInfo->GetAddress();
			//	sharedSeriesInfo->replace = false;
			//}

			ps2=pss->selectSingleNode(L"subRectTLX");
			tlx=atoi((LPCSTR)(ps2->text));
			ps2=pss->selectSingleNode(L"subRectTLY");
			tly=atoi((LPCSTR)(ps2->text));
			ps2=pss->selectSingleNode(L"subRectBRX");
			brx=atoi((LPCSTR)(ps2->text));
			ps2=pss->selectSingleNode(L"subRectBRY");
			bry=atoi((LPCSTR)(ps2->text));
			pSeriesLog->rect.SetRect(tlx,tly,brx,bry);
			pSeriesLog->SetPlotWndProp(pPlotWnd);
			pPlotWnd->AddSeries(pSeriesLog);			
		}
		else if(ps2->text==_bstr_t(typeid(TextSeriesProp).name()))
		{
			ps2=pss->selectSingleNode(L"SName");
			cs.Format(_T("%S"),(LPCSTR)ps2->text);

			bool isLog;
			ps2 = pss->selectSingleNode(L"IsLog");
			if (ps2->text == (_bstr_t)(L"0"))
				isLog = false;
			else
				isLog = true;

			pSeriestext = (TextSeriesProp *)MatchSeriesText(cs, pProcess, isLog);
			if (!pSeriestext)
			{
				pSeriestext=new TextSeriesProp;
				//pSeriestext->type=TYPE_TEXT;
				pSeriestext->name=cs;
				pSeriestext->SetProcess(pProcess);
				pProcess->AddSeries(pSeriestext);
				newSeries.insert(pSeriestext);
			}

			//ps2=pss->selectSingleNode(L"colorIsSet");
			//if(ps2->text==(_bstr_t)(L"0"))
			//	pSeriestext->colorIsSet=0;
			//else
			//	pSeriestext->colorIsSet=1;
					
			ps2 = pss->selectSingleNode(L"IsLog");
			if (ps2->text == (_bstr_t)(L"0"))
				pSeriestext->isLog = 0;
			else
				pSeriestext->isLog = 1;

			ps2=pss->selectSingleNode(L"color");
			pSeriestext->color=(atol)((LPCSTR)ps2->text);

			ps2 = pss->selectSingleNode(L"replaceMode");
			if (ps2->text == (_bstr_t)(L"0"))
				pSeriestext->replaceMode = false;
			else
				pSeriestext->replaceMode = true;

			//if (pSeriestext->smSeriesInfo)
			//{
			//	SharedSeriesInfo * sharedSeriesInfo = (SharedSeriesInfo *)pSeriestext->smSeriesInfo->GetAddress();
			//	sharedSeriesInfo->replace = pSeriestext->replaceMode;
			//}

			ps2=pss->selectSingleNode(L"subRectTLX");
			tlx=atoi((LPCSTR)(ps2->text));
			ps2=pss->selectSingleNode(L"subRectTLY");
			tly=atoi((LPCSTR)(ps2->text));
			ps2=pss->selectSingleNode(L"subRectBRX");
			brx=atoi((LPCSTR)(ps2->text));
			ps2=pss->selectSingleNode(L"subRectBRY");
			bry=atoi((LPCSTR)(ps2->text));
			pSeriestext->rect.SetRect(tlx,tly,brx,bry);
			pSeriestext->SetPlotWndProp(pPlotWnd);
			pPlotWnd->AddSeries(pSeriestext);
		}
		else if (ps2->text==_bstr_t(typeid(SpectrumSeriesProp).name()))
		{
			ps2=pss->selectSingleNode(L"SName");
			cs.Format(_T("%S"),(LPCSTR)ps2->text);
			pSeriesSpectrum = (SpectrumSeriesProp *)MatchSeries(cs, pProcess, typeid(SpectrumSeriesProp));
			if (!pSeriesSpectrum)
			{
				pSeriesSpectrum =new SpectrumSeriesProp;
				//pSeriesSpectrum->type=TYPE_SPECTRUM;
				pSeriesSpectrum->name=cs;
				pSeriesSpectrum->SetProcess(pProcess);
				pProcess->AddSeries(pSeriesSpectrum);
				newSeries.insert(pSeriesSpectrum);
			}

			//ps2=pss->selectSingleNode(L"colorIsSet");
			//if(ps2->text==(_bstr_t)(L"0"))
			//	pSeriesSpectrum->colorIsSet=0;
			//else
			//	pSeriesSpectrum->colorIsSet=1;

			ps2=pss->selectSingleNode(L"color");
			pSeriesSpectrum->color=(atol)((LPCSTR)ps2->text);
			ps2=pss->selectSingleNode(L"subRectTLX");
			tlx=atoi((LPCSTR)(ps2->text));
			ps2=pss->selectSingleNode(L"subRectTLY");
			tly=atoi((LPCSTR)(ps2->text));
			ps2=pss->selectSingleNode(L"subRectBRX");
			brx=atoi((LPCSTR)(ps2->text));
			ps2=pss->selectSingleNode(L"subRectBRY");
			bry=atoi((LPCSTR)(ps2->text));
			pSeriesSpectrum->rect.SetRect(tlx,tly,brx,bry);
			pSeriesSpectrum->SetPlotWndProp(pPlotWnd);
			pPlotWnd->AddSeries(pSeriesSpectrum);			
		}

						
	}

}


//HRESULT CreateAndAddElementNode(IXMLDOMDocument *pDom, PCWSTR wszName, PCWSTR wszNewline, IXMLDOMNode *pParent, IXMLDOMElement **ppElement )
//{
//    HRESULT hr = S_OK;
//    IXMLDOMElement* pElement = NULL;
//
//    CHK_HR(CreateElement(pDom, wszName, &pElement));
//    
//    CHK_HR(CreateAndAddTextNode(pDom, wszNewline, pParent));
//    
//    CHK_HR(AppendChildToParent(pElement, pParent));
//
//CleanUp:
//    if (ppElement)
//        *ppElement = pElement;  
//    else
//        SAFE_RELEASE(pElement); 
//
//    return hr;
//}



CDebugPlotViewerDoc::CDebugPlotViewerDoc()
{
	// TODO: add one-time construction code here

}

CDebugPlotViewerDoc::~CDebugPlotViewerDoc()
{
}

BOOL CDebugPlotViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	//LARGE_INTEGER frequency;
	//QueryPerformanceFrequency(&frequency);
	//LARGE_INTEGER timer1, timer2, timer3;

	//QueryPerformanceCounter(&timer1);
	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_MAIN_FRAME_ENABLE_TIMER, (LPARAM)false);
	//QueryPerformanceCounter(&timer2);
	ResetDocumentState();
	//QueryPerformanceCounter(&timer3);

	//Logger::Print(L"(%I64d,%I64d)\n", 
	//				(timer2.QuadPart - timer1.QuadPart) * 1000000000 / frequency.QuadPart,
	//				(timer3.QuadPart - timer2.QuadPart) * 1000000000 / frequency.QuadPart);

	::AfxGetMainWnd()->PostMessage(WM_APP, CMD_NEW_DOCUMENT, (LPARAM)this);
	BaseProperty * property = this->plotWndAreaProp.GetPropertyPage();
	::AfxGetMainWnd()->PostMessage(WM_APP, CMD_CHANGE_PROPERTY, (LPARAM)property);

	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_MAIN_FRAME_ENABLE_TIMER, (LPARAM)true);

	return TRUE;
}




// CDebugPlotViewerDoc serialization

void CDebugPlotViewerDoc::Serialize(CArchive& ar)
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
void CDebugPlotViewerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
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
void CDebugPlotViewerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CDebugPlotViewerDoc::SetSearchContent(const CString& value)
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

// CDebugPlotViewerDoc diagnostics

#ifdef _DEBUG
void CDebugPlotViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDebugPlotViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


void CDebugPlotViewerDoc::DBG_PrintWndTree()
{
	//Logger::Print(L"\nwnd tree:\n");
	//std::vector<PlotWndProp *>::iterator iterPlotWnd;
	//for (
	//	iterPlotWnd = this->plotWndProp.begin();
	//	iterPlotWnd != this->plotWndProp.end();
	//iterPlotWnd++
	//	)
	//{
	//	Logger::Print(L"%s, type:%d\n", (*iterPlotWnd)->name.GetBuffer(), (*iterPlotWnd)->type);
	//	std::vector<SeriesProp *>::iterator iterSeries;
	//	for (
	//		iterSeries = (*iterPlotWnd)->series.begin();
	//		iterSeries != (*iterPlotWnd)->series.end();
	//	iterSeries++
	//		)
	//	{
	//		Logger::Print(L"    %s, %s, type:%d\n", (*iterSeries)->name.GetBuffer(), (*iterSeries)->plotWnd->name.GetBuffer(), (*iterSeries)->type);
	//	}
	//}	
}

void CDebugPlotViewerDoc::DBG_PrintChannelTree()
{
	//Logger::Print(L"\nchannel tree:\n");
	//std::vector<DebuggingProcessProp *>::iterator iterProcess;
	//for (
	//	iterProcess = this->debuggingProcesses.begin();
	//	iterProcess != this->debuggingProcesses.end();
	//iterProcess++
	//	)
	//{
	//	Logger::Print(L"%s:%d\n", (*iterProcess)->moduleName.GetBuffer(), (*iterProcess)->pid);
	//	std::vector<SeriesProp *>::iterator iterSeries;
	//	for (
	//		iterSeries = (*iterProcess)->series.begin();
	//		iterSeries != (*iterProcess)->series.end();
	//	iterSeries++
	//		)
	//	{
	//		Logger::Print(L"    %s, %s:%d\n", 
	//			(*iterSeries)->name.GetBuffer(), 
	//			(*iterSeries)->processProp->moduleName, 
	//			(*iterSeries)->processProp->pid
	//			);
	//	}
	//}
}

BOOL CDebugPlotViewerDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  Add your specialized creation code here
	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_MAIN_FRAME_ENABLE_TIMER, (LPARAM)false);

	ResetDocumentState();

	matchedProcess.clear();
	matchedSeries.clear();
	newProcess.clear();
	newSeries.clear();

	SharedObjManager * manager = SharedObjManager::Instance();
	manager->Lock();

	// XML TODO
	//...
	VARIANT var;
	VariantInit(&var);
	BSTR bstr=SysAllocString(lpszPathName);
	V_VT(&var)   = VT_BSTR;
	V_BSTR(&var) = bstr;
	MSXML2::IXMLDOMDocumentPtr pXMLDom;
    HRESULT hr = pXMLDom.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);
     
    pXMLDom->async = VARIANT_FALSE;
    pXMLDom->validateOnParse = VARIANT_FALSE;
    pXMLDom->resolveExternals = VARIANT_FALSE;
	pXMLDom->load(var);
	MSXML2::IXMLDOMNodePtr pNode=NULL ; 
    MSXML2::IXMLDOMNodeListPtr pnl = pXMLDom->selectNodes(L"//PlotWnd");
	int k=pnl->length;
	MSXML2::IXMLDOMNodePtr pn2=NULL ; 
	MSXML2::IXMLDOMNodeListPtr pls=NULL;
	BSTR str1=SysAllocString(L" ");
	PlotWndPropLine *pPlotline=NULL;
	PlotWndPropDots *pPlotdots=NULL;
	PlotWndPropText *pPlottext=NULL;
	PlotWndPropLog * pPlotlog = NULL;
	//LineSeriesProp *pSeriesline=NULL;
	//DotsSeriesProp *pSeriesdots=NULL;
	//TextSeriesProp *pSeriestext=NULL;
	DebuggingProcessProp *pProcess=NULL;
	DebuggingProcessProp *pProcess2=NULL;
	DebuggingProcessProp *pProcess3=NULL;
	PlotWndAreaProp *pArea=NULL;
	CString cs;
	BSTR proname;
	int proid;
	std::map<int , DebuggingProcessProp *>  processMap; 
	std::map<int , DebuggingProcessProp *>::iterator itermap;
	int tlx;
	int tly;
	int brx;
	int bry;
	CRect;
	int ifc;

    if(pnl)
    {
        for (long i = 0; i < pnl->length; i++)
        {
			pNode=pnl->item[i];
			pn2=pNode->selectSingleNode(L"Process");
			proname=pn2->text;
			cs.Format(_T("%s"),(LPCTSTR)proname);

			pn2=pNode->selectSingleNode(L"ProcessName");
			proid=atoi((LPCSTR)(pn2->text));
			itermap=processMap.find(proid);
			if(itermap==processMap.end())
			{
				//EXPERIMENT

				// try match process
				{
					pProcess2 = 0;
					wstring key;
					key.append(cs);
					std::pair<std::multimap<wstring, DebuggingProcessProp *>::iterator, std::multimap<wstring, DebuggingProcessProp *>::iterator> range;  
					range = manager->allProcessObjs.equal_range(key);
					std::multimap<wstring, DebuggingProcessProp *>::iterator iter;
					for (iter = range.first; iter != range.second; iter++)
					{
						DebuggingProcessProp * process = (*iter).second;

						std::set<DebuggingProcessProp *>::iterator iterMatchedProcess = 
							matchedProcess.find(process);
						if (iterMatchedProcess == matchedProcess.end())
						{
							ASSERT(process->openCount == 0);
							ASSERT(process->moduleName.Compare(cs) == 0);
							pProcess2 =  process;
							matchedProcess.insert(process);
							break;
						}
					}

					if (!pProcess2)
					{
						pProcess2=new DebuggingProcessProp;
						newProcess.insert(pProcess2);
					}
					processMap.insert(map<int,DebuggingProcessProp *>::value_type(proid,pProcess2));
				}
				
				pProcess2->moduleName=cs;
				pProcess=pProcess2;
			}
			else 
			
			/*pProcess3=(*itermap).second;*/
				pProcess=(*itermap).second;
			/*pProcess=pProcess;*/

			pn2=pNode->selectSingleNode(L"WndType");

			if(pn2->text==_bstr_t(typeid(PlotWndPropLine).name()))
			{
				PlotWndPropLine *pPlotline =new PlotWndPropLine;
				pPlotline->SetPlotWndArea(&(this->plotWndAreaProp));
				this->plotWndAreaProp.AddPlotWnd(pPlotline);
				//pPlotline->type=TYPE_LINE;

				pn2=pNode->selectSingleNode(L"Logarithm");
				if(pn2->text==(_bstr_t)(L"0"))
					pPlotline->isLog = 0;
				else
					pPlotline->isLog = 1;				

				pn2=pNode->selectSingleNode(L"WndName");
				cs.Format(_T("%S"),(LPCSTR)pn2->text);
				pPlotline->SetName(cs);
				pn2=pNode->selectSingleNode(L"WndNameIsSet");
				if(pn2->text==(_bstr_t)(L"0"))
					pPlotline->nameIsSet=0;
				else
					pPlotline->nameIsSet=1;
				pn2=pNode->selectSingleNode(L"WndautoScale");
				if(pn2->text==(_bstr_t)(L"0"))
					pPlotline->autoScale=0;
				else
					pPlotline->autoScale=1;
				
				pn2=pNode->selectSingleNode(L"WndshowGrid");
				if(pn2->text==(_bstr_t)(L"0"))
					pPlotline->showGrid=0;
				else
					pPlotline->showGrid=1;

				pn2=pNode->selectSingleNode(L"WndmaxValue");
				ifc=atoi((LPCSTR)(pn2->text));
				pPlotline->SetMaxValue(ifc);

				pn2=pNode->selectSingleNode(L"WndminValue");
				ifc=atoi((LPCSTR)(pn2->text));
				pPlotline->SetMinValue(ifc);
				
				if (pPlotline->autoScale)
				{
					pPlotline->autoScaleReset = true;
				}

				pn2=pNode->selectSingleNode(L"WndFrameCount");

				ifc=atoi((LPCSTR)(pn2->text));
				pPlotline->frameCount=ifc;

				pn2=pNode->selectSingleNode(L"WndRectTLX");
				tlx=atoi((LPCSTR)(pn2->text));
				pn2=pNode->selectSingleNode(L"WndRectTLY");
				tly=atoi((LPCSTR)(pn2->text));
				pn2=pNode->selectSingleNode(L"WndRectBRX");
				brx=atoi((LPCSTR)(pn2->text));
				pn2=pNode->selectSingleNode(L"WndRectBRY");
				bry=atoi((LPCSTR)(pn2->text));
				pPlotline->rect.SetRect(tlx,tly,brx,bry);
				pls=pNode->selectNodes(L"./Series");
				createSeries(pPlotline,pls,pProcess);
			}

			else if(pn2->text==_bstr_t(typeid(PlotWndPropSpectrum).name()))
			{
				PlotWndPropSpectrum *pPlotSpectrum =new PlotWndPropSpectrum;
				pPlotSpectrum->SetPlotWndArea(&(this->plotWndAreaProp));
				this->plotWndAreaProp.AddPlotWnd(pPlotSpectrum);
				//pPlotSpectrum->type=TYPE_SPECTRUM;
			
				pn2=pNode->selectSingleNode(L"Logarithm");
				if(pn2->text==(_bstr_t)(L"0"))
					pPlotSpectrum->isLog = 0;
				else
					pPlotSpectrum->isLog = 1;				
				
				pn2=pNode->selectSingleNode(L"WndName");
				cs.Format(_T("%S"),(LPCSTR)pn2->text);
				pPlotSpectrum->SetName(cs);
				pn2=pNode->selectSingleNode(L"WndNameIsSet");
				if(pn2->text==(_bstr_t)(L"0"))
					pPlotSpectrum->nameIsSet=0;
				else
					pPlotSpectrum->nameIsSet=1;					
				pn2=pNode->selectSingleNode(L"WndFrameCount");

				ifc=atoi((LPCSTR)(pn2->text));
				pPlotSpectrum->frameCount=ifc;
				
				pn2=pNode->selectSingleNode(L"WndshowGrid");
				if(pn2->text==(_bstr_t)(L"0"))
					pPlotSpectrum->showGrid=0;
				else
					pPlotSpectrum->showGrid=1;

				pn2=pNode->selectSingleNode(L"WndautoScale");
				if(pn2->text==(_bstr_t)(L"0"))
					pPlotSpectrum->autoScale=0;
				else
					pPlotSpectrum->autoScale=1;
				
				pn2=pNode->selectSingleNode(L"WndmaxValue");
				ifc=atoi((LPCSTR)(pn2->text));
				pPlotSpectrum->SetMaxValue(ifc);

				pn2=pNode->selectSingleNode(L"WndminValue");
				ifc=atoi((LPCSTR)(pn2->text));
				pPlotSpectrum->SetMinValue(ifc);

				if (pPlotSpectrum->autoScale)
				{
					pPlotSpectrum->autoScaleReset = true;
				}

				pn2=pNode->selectSingleNode(L"WndRectTLX");
				tlx=atoi((LPCSTR)(pn2->text));
				pn2=pNode->selectSingleNode(L"WndRectTLY");
				tly=atoi((LPCSTR)(pn2->text));
				pn2=pNode->selectSingleNode(L"WndRectBRX");
				brx=atoi((LPCSTR)(pn2->text));
				pn2=pNode->selectSingleNode(L"WndRectBRY");
				bry=atoi((LPCSTR)(pn2->text));
				pPlotSpectrum->rect.SetRect(tlx,tly,brx,bry);
				pls=pNode->selectNodes(L"./Series");
				createSeries(pPlotSpectrum,pls,pProcess);
			}
			else if(pn2->text==_bstr_t(typeid(PlotWndPropDots).name()))
			{
				
				PlotWndPropDots *pPlotdots =new PlotWndPropDots;
				pPlotdots->SetPlotWndArea(&(this->plotWndAreaProp));
				this->plotWndAreaProp.AddPlotWnd(pPlotdots);
				//pPlotdots->type=TYPE_DOTS;
				pn2=pNode->selectSingleNode(L"WndName");
				cs.Format(_T("%S"),(LPCSTR)pn2->text);
				pPlotdots->SetName(cs);
				pn2=pNode->selectSingleNode(L"WndNameIsSet");
				if(pn2->text==(_bstr_t)(L"0"))
					pPlotdots->nameIsSet=0;
				else
					pPlotdots->nameIsSet=1;
				pn2=pNode->selectSingleNode(L"WndautoScale");
				if(pn2->text==(_bstr_t)(L"0"))
					pPlotdots->autoScale=0;
				else
					pPlotdots->autoScale=1;
				

				pn2=pNode->selectSingleNode(L"WndshowGrid");
				if(pn2->text==(_bstr_t)(L"0"))
					pPlotdots->showGrid=0;
				else
					pPlotdots->showGrid=1;
				

				pn2=pNode->selectSingleNode(L"WndmaxValue");
				ifc=atoi((LPCSTR)(pn2->text));
				pPlotdots->SetMaxValue(ifc);

				if (pPlotdots->autoScale)
					pPlotdots->autoScaleMaxValue = 0;

				pn2=pNode->selectSingleNode(L"Wndluminescence");
				ifc=atoi((LPCSTR)(pn2->text));
				pPlotdots->luminescence= (ifc != 0);
				
				pn2=pNode->selectSingleNode(L"DotCount");

				ifc=atoi((LPCSTR)(pn2->text));
				pPlotdots->dotShowCount=ifc;

				pn2=pNode->selectSingleNode(L"WndRectTLX");
				tlx=atoi((LPCSTR)(pn2->text));
				pn2=pNode->selectSingleNode(L"WndRectTLY");
				tly=atoi((LPCSTR)(pn2->text));
				pn2=pNode->selectSingleNode(L"WndRectBRX");
				brx=atoi((LPCSTR)(pn2->text));
				pn2=pNode->selectSingleNode(L"WndRectBRY");
				bry=atoi((LPCSTR)(pn2->text));
				pPlotdots->rect.SetRect(tlx,tly,brx,bry);
				pn2=pNode->selectSingleNode(L"Process");
				pn2=pNode->selectSingleNode(L"ProcessName");
				pls=pNode->selectNodes(L"./Series");
				createSeries(pPlotdots,pls,pProcess);
				
			}

			else if(pn2->text==_bstr_t(typeid(PlotWndPropText).name()))
			{
				
			    PlotWndPropText *pPlottext =new PlotWndPropText;
				pPlottext->SetPlotWndArea(&(this->plotWndAreaProp));
				this->plotWndAreaProp.AddPlotWnd(pPlottext);
				//pPlottext->type=TYPE_TEXT;
				pn2=pNode->selectSingleNode(L"WndName");
				cs.Format(_T("%S"),(LPCSTR)pn2->text);
				pPlottext->SetName(cs);
				pn2=pNode->selectSingleNode(L"WndNameIsSet");
				if(pn2->text==(_bstr_t)(L"0"))
					pPlottext->nameIsSet=0;
				else
					pPlottext->nameIsSet=1;
				pn2=pNode->selectSingleNode(L"WndFrameCount");
				ifc=atoi((LPCSTR)(pn2->text));
				pPlottext->frameCount=ifc;
				pn2=pNode->selectSingleNode(L"WndRectTLX");
				tlx=atoi((LPCSTR)(pn2->text));
				pn2=pNode->selectSingleNode(L"WndRectTLY");
				tly=atoi((LPCSTR)(pn2->text));
				pn2=pNode->selectSingleNode(L"WndRectBRX");
				brx=atoi((LPCSTR)(pn2->text));
				pn2=pNode->selectSingleNode(L"WndRectBRY");
				bry=atoi((LPCSTR)(pn2->text));
				pPlottext->rect.SetRect(tlx,tly,brx,bry);
				pn2=pNode->selectSingleNode(L"Process");
				pn2=pNode->selectSingleNode(L"ProcessName");
				pls=pNode->selectNodes(L"./Series");
				createSeries(pPlottext,pls,pProcess);

			}

			else if(pn2->text==_bstr_t(typeid(PlotWndPropLog).name()))
			{
				
			    PlotWndPropLog *pPlotlog =new PlotWndPropLog;
				pPlotlog->SetPlotWndArea(&(this->plotWndAreaProp));
				this->plotWndAreaProp.AddPlotWnd(pPlotlog);
				//pPlottext->type=TYPE_TEXT;
				pn2=pNode->selectSingleNode(L"WndName");
				cs.Format(_T("%S"),(LPCSTR)pn2->text);
				pPlotlog->SetName(cs);
				pn2=pNode->selectSingleNode(L"WndNameIsSet");
				if(pn2->text==(_bstr_t)(L"0"))
					pPlotlog->nameIsSet=0;
				else
					pPlotlog->nameIsSet=1;
				pn2=pNode->selectSingleNode(L"WndFrameCount");
				ifc=atoi((LPCSTR)(pn2->text));
				pPlotlog->frameCount=ifc;
				pn2=pNode->selectSingleNode(L"WndRectTLX");
				tlx=atoi((LPCSTR)(pn2->text));
				pn2=pNode->selectSingleNode(L"WndRectTLY");
				tly=atoi((LPCSTR)(pn2->text));
				pn2=pNode->selectSingleNode(L"WndRectBRX");
				brx=atoi((LPCSTR)(pn2->text));
				pn2=pNode->selectSingleNode(L"WndRectBRY");
				bry=atoi((LPCSTR)(pn2->text));
				pPlotlog->rect.SetRect(tlx,tly,brx,bry);
				pn2=pNode->selectSingleNode(L"Process");
				pn2=pNode->selectSingleNode(L"ProcessName");
				pls=pNode->selectNodes(L"./Series");
				createSeries(pPlotlog,pls,pProcess);

			}

		}

	}

	manager->Unlock();

	// create window
	if (1)
	{
		CWnd * plotWndAreaWnd;
		::AfxGetMainWnd()->SendMessage(WM_APP, CMD_GET_PLOT_WND_AREA_WND, (LPARAM)&plotWndAreaWnd);
		this->plotWndAreaProp.targetWnd = plotWndAreaWnd;

		std::vector<PlotWndProp *>::iterator iterWnd;
		for ( iterWnd = this->plotWndAreaProp.plotWndsInPlotWndAreaProp.begin();
			iterWnd!= this->plotWndAreaProp.plotWndsInPlotWndAreaProp.end();
			iterWnd++)
		{
			PlotWndProp * plotWndProp = *iterWnd;
			plotWndAreaProp.targetWnd->SendMessage(WM_APP, CMD_NEW_PLOT_WND, (LPARAM)plotWndProp);
			std::vector<SeriesProp *>::iterator iterSeries;
			for (iterSeries = plotWndProp->seriesInPlotWndProp.begin();
				iterSeries != plotWndProp->seriesInPlotWndProp.end();
				iterSeries++)
			{
				SeriesProp * seriesProp = *iterSeries;
				TextSeriesProp * textSeries = dynamic_cast<TextSeriesProp *>(seriesProp);
				auto plotWndPropBitmap = dynamic_cast<BitmapTypePlotWndProp *>(plotWndProp);
				if (textSeries && plotWndPropBitmap)
				{
					plotWndPropBitmap->AddTextPlotWnd(textSeries);
				}

				//plotWndProp->targetWnd->SendMessage(WM_APP, CMD_ADD_SERIES, (LPARAM)seriesProp);
			}
		}
	}

	if (1)
	{
		SharedObjManager * manager = SharedObjManager::Instance();
		manager->Lock();

		std::set<DebuggingProcessProp *>::iterator iterProcess;

		for (iterProcess = newProcess.begin();
			iterProcess != newProcess.end();
			iterProcess++)
		{
			DebuggingProcessProp * processProp = *iterProcess;
			wstring processName;
			processName.append(processProp->moduleName);
			processProp->IncRefCount();
			manager->allProcessObjs.insert(std::pair<wstring, DebuggingProcessProp *>(processName, processProp));
		}

		// add series
		std::set<SeriesProp *>::iterator iterSeries;
		for (iterSeries = newSeries.begin();
			iterSeries != newSeries.end();
			iterSeries++)
		{
			SeriesProp * seriesProp = *iterSeries;
			DebuggingProcessProp * processProp = seriesProp->GetProcess();
			wstring seriesName;
			seriesName.append(processProp->moduleName);
			seriesName.append(L"|");
			seriesName.append(seriesProp->name);
			seriesProp->IncRefCount();
			
			manager->allSeriesObjs.insert(std::pair<wstring, SeriesProp *>(seriesName, seriesProp));
			if (seriesProp->SerialNum() > 0)
				manager->seriesMapForReading.insert(std::pair<int, SeriesProp *>(seriesProp->SerialNum(), seriesProp));
			else
				ASSERT(FALSE);
		}

		manager->Unlock();		
	}

	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_MAIN_FRAME_ENABLE_TIMER, (LPARAM)true);

	return TRUE;
}


BOOL CDebugPlotViewerDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	
	BOOL defaultRet = CDocument::OnSaveDocument(lpszPathName);
	BSTR savePath = SysAllocString(lpszPathName);
	IXMLDOMDocument *px;
	IXMLDOMNode *pout;
	IXMLDOMElement *proot=NULL;
	IXMLDOMElement *pe=NULL;
	HRESULT hr = CoCreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&px));
	VARIANT varFileName;
    VariantInit(&varFileName);
	px->put_async(VARIANT_FALSE);  
    px->put_validateOnParse(VARIANT_FALSE);
    px->put_resolveExternals(VARIANT_FALSE);
    px->put_preserveWhiteSpace(VARIANT_TRUE);

	px->createElement(L"PlotWndAreaProp",&proot);
	px->appendChild(proot,&pout);
	
	IXMLDOMElement *pProcess=NULL;
	IXMLDOMElement *ppro=NULL;
	

	std::vector<PlotWndProp *>::iterator iterPoltWnd;
	for (iterPoltWnd = this->plotWndAreaProp.plotWndsInPlotWndAreaProp.begin();
		iterPoltWnd != this->plotWndAreaProp.plotWndsInPlotWndAreaProp.end();
		iterPoltWnd++)
	{
		PlotWndProp * plotWndProp = (*iterPoltWnd);
		if (plotWndProp->seriesInPlotWndProp.size() == 0)
			continue;

		CreateAndAddElementNode(px, L"PlotWnd", L"\n\t", proot, &pe);
		CreateAndAddElementNode(px, L"Process", L"\n\t\t", pe, &pProcess);
		pProcess->put_text((_bstr_t)(*(*iterPoltWnd)->seriesInPlotWndProp.begin())->GetProcess()->moduleName);
		CreateAndAddElementNode(px, L"ProcessName", L"\n\t\t", pe,&ppro);
		ppro->put_text((_bstr_t)(*(*iterPoltWnd)->seriesInPlotWndProp.begin())->GetProcess()->pid);
		
		(*iterPoltWnd)->CreateElementPlot(px,pe);
		pe->Release();
		
    }
	
	V_VT(&varFileName)   = VT_BSTR;
	V_BSTR(&varFileName) = savePath;
	hr=px->save(varFileName);
	SysFreeString(savePath);

	// XML TODO
	//...

	return defaultRet;
}


void CDebugPlotViewerDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class

	CDocument::OnCloseDocument();
}

void CDebugPlotViewerDoc::ResetDocumentState()
{
	std::vector<PlotWndProp *> plotWnds;
	
	std::vector<PlotWndProp *>::iterator iter;
	
	for (iter = this->plotWndAreaProp.plotWndsInPlotWndAreaProp.begin();
		iter != this->plotWndAreaProp.plotWndsInPlotWndAreaProp.end();
		iter++)
	{
		plotWnds.push_back(*iter);
	}

	for (iter = plotWnds.begin();
		iter != plotWnds.end();
		iter++)
	{
		(*iter)->targetWnd->SendMessage(WM_CLOSE, 0, 0);
	}

	ASSERT(this->plotWndAreaProp.plotWndsInPlotWndAreaProp.size() == 0);
}


#include "stdafx.h"
#include <algorithm>
#include "HelperFunc.h"
#include "DocumentSaverLoader.h"

using namespace std;
using namespace SoraDbgPlot::Task;


DocumentSaverLoader::DocumentSaverLoader()
{
}

DocumentSaverLoader::~DocumentSaverLoader()
{

}

void DocumentSaverLoader::Load(const CString & filepath)
{
	auto SThis = dynamic_pointer_cast<DocumentSaverLoader, AsyncObject>(shared_from_this());
	this->DoLater([SThis, filepath](){
		// XML TODO
		//...
		VARIANT var;
		VariantInit(&var);
		BSTR bstr = SysAllocString(filepath);
		V_VT(&var)   = VT_BSTR;
		V_BSTR(&var) = bstr;
		MSXML2::IXMLDOMDocumentPtr pXMLDom;
		HRESULT hr = pXMLDom.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

		pXMLDom->async = VARIANT_FALSE;
		pXMLDom->validateOnParse = VARIANT_FALSE;
		pXMLDom->resolveExternals = VARIANT_FALSE;
		pXMLDom->load(var);

		MSXML2::IXMLDOMNodePtr pNode=NULL ; 
		MSXML2::IXMLDOMNodePtr pn2=NULL ; 
		MSXML2::IXMLDOMNodeListPtr pls=NULL;
		BSTR str1=SysAllocString(L" ");
		
		MSXML2::IXMLDOMNodeListPtr plotWndNodes = pXMLDom->selectNodes(L"//PlotWnd");

		//if (plotWndNodes)
		//{
		//	for (long i = 0; i < pnl->length; i++)
		//	{
		//		pNode=pnl->item[i];			

		//	}
		//}
	});
}

void DocumentSaverLoader::Save(const CString & filepath)
{
	auto SThis = dynamic_pointer_cast<DocumentSaverLoader, AsyncObject>(shared_from_this());
	this->DoLater([SThis, filepath](){

		::CoInitialize(NULL);

		BSTR savePath = SysAllocString(filepath);
		IXMLDOMDocument *pXmlDoc;
		IXMLDOMNode *pout;
		IXMLDOMElement *pRootElement=NULL;
		IXMLDOMElement *pe=NULL;

		HRESULT hr = CoCreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pXmlDoc));

		assert(hr == S_OK);

		VARIANT varFileName;
		VariantInit(&varFileName);

		// settings
		pXmlDoc->put_async(VARIANT_FALSE);  
		pXmlDoc->put_validateOnParse(VARIANT_FALSE);
		pXmlDoc->put_resolveExternals(VARIANT_FALSE);
		pXmlDoc->put_preserveWhiteSpace(VARIANT_TRUE);

		// create and add root element
		pXmlDoc->createElement(L"PlotWndAreaProp",&pRootElement);
		pXmlDoc->appendChild(pRootElement,&pout);

		//IXMLDOMElement *pPlotWndElement = NULL;

		for (auto iter = SThis->_mapProcessPlotwnd.begin();
			iter != SThis->_mapProcessPlotwnd.end();
			++iter)
		{
			auto plotWnd = iter->second;
			plotWnd->CreateXmlElement(pXmlDoc, pRootElement);
		}

		V_VT(&varFileName)   = VT_BSTR;
		V_BSTR(&varFileName) = savePath;
		hr = pXmlDoc->save(varFileName);
		SysFreeString(savePath);

		::CoUninitialize();

	});
}

void DocumentSaverLoader::AddPlotWnd(std::shared_ptr<PlotWindowOpened> plotWnd, std::shared_ptr<ProcessOpened> process)
{
	auto SThis = dynamic_pointer_cast<DocumentSaverLoader, AsyncObject>(shared_from_this());
	this->DoLater([SThis, plotWnd, process](){
		SThis->_mapProcessPlotwnd.insert(make_pair<shared_ptr<ProcessOpened>, shared_ptr<PlotWindowOpened> >(process, plotWnd));
	});
}

void DocumentSaverLoader::RemovePlotWnd(std::shared_ptr<PlotWindowOpened> plotWnd)
{
	auto SThis = dynamic_pointer_cast<DocumentSaverLoader, AsyncObject>(shared_from_this());
	this->DoLater([SThis, plotWnd](){
		for (auto iter = SThis->_mapProcessPlotwnd.begin();
			iter != SThis->_mapProcessPlotwnd.end();)
		{
			if (iter->second == plotWnd)
			{
				SThis->_mapProcessPlotwnd.erase(iter++);
			}
			else
			{
				++iter;
			}
		}
	});
}

void DocumentSaverLoader::ClearPlotWindows()
{
	auto SThis = dynamic_pointer_cast<DocumentSaverLoader, AsyncObject>(shared_from_this());
	this->DoLater([SThis](){
		SThis->CloseAllPlotWindow();
	});	
}

void DocumentSaverLoader::Clear()
{
	EventSaveComplete.Reset();
}

void DocumentSaverLoader::CloseAllPlotWindow()
{
	for (auto iter = this->_mapProcessPlotwnd.begin();
		iter != this->_mapProcessPlotwnd.end();
		++iter)
	{
		auto plotWnd = iter->second;
		plotWnd->Close();
	}
}


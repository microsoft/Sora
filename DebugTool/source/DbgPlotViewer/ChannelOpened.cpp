#include "stdafx.h"
#include <memory>
#include "ChannelOpened.h"
#include "SharedChannel.h"
#include "ChannelProperty.h"
#include "HelperFunc.h"

using namespace std;
using namespace SoraDbgPlot::Task;
using namespace SoraDbgPlot::SharedObj;

ChannelOpened::ChannelOpened() :
	_color(RGB(200, 200, 200))
{
	_spectrumSize = 1;
	_openCount = 0;
	_bAttatched = false;
}

ChannelOpened::~ChannelOpened()
{
	_sharedChannel.reset();
}

void ChannelOpened::AttatchSharedChannelSync(shared_ptr<SoraDbgPlot::SharedObj::SharedChannel> sch)
{
	auto shared_me = dynamic_pointer_cast<ChannelOpened, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([shared_me, sch](){
		if (sch)
		{
			if (shared_me->_sharedChannel)
			{
				shared_me->_sharedChannel->StrategyNewData.UnSet();
				shared_me->_sharedChannel.reset();
			}

			shared_me->_sharedChannel = sch;

			shared_me->_id = sch->Id();
			shared_me->_pid = sch->Pid();
			shared_me->_spectrumSize = sch->SpectrumDataSize();
			shared_me->_type = sch->Type();
			shared_me->_name = sch->Name();

			shared_me->_bAttatched = true;

			shared_me->ClearData();
		}
	});
}

void ChannelOpened::DeattatchSharedChannelSync()
{
	auto SThis = dynamic_pointer_cast<ChannelOpened, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([SThis](){
		SThis->_sharedChannel.reset();
		SThis->_pid = -1;
		SThis->_bAttatched = false;
	});
}

bool ChannelOpened::IsAttatched()
{
	return _bAttatched;
}

int ChannelOpened::Id()
{
	int ret;
	auto shared_me = dynamic_pointer_cast<ChannelOpened, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([this, &ret](){
		ret = _id;
	});
	return ret;
}

int ChannelOpened::Pid()
{
	int ret;
	TaskQueue()->DoTask([this, &ret](){
		ret = _pid;
	});
	return ret;
}

int ChannelOpened::SpectrumDataSize()
{
	int ret;
	TaskQueue()->DoTask([this, &ret](){
		ret = _spectrumSize;
	});
	return ret;
}

wstring ChannelOpened::Name()
{
	wstring ret;
	TaskQueue()->DoTask([this, &ret](){
		ret = _name;
	});
	return std::move(ret);
}

ChannelType ChannelOpened::Type()
{
	ChannelType ret;
	TaskQueue()->DoTask([this, &ret](){
		ret = _type;
	});
	return ret;
}

void ChannelOpened::SetColor(COLORREF color)
{
	auto shared_me = dynamic_pointer_cast<ChannelOpened, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([shared_me, color](){
		shared_me->_color = color;
	});
}

COLORREF ChannelOpened::Color()
{
	COLORREF color;
	auto shared_me = dynamic_pointer_cast<ChannelOpened, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([shared_me, &color](){
		color = shared_me->_color;
	});

	return color;
}

void ChannelOpened::Release()
{
	auto SThis = dynamic_pointer_cast<ChannelOpened, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([SThis](){
		SThis->TaskFunc_Release();
	});	
}

void ChannelOpened::TaskFunc_Release()
{
	if (_sharedChannel)
	{
		_sharedChannel.reset();
		_pid = -1;
	}
}


std::shared_ptr<TaskSimple> ChannelOpened::TaskUpdateData(std::shared_ptr<bool> updated)
{
	auto SThis = dynamic_pointer_cast<ChannelOpened, AsyncObject>(shared_from_this());

	shared_ptr<TaskSimple> task;

	task = make_shared<TaskSimple>(TaskQueue(), [SThis, updated](){

		if (updated.get())
			*updated = false;

		if (SThis->_sharedChannel)
		{
			auto dataList = SThis->_sharedChannel->CheckOut();
			while(dataList->size() > 0)
			{
				if (updated.get())
					*updated = true;

				auto iter = dataList->front();
				SThis->WriteData(iter.ptr, iter.length);
				dataList->pop_front();
			}
		}
		//TRACE1("ChannelOpened::TaskUpdateData [%d]\n", SThis->Id());
	});

	return task;
}

void ChannelOpened::WriteData(const char * data, size_t length)
{
	// do nothint
}

std::shared_ptr<BaseProperty> ChannelOpened::CreatePropertyPage()
{	
	shared_ptr<ChannelProperty> propertyPage;

	auto SThis = dynamic_pointer_cast<ChannelOpened, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([SThis, &propertyPage](){

		wstring typeName = SThis->GetTypeName();

		propertyPage = make_shared<ChannelProperty>(typeName, SThis->_name, SThis->_color);
		auto SThis2 = SThis;
		propertyPage->EventColor.Subscribe([SThis2](const void * sender, const COLORREF & color){
			auto SThis3 = SThis2;
			auto color2 = color;
			SThis3->DoLater([SThis3, color2](){
				SThis3->_color = color2;
				SThis3->OnColorUpdated();
			});
		});
	});

	return propertyPage;
}

void ChannelOpened::OnColorUpdated()
{
	// do nothing
}

void ChannelOpened::GetRect(CRect & rect)
{
	rect.SetRectEmpty();
}

HRESULT ChannelOpened::CreateXmlElement(IXMLDOMDocument *pDom, IXMLDOMElement *pParent)
{
	IXMLDOMElement *pElement=NULL;
	CreateAndAddElementNode(pDom, L"Channel", L"\n\t\t", pParent, &pElement);

	this->AppendXmlProperty(pDom, pElement);
	CreateAndAddTextNode(pDom, L"\n\t\t", pElement);

	return S_OK;
}

HRESULT ChannelOpened::AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent)
{
	auto SThis = dynamic_pointer_cast<ChannelOpened, AsyncObject>(shared_from_this());
	this->DoNow([SThis, pDom, pParent](){

		IXMLDOMElement *pElement = NULL;

		CreateAndAddElementNode(pDom, L"SName", L"\n\t\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_name.c_str());

		CreateAndAddElementNode(pDom, L"SType", L"\n\t\t\t", pParent, &pElement);
		pElement->put_text(_bstr_t(typeid(*SThis).name()));

		CreateAndAddElementNode(pDom, L"color", L"\n\t\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_color);
	});

	return S_OK;
}

HRESULT ChannelOpened::LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement)
{
	CString cs;
	MSXML2::IXMLDOMNodePtr pNode;

	pNode = pElement->selectSingleNode(L"SName");
	if (pNode == 0)
	{
		this->_name = L"invalid name";
	}
	else
	{
		cs.Format(_T("%S"),(LPCSTR)pNode->text);
		this->_name = cs;
	}

	pNode = pElement->selectSingleNode(L"color");
	if (pNode != 0)
	{
		this->_color = (atol)((LPCSTR)pNode->text);
	}

	return S_OK;
}

bool ChannelOpened::Export(const CString &, bool bAll)
{
	//::AfxMessageBox(L"not implemented");
	return false;
}

void ChannelOpened::Clear()
{
	// do nothing
}

void ChannelOpened::SetOpenState(bool bOpened)
{
	if (bOpened)
		::InterlockedIncrement(&_openCount);
	else
		::InterlockedDecrement(&_openCount);
}

bool ChannelOpened::GetOpenState()
{
	return _openCount > 0;
}

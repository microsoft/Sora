#include "stdafx.h"
#include <sstream>
#include "PlotWindowOpened.h"
#include "HelperFunc.h"

using namespace std;
using namespace SoraDbgPlot::Task;

PlotWindowOpened::PlotWindowOpened(std::shared_ptr<ProcessOpened> process) :
	_processOpened(process),
	_bNameIsSet(false),
	_plotWnd(0),
	_isPlay(true),
	_isPlayLastState(true)
{
	_processName = process->Name();
	_pid = process->Pid();
}

PlotWindowOpened::PlotWindowOpened(std::shared_ptr<ProcessOpened> process, const wstring & name, bool bNameIsSet, const CRect & rect) :
	_processOpened(process),
	_name(name),
	_bNameIsSet(bNameIsSet),
	_rect(rect),
	_pid(-1)
{}

void PlotWindowOpened::SetSnapGridSize(int size)
{
	auto shared_me = dynamic_pointer_cast<PlotWindowOpened, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([shared_me, size](){
		shared_me->_plotWnd->SetSnapGridSize(size);
	});
}

void PlotWindowOpened::SetName(const std::wstring & name)
{
	auto shared_me = dynamic_pointer_cast<PlotWindowOpened, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([shared_me, name](){
		shared_me->_name = move(name);
		shared_me->_bNameIsSet = name != L"";
		shared_me->UpdateWndCaption();
	});
}

void PlotWindowOpened::SetRect(const CRect & rect)
{
	auto shared_me = dynamic_pointer_cast<PlotWindowOpened, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([shared_me, rect](){
		shared_me->_rect = rect;
		if (shared_me->_bWndValid)
		{
			shared_me->_plotWnd->MoveWindowAsync(rect);
		}
	});
}

CPlotWnd * PlotWindowOpened::GetPlotWnd()
{
	assert(_plotWnd == 0);

	_plotWnd = this->CreatePlotWnd();
	this->AddPlotWndStrategy(_plotWnd);
	_bWndValid = true;

	return _plotWnd;
}

void PlotWindowOpened::AddPlotWndStrategy(CPlotWnd * plotWnd)
{
	// todo
	// 1, set caption
	// 2, set grid size
	// 3. updatepropertypage

	auto SThis = dynamic_pointer_cast<PlotWindowOpened, AsyncObject>(shared_from_this());

	plotWnd->EventBringToFront.Subscribe([SThis](const void * sender, const CPlotWnd::BringToFrontEvent & e) {
		//this->UpdatePropertyPage();
		SThis->EventBringToFront.Raise(SThis.get(), SThis);
		SThis->EventProcessSelected.Raise(SThis.get(), SThis->_processOpened);
		
	});

	plotWnd->EventMove.Subscribe([SThis](const void * sender, const CPlotWnd::MoveEvent & e){
		auto SThis2 = SThis;
		SThis2->TaskQueue()->QueueTask([SThis2, e](){
			SThis2->_rect = e.rect;
		});
	});

	plotWnd->EventClosed.Subscribe([SThis](const void * sender, const CPlotWnd::NullEvent & e){
		auto SThis2 = SThis;

		auto taskClear = [SThis2](){

			CPlotWnd * wndToDelete = SThis2->_plotWnd;

			auto taskDelete = [wndToDelete](){
				delete wndToDelete;
			};

			SThis2->DoLater(taskDelete, taskDelete);

			SThis2->_plotWnd = 0;

			for (auto iterCh = SThis2->_chMap.begin(); iterCh != SThis2->_chMap.end(); ++iterCh)
			{
				auto channel = *(iterCh);
				channel->Clear();
				PlotWindowOpened::CloseChannelEvent e;
				e._channel = channel;
				channel->SetOpenState(false);
				SThis2->EventCloseChannel.Raise(SThis2.get(), e);
			}

			SThis2->_chMap.clear();
			SThis2->_newChSet.clear();

			SThis2->EventPlotWndClosed.Raise(SThis2.get(), SThis2);
			SThis2->EventBringToFront.Reset();
			SThis2->EventAddChannelRequest.Reset();
			SThis2->EventCloseChannel.Reset();
			SThis2->EventNameChanged.Reset();
			SThis2->EventPropertyPageChanged.Reset();
			SThis2->EventRectChanged.Reset();
			SThis2->EventPlotWndClosed.Reset();
		};

		SThis2->DoLater(taskClear, taskClear);
	});

	plotWnd->EventCreated.Subscribe([SThis](const void * sender, const bool & dummy){
		auto SThis2 = SThis;
		SThis2->TaskQueue()->QueueTask([SThis2](){
			SThis2->UpdateWndCaption();
			//SThis2->EventBringToFront.Raise(SThis2.get(), SThis2);
		});
	});
}

void PlotWindowOpened::AddChannelOpened(shared_ptr<ChannelOpened> channel, const CRect & rect)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpened, AsyncObject>(shared_from_this());

	TaskQueue()->QueueTask([SThis, channel, rect](){

		int chId = channel->Id();

		auto iterCh = SThis->_chMap.find(channel);
		if (iterCh == SThis->_chMap.end())
		{
			SThis->_chMap.insert(channel);
		}

		auto iterNewCh = SThis->_newChSet.find(chId);
		if (iterNewCh == SThis->_newChSet.end())
		{
			SThis->_newChSet.insert(chId);
		}

		SThis->UpdateWndCaption();

		SThis->OnChannelAdded(channel, rect);
		channel->SetOpenState(true);
	});
}

void PlotWindowOpened::OnChannelAdded(std::shared_ptr<ChannelOpened> channel, const CRect & rect)
{
	// do nothing
}

void PlotWindowOpened::OnChannelClosed(std::shared_ptr<ChannelOpened>)
{
	// do nothing
}

void PlotWindowOpened::RemoveChannelOpened(std::shared_ptr<ChannelOpened> channel)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpened, AsyncObject>(shared_from_this());

	channel->Clear();

	TaskQueue()->QueueTask([SThis, channel](){	

		int chId = channel->Id();

		auto iterCh = SThis->_chMap.find(channel);
		if (iterCh != SThis->_chMap.end())
		{
			SThis->_chMap.erase(iterCh);
			SThis->OnChannelClosed(channel);
			channel->SetOpenState(false);
		}

		auto iterNewCh = SThis->_newChSet.find(chId);
		if (iterNewCh != SThis->_newChSet.end())
		{
			SThis->_newChSet.erase(chId);
		}

		if (SThis->_chMap.size() == 0)
		{
			HWND hWnd = SThis->_plotWnd->m_hWnd;
			if (hWnd)
				::PostMessage(hWnd, WM_CLOSE, 0, 0);
			SThis->_bWndValid = false;
		}
		else
		{
			SThis->UpdateWndCaption();
		}

	});
}


std::shared_ptr<SoraDbgPlot::Task::TaskSimple> PlotWindowOpened::TaskUpdate()
{
	return make_shared<TaskSimple>(TaskQueue(), [this](){
		TRACE1("plot: %x\n", this);
	});
}

void PlotWindowOpened::GetRect(CRect & rect)
{
	TaskQueue()->DoTask([&rect, this]() {
		rect = this->_rect;
	});
}

void PlotWindowOpened::SetRectSync(const CRect & rect)
{
	TaskQueue()->DoTask([&rect, this](){
		this->_rect = rect;
	});
}

void PlotWindowOpened::ProcessAttatchDetatch(std::shared_ptr<ProcessOpened> process, bool bIsAttatch)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpened, AsyncObject>(shared_from_this());
	this->DoLater([SThis, process, bIsAttatch](){
		if (SThis->_processOpened == process)
		{
			SThis->_pid = SThis->_processOpened->Pid();
			SThis->UpdateWndCaption();
			if (bIsAttatch)
			{
				SThis->OnProcessAttatched();
				SThis->PlayPauseToLatest(true);
			}
		}
	});	
}

void PlotWindowOpened::SetPlayPause(bool bPlay, bool seekToLatest)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpened, AsyncObject>(shared_from_this());
	this->DoLater([SThis, bPlay, seekToLatest](){
		SThis->_isPlay = bPlay;
		if (bPlay && seekToLatest)
			SThis->SeekToLatest();
	});
}

void PlotWindowOpened::UpdateWndCaption()
{
	if (_plotWnd == 0)
		return;

	if (_bNameIsSet)
	{
		_plotWnd->SetCaption(_name.c_str());
	}
	else
	{
		wstring name;
		wstringstream ss;
		ss << _pid;

		name.append(_processName.c_str()).append(L"(").append(ss.str()).append(L")").append(L":");
		for (auto iterCh = _chMap.begin(); iterCh != _chMap.end(); ++iterCh)
		{
			auto channel = *iterCh;
			name.append(L"[").append(channel->Name().c_str()).append(L"]");
		}

		_plotWnd->SetCaption(name.c_str());
	}
}

void PlotWindowOpened::Highlight(bool bHighlight)
{
	if (_plotWnd)
	{
		_plotWnd->HighLight(bHighlight);
	}
}

void PlotWindowOpened::RequestAddChannel(std::shared_ptr<ProcessOpened> process, shared_ptr<ChannelOpened> channel, CPoint pointIn)
{
	AddChannelEvent ee;
	ee._plotWnd = dynamic_pointer_cast<PlotWindowOpened, AsyncObject>(shared_from_this());
	ee._channel = channel;
	ee._rect.left = pointIn.x;
	ee._rect.top = pointIn.y;
	ee._rect.right = ee._rect.left + 100;
	ee._rect.bottom = ee._rect.top + 100;
	EventAddChannelRequest.Raise(this, ee);
}

void PlotWindowOpened::Close()
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpened, AsyncObject>(shared_from_this());
	
	this->DoLater([SThis](){
		if (SThis->_plotWnd)
		{
			HWND hWnd = SThis->_plotWnd->m_hWnd;
			if (hWnd)
				::PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
	});
}

void PlotWindowOpened::SeekToLatest()
{

}

HRESULT PlotWindowOpened::CreateXmlElement(IXMLDOMDocument *pDom, IXMLDOMElement *pParent)
{
	IXMLDOMElement *pElement=NULL;
	CreateAndAddElementNode(pDom, L"PlotWnd", L"\n\t", pParent, &pElement);

	this->AppendXmlProperty(pDom, pElement);

	shared_ptr<ProcessOpened> process;
	vector<shared_ptr<ChannelOpened> > channels;
	auto SThis = dynamic_pointer_cast<PlotWindowOpened, AsyncObject>(shared_from_this());
	this->DoNow([SThis, &process, &channels]() mutable {
		process = SThis->_processOpened;
		for (auto iter = SThis->_chMap.begin();
			iter != SThis->_chMap.end();
			++iter)
		{
			auto channel = *(iter);
			channels.push_back(channel);
		}
	});

	process->CreateXmlElement(pDom, pElement);

	for (auto iter = channels.begin(); iter != channels.end(); ++iter)
	{
		(*iter)->CreateXmlElement(pDom, pElement);
	}

	CreateAndAddTextNode(pDom, L"\n\t", pElement);

	return S_OK;
}

HRESULT PlotWindowOpened::LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement)
{
	CString cs;
	MSXML2::IXMLDOMNodePtr pNode;

	pNode = pElement->selectSingleNode(L"WndName");
	if (pNode != 0)
	{
		cs.Format(_T("%S"),(LPCSTR)pNode->text);
		this->_name = cs;
	}
	else
	{
		this->_name = L"invalid name";
	}

	pNode = pElement->selectSingleNode(L"WndNameIsSet");
	if (pNode != 0)
	{
		if(pNode->text == (_bstr_t)(L"0"))
			this->_bNameIsSet = false;
		else
			this->_bNameIsSet = true;
	}

	pNode = pElement->selectSingleNode(L"WndRectTLX");
	if (pNode == 0)
		goto RET;
	int tlx = atoi((LPCSTR)(pNode->text));

	pNode = pElement->selectSingleNode(L"WndRectTLY");
	if (pNode == 0)
		goto RET;
	int tly = atoi((LPCSTR)(pNode->text));

	pNode = pElement->selectSingleNode(L"WndRectBRX");
	if (pNode == 0)
		goto RET;
	int brx = atoi((LPCSTR)(pNode->text));

	pNode = pElement->selectSingleNode(L"WndRectBRY");
	if (pNode == 0)
		goto RET;
	int bry = atoi((LPCSTR)(pNode->text));

	this->_rect.SetRect(tlx,tly,brx,bry);

RET:
	return S_OK;
}

HRESULT PlotWindowOpened::AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpened, AsyncObject>(shared_from_this());
	this->DoNow([SThis, pDom, pParent](){

		IXMLDOMElement *pElement = NULL;

		CreateAndAddElementNode(pDom, L"WndName", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_name.c_str());

		CreateAndAddElementNode(pDom, L"WndType", L"\n\t\t", pParent, &pElement);
		pElement->put_text(_bstr_t(typeid(*SThis).name()));

		CreateAndAddElementNode(pDom, L"WndNameIsSet", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_bNameIsSet);

		CreateAndAddElementNode(pDom, L"WndRectTLX", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_rect.TopLeft().x);

		CreateAndAddElementNode(pDom, L"WndRectTLY", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_rect.TopLeft().y);

		CreateAndAddElementNode(pDom, L"WndRectBRX", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_rect.BottomRight().x);

		CreateAndAddElementNode(pDom, L"WndRectBRY", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_rect.BottomRight().y);
	});

	return S_OK;
}

void PlotWindowOpened::OnProcessAttatched()
{
	// do nothing
}

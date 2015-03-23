#include "stdafx.h"
#include "ControlPanelDriver.h"

using namespace std;

ControlPanelDriver::~ControlPanelDriver()
{
}

std::shared_ptr<CWnd> ControlPanelDriver::GetControlWnd()
{
	shared_ptr<CWnd> ret;
	auto SThis = dynamic_pointer_cast<ControlPanelDriver, AsyncObject>(shared_from_this());
	this->DoNow([SThis, &ret](){
		if (SThis->_controlPanelList == 0)
		{
			SThis->_controlPanelList = make_shared<ControlPanelList>();
		}
		
		ret = SThis->_controlPanelList;
	});

	return ret;
}

void ControlPanelDriver::AddProcessOpened(std::shared_ptr<ProcessOpened> process)
{
	auto SThis = dynamic_pointer_cast<ControlPanelDriver, AsyncObject>(shared_from_this());
	this->DoLater([SThis, process](){

		auto process2 = process;

		auto iter = SThis->_processWndMap.find(process);
		if (iter == SThis->_processWndMap.end())
		{
			CString caption;
			caption.Format(L"%s(%d)", process2->Name().c_str(), process2->Pid());

			auto wnd = make_shared<ControlPanelWnd>(caption);
			
			wnd->EventPlayPause.Subscribe([process2](const void * sender, const bool & bPlayPause){
				process2->TraceBarPlayPause(bPlayPause);
			});

			wnd->EventSingleStep.Subscribe([process2](const void * sender, const bool & dummy){
				process2->TraceBarSingleStep();
			});

			wnd->EventTraceBarSizeChanged.Subscribe([process2](const void * sender, const CRect & rect){
				process2->SetBitmapRect(rect);
			});

			wnd->EventTraceBarWheel.Subscribe([process2](const void * sender, const bool & isUp){
				process2->TrackBarWheel(isUp);
			});

			wnd->EventTraceBarSeek.Subscribe([process2](const void * sender, const double & pos){
				process2->TrackBarSeek(pos);
			});

			auto SThis2 = SThis;
			wnd->EventClosed.Subscribe([SThis2, process2](const void * sender, const bool & e){

				auto SThis = SThis2;
				auto process = process2;
				SThis->DoLater([SThis, process](){
					auto iter = SThis->_processWndMap.find(process);
					assert(process);
					auto wnd = iter->second;
					SThis->_processWndMap.erase(process);
					SThis->_processIsUpdating.erase(process);
					if (SThis->_controlPanelList)
					{
						SThis->_controlPanelList->RemoveControlPanelWnd(wnd);
					}	
				});
			});

			SThis->_processWndMap.insert(make_pair(process, wnd));
			SThis->_processIsUpdating.insert(make_pair(process, false));

			if (SThis->_controlPanelList)
			{
				SThis->_controlPanelList->AddControlPanelWnd(wnd);
			}
			TRACE1("Process Opened activated %d\n", process->Pid());
		}
	});
}

void ControlPanelDriver::RemoveProcess(std::shared_ptr<ProcessOpened> process)
{
	auto SThis = dynamic_pointer_cast<ControlPanelDriver, AsyncObject>(shared_from_this());
	this->DoLater([SThis, process](){
		auto iter = SThis->_processWndMap.find(process);

		if (iter == SThis->_processWndMap.end())
		{
			assert(false);
			return;
		}

		auto wnd = iter->second;

		HWND hWnd = wnd->m_hWnd;
		if (hWnd)
			::PostMessage(hWnd, WM_CLOSE, 0, 0);
	});
}

void ControlPanelDriver::Update()
{
	auto SThis = dynamic_pointer_cast<ControlPanelDriver, AsyncObject>(shared_from_this());
	this->DoLater([SThis](){
		for (auto iter = SThis->_processWndMap.begin(); iter != SThis->_processWndMap.end(); ++iter)
		{
			auto process = iter->first;
			auto wnd = iter->second;
			bool bProcessUpdating = SThis->_processIsUpdating.find(process)->second;
			if (bProcessUpdating)
				continue;

			auto SThis2 = SThis;
			process->GenUpdateData([SThis2](Bitmap * bitmap, bool isRawDataBufInUse, shared_ptr<ProcessOpened> process){
				auto SThis3 = SThis2;
				SThis2->DoLater([SThis3, bitmap, isRawDataBufInUse, process](){
					auto iter = SThis3->_processWndMap.find(process);
					if (iter != SThis3->_processWndMap.end())
					{
						auto wnd = iter->second;
						wnd->SetBitmap(bitmap);
						wnd->EnableButton(isRawDataBufInUse);
					}

					auto iterUpdate = SThis3->_processIsUpdating.find(process);
					if (iterUpdate != SThis3->_processIsUpdating.end())
						iterUpdate->second = false;
				});
			});
		}
	}, [](){});
}

void ControlPanelDriver::Clear()
{

}

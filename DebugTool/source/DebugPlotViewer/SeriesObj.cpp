#include "stdafx.h"
#include <algorithm>
#include <string>
#include "AppSettings.h"
#include "SeriesObj.h"
#include "PlotWnd.h"
#include "SubPlotWnd.h"
#include "SubBitmapPlotWnd.h"
#include "SubTextPlotWnd.h"
#include "AppMessage.h"
#include "LineSeriesProperty.h"
#include "LinePlotWndProperty.h"
#include "DotsSeriesProperty.h"
#include "DotsPlotWndProperty.h"
#include "TextSeriesProperty.h"
#include "TextPlotWndProperty.h"
#include "LogSeriesProperty.h"
#include "LogPlotWndProperty.h"
#include "PlotWndAreaProperty.h"
#include "SpectrumSeriesProperty.h"
#include "SpectrumPlotWndProperty.h"
#include "SharedStruct.h"
#include "HelperFunc.h"
#include "Debug.h"
#include "SubLogPlotWnd.h"
#include "LogWithFileBackup.h"

/***********************************

Base Obj

************************************/

	
ObjProp::ObjProp()
{
	//this->objType = TYPE_UNKNOWN;
	this->refCount = 0;
}

ObjProp::~ObjProp()
{
	CWnd * wnd = ::AfxGetMainWnd();
	if (wnd)
		wnd->SendMessage(WM_APP, CMD_NOTIFY_RELEASED, (LPARAM)this);
}

//BaseProperty * ObjProp::GetPropertyPage()
//{
//	return 0;
//}

void ObjProp::UpdatePropertyPage()
{
	BaseProperty * property = this->GetPropertyPage();
	::AfxGetMainWnd()->PostMessage(WM_APP, CMD_CHANGE_PROPERTY, (LPARAM)property);
}

void ObjProp::IncRefCount()
{
	InterlockedIncrement(&this->refCount);
}

void ObjProp::DecRefCount()
{
	InterlockedDecrement(&this->refCount);
	if (this->refCount == 0)
		delete this;
}

int ObjProp::GetRefCount()
{
	return refCount;
}

/***********************************

Base Series

************************************/
SeriesProp::SeriesProp()
{
	plotWnd = 0;
	process = 0;
	pWnd = 0;
	smSeriesInfo = 0;
	alive = false;
	isActive = false;
	isOpened = false;
	lastReplayIdx = -1;
	_writeEnabled = true;

	this->replayBuffer.resize(DebuggingProcessProp::REPLAY_BUF_LEN);
	for (int i = 0; i < DebuggingProcessProp::REPLAY_BUF_LEN; i++)
	{
		replayBuffer[i].data = new char[::SettingGetSnapShotBufferSize()];
		replayBuffer[i].bufLen = ::SettingGetSnapShotBufferSize();
	}

	::InitializeCriticalSection(&csSeries);
	_dataQueue = new SoraDbgPlot::DataQueue::DataQueue(1024);
}

SeriesProp::~SeriesProp()
{
	delete _dataQueue;
	::DeleteCriticalSection(&csSeries);

	if (process != 0)
	{
		process->DecRefCount();
	}

	this->FreeSharedMem();
}

void SeriesProp::SetPlotWndProp(PlotWndProp * prop)
{
	this->plotWnd = prop;
}

unsigned int SeriesProp::SerialNum()
{
	size_t serialNum = 0;
	_lockSmInfo.Lock();

	if (smSeriesInfo)
	{
		SharedSeriesInfo * sharedSeriesInfo = (SharedSeriesInfo *)smSeriesInfo->GetAddress();
		serialNum = sharedSeriesInfo->serialNum;
	}

	_lockSmInfo.Unlock();

	return serialNum;
}

void SeriesProp::WriteData(const void * ptr, size_t length)
{
	this->EventWriteData.Raise(this, true);

	this->_dataQueue->Write((const char *)ptr, length);

	this->EventWriteData.Raise(this, false);
}


void SeriesProp::ReadData(const function<void(const char * ptr, const size_t length)> & f)
{
	this->_dataQueue->Read(f);
}

PlotWndProp * SeriesProp::GetPlotWndProp()
{
	return this->plotWnd;
}

void SeriesProp::Close()
{
	if (this->isOpened)
	{
		this->isOpened = false;
		this->process->openCount--;
		::AfxGetMainWnd()->PostMessage(WM_APP, CMD_UPDATE_CONTROLLER, 0);
		if (this->plotWnd)
			this->plotWnd->ForceUpdateGraph();
	}
}

void SeriesProp::NotifyClosed()
{
	if (this->isOpened)
	{
		this->isOpened = false;
		this->process->openCount--;
		::AfxGetMainWnd()->PostMessage(WM_APP, CMD_UPDATE_CONTROLLER, 0);
	}
	this->pWnd = 0;
}

void SeriesProp::FreeSharedMem()
{
	_lockSmInfo.Lock();
	if (this->smSeriesInfo)
	{
		ShareMem::FreeShareMem(this->smSeriesInfo);
		this->smSeriesInfo = 0;
		OnSmInfoRemovedMe();
	}
	_lockSmInfo.Unlock();
}


void SeriesProp::SetSmInfo(ShareMem * sm)
{
	_lockSmInfo.Lock();

	this->smSeriesInfo = sm;

	OnSmInfoSetMe();

	_lockSmInfo.Unlock();
}

void SeriesProp::OnSmInfoSetMe()
{
	this->_dataQueue->Clear();

	this->ClearData();

	auto sharedSeriesInfo = (SharedSeriesInfo *)this->smSeriesInfo->GetAddress();
	OnSmInfoSet(sharedSeriesInfo);
}

void SeriesProp::OnSmInfoSet(SharedSeriesInfo * sharedSeriesInfo)
{
	// do nothing
}

void SeriesProp::OnSmInfoRemovedMe()
{
	OnSmInfoRemoved();
}

void SeriesProp::OnSmInfoRemoved()
{
	// do nothing
}

void SeriesProp::EraseFromProcess()
{
	ASSERT(this->process);
	this->process->RemoveSeries(this);
	this->process->DecRefCount();
	this->process = 0;
}

void SeriesProp::EnableWrite(bool enable)
{
	_writeEnabled = enable;
}

bool SeriesProp::IsWriteEnabled()
{
	return _writeEnabled;
}

CWnd * SeriesProp::GetTargetWnd()
{
	return this->pWnd;
}

void SeriesProp::SetTargetWnd(CWnd * pWnd)
{
	this->pWnd = pWnd;
}

void SeriesProp::Lock()
{
	::EnterCriticalSection(&csSeries);
}


void SeriesProp::Unlock()
{
	::LeaveCriticalSection(&csSeries);
}


void SeriesProp::LockData()
{
	_lockData.Lock();
}

void SeriesProp::UnlockData()
{
	_lockData.Unlock();
}

bool SeriesProp::Export(const CString &, bool bAll)
{
	::AfxMessageBox(L"not implemented");
	return false;
}

DebuggingProcessProp * SeriesProp::GetProcess()
{
	return this->process;
}

void SeriesProp::SetProcess(DebuggingProcessProp * process)
{
	ASSERT(this->process == 0);
	process->IncRefCount();
	this->process = process;
}

void SeriesProp::ReleaseProcess()
{
	if (this->process)
	{
		this->process->DecRefCount();
		this->process = 0;
	}
}

//void SeriesProp::UpdateView()
//{
//	if (!process->updateReplayBuffer)
//		return;
//
//	SharedSeriesInfo * sharedSeriesInfo = (SharedSeriesInfo *)this->smSeriesInfo->GetAddress();
//	if (sharedSeriesInfo->dataElementSize == 0)
//		return;
//
//	char * dataAddr = (char *)this->smSeriesData->GetAddress();
//
//	int wIdx = sharedSeriesInfo->wIdx;
//	int rIdx = sharedSeriesInfo->rIdx;
//
//	int bufLen = sharedSeriesInfo->bufLen;
//	bufLen = bufLen - bufLen % sharedSeriesInfo->dataElementSize;
//
//	if (sharedSeriesInfo->replace)
//		sharedSeriesInfo->rIdx = 0;
//	else
//		sharedSeriesInfo->rIdx = wIdx;
//
//	int dataLen = wIdx - rIdx;
//	if (dataLen < 0)
//	{
//		dataLen += bufLen;
//	}
//
//	if (dataLen >= 0)
//	{
//		int replayWritePos = this->process->replayWritePos;
//		SnapshotData * snapShotData = &this->replayBuffer[replayWritePos];
//		if (!snapShotData->data)
//		{
//			snapShotData->data = new char[::SettingGetSnapShotBufferSize()];	//TODO
//			snapShotData->bufLen = ::SettingGetSnapShotBufferSize();
//		}
//		snapShotData->len = dataLen;
//
//		for (int i = 0; i < dataLen; i++)
//		{
//			snapShotData->data[i] = dataAddr[(rIdx+i)%bufLen];
//		}
//	}
//}

void SeriesProp::ForceUpdateGraph()
{
	if (this->plotWnd)
		this->plotWnd->ForceUpdateGraph();
}

struct DataItem
{
	const char * ptr;
	size_t length;
};

bool SeriesProp::UpdateFromDataQueue()
{
	vector<DataItem> vecDataItem;
	this->ReadData([this, &vecDataItem](const char * ptr, size_t length){
		DataItem d;
		d.ptr = ptr;
		d.length = length;
		vecDataItem.push_back(d);
	});

	for_each(vecDataItem.begin(), vecDataItem.end(), [this](DataItem item){
		this->Write(item.ptr, item.length);
		delete [] item.ptr;
	});

	return vecDataItem.size() != 0;
}

/***********************************

Spectrum Series

***********************************/
//SpectrumSeriesProp::SpectrumSeriesProp()
//{
//	colorIsSet = false;
//	color = RGB(0, 255, 0);	// green;
//
//	maxValue = 0;
//	minValue = 0;
//	spectrumDataSize = 0;
//}
//
//void SpectrumSeriesProp::Close()
//{
//	CWnd * targetWnd = this->GetTargetWnd();
//	if (targetWnd)
//	{
//		targetWnd->PostMessage(WM_APP, CMD_DATA_CLEAR, (LPARAM)this);
//		targetWnd->InvalidateRgn(NULL, 1);
//		this->SetTargetWnd(0);
//	}
//
//	SeriesProp::Close();
//}
//
//BaseProperty * SpectrumSeriesProp::GetPropertyPage()
//{
//	BaseProperty * property = new SpectrumSeriesProperty;
//	property->SetTarget(this);
//	return property;	
//}
//
//HRESULT SpectrumSeriesProp::CreateElementSeries(IXMLDOMDocument *pDom, IXMLDOMElement *pe)
//{
//	IXMLDOMElement *pssub=NULL;
//	HRESULT hr=S_OK;
//
//	CreateAndAddElementNode(pDom, L"SName", L"\n\t\t\t", pe, &pssub);
//	pssub->put_text((_bstr_t)this->name);
//	//pssub->Release();
//	CreateAndAddElementNode(pDom, L"SType", L"\n\t\t\t", pe, &pssub);
//	pssub->put_text(_bstr_t(typeid(*this).name()));
//	//pssub->Release();
//	CreateAndAddElementNode(pDom, L"colorIsSet", L"\n\t\t\t", pe, &pssub);
//	pssub->put_text((_bstr_t)this->colorIsSet);
//	//pssub->Release();
//	CreateAndAddElementNode(pDom, L"color", L"\n\t\t\t", pe, &pssub);
//	pssub->put_text((_bstr_t)this->color);
//
//	CreateAndAddElementNode(pDom, L"subRectTLX", L"\n\t\t", pe, &pssub);
//	pssub->put_text((_bstr_t)this->rect.TopLeft().x);
//
//	CreateAndAddElementNode(pDom, L"subRectTLY", L"\n\t\t", pe, &pssub);
//	pssub->put_text((_bstr_t)this->rect.TopLeft().y);
//
//	CreateAndAddElementNode(pDom, L"subRectBRX", L"\n\t\t", pe, &pssub);
//	pssub->put_text((_bstr_t)this->rect.BottomRight().x);
//
//	CreateAndAddElementNode(pDom, L"subRectBRY", L"\n\t\t", pe, &pssub);
//	pssub->put_text((_bstr_t)this->rect.BottomRight().y);
//
//	return hr;
//}
//
//





DebuggingProcessProp::DebuggingProcessProp()
{
	smProcess = 0;
	eventViewerPlotterSync = 0;
	smSourceInfo = 0;
	smSourceData = 0;

	alive = false;
	this->openCount = 0;

	this->pid = 0;
	this->replayCapacity = REPLAY_BUF_LEN;
	this->replayReadPosInProcess = 0;
	replayReadPosRatio = 0.0;
	replayStep = 1.0;
	this->replayWritePos = 0;
	this->replayPaused = false;
	//this->replayAutoPlay = true;
	this->isActive = false;
	this->replayAutoPlayActionPerformed = false;
	this->trackbarWnd = 0;
	
	traceMode = MODE_TRACE_SOURCE;
	//replayAutoPlay = true;
	//threshold = ::SettingGetSnapShotBufferDefaultThreshold();
	// source control
	viewWndStart = 0;
	viewWndSize = ::SettingGetSourceBufferSize() / sizeof(COMPLEX16) / 16;
}

DebuggingProcessProp::~DebuggingProcessProp()
{
	this->FreeSharedMem();
}

BaseProperty * DebuggingProcessProp::GetPropertyPage()
{
	return 0;
}

void DebuggingProcessProp::ReplayPlay()
{
	this->replayPaused = false;
}

void DebuggingProcessProp::ReplayPause()
{
	this->replayPaused = true;
}

bool DebuggingProcessProp::ReplayIsPaused()
{
	return this->replayPaused;
}

HRESULT DebuggingProcessProp::ReplaySeek(int rPos)
{
	if (
		(rPos < 0) ||
		(rPos > this->replayWritePos) )
	{
		return -1;
	}

	this->replayReadPosInProcess = rPos;

	return S_OK;
}

void DebuggingProcessProp::PlayAFrame()
{
	//this->updateReplayBuffer = false;
	////
	//if (this->smProcess)
	//{
	//	SharedProcessInfo * sharedProcessInfo = (SharedProcessInfo *)this->smProcess->GetAddress();
	//	if (sharedProcessInfo->pauseFlag)
	//	{
	//		sharedProcessInfo->pauseFlag = false;
	//		this->replayPaused = true;
	//		if (this->trackbarWnd)
	//			this->trackbarWnd->PostMessage(WM_APP, CMD_CONTROLLER_UPDATE_BUTTON, 0);
	//	}
	//}

	////
	//if (this->replayWritePos == this->replayCapacity - 1) // last element
	//{
	//	if ( this->replayReadPosInProcess == this->replayCapacity - 1 )
	//	{
	//		if (!this->replayAutoPlay && !this->replayAutoPlayActionPerformed)
	//		{
	//			this->replayPaused = true;
	//			this->replayAutoPlayActionPerformed = true;
	//			if (this->trackbarWnd)
	//				this->trackbarWnd->PostMessage(WM_APP, CMD_CONTROLLER_UPDATE_BUTTON, 0);
	//		}

	//		if (!this->replayPaused)
	//		{
	//			this->replayWritePos = 0;
	//			this->replayReadPosInProcess = 0;
	//			this->updateReplayBuffer = true;
	//			this->replayAutoPlayActionPerformed = false;
	//			int writePos = this->replayWritePos;
	//			std::for_each(series.begin(), series.end(), [&writePos](SeriesProp * series){
	//				series->replayBuffer[writePos].len = 0;
	//			});
	//		}
	//	}
	//	else
	//	{
	//		if (!this->replayPaused)
	//		{
	//			this->replayReadPosRatio += this->replayStep;
	//			if (this->replayReadPosRatio >= 1.0)
	//			{
	//				this->replayReadPosRatio = 0.0;
	//				this->replayReadPosInProcess++;
	//			}
	//		}
	//	}
	//}
	//else
	//{
	//	if (!this->replayPaused)
	//	{
	//		this->replayReadPosRatio += this->replayStep;
	//		if (this->replayReadPosRatio >= 1.0)
	//		{
	//			this->replayReadPosRatio = 0.0;
	//			this->replayReadPosInProcess++;
	//		}
	//	}

	//	if (this->replayWritePos < this->replayReadPosInProcess)
	//	{
	//		this->replayWritePos++;
	//		this->updateReplayBuffer = true;
	//		int writePos = this->replayWritePos;
	//		std::for_each(series.begin(), series.end(), [&writePos](SeriesProp * series){
	//			series->replayBuffer[writePos].len = 0;
	//		});
	//	}
	//}
}

bool DebuggingProcessProp::ShouldUpdateReplayBuffer()
{
	return this->updateReplayBuffer;
}

void DebuggingProcessProp::FreeSharedMem()
{
	if (this->smProcess)
		ShareMem::FreeShareMem(this->smProcess);
	this->smProcess = 0;

	if (this->eventViewerPlotterSync)
		ShareLock::FreeShareLock(this->eventViewerPlotterSync);
	this->eventViewerPlotterSync = 0;

	if (this->smSourceInfo)
		ShareMem::FreeShareMem(this->smSourceInfo);
	this->smSourceInfo = 0;

	if (this->smSourceData)
		ShareMem::FreeShareMem(this->smSourceData);
	this->smSourceData = 0;
}

void DebuggingProcessProp::RemoveSeries(SeriesProp * seriesProp)
{
	std::vector<SeriesProp *>::iterator iter;
	for (iter = this->series.begin();
		iter != this->series.end();
		iter++)
	{
		if (*iter == seriesProp)
		{
			this->series.erase(iter);
			break;
		}
	}
}


void DebuggingProcessProp::AddSeries(SeriesProp * prop)
{
	this->series.push_back(prop);
}

void DebuggingProcessProp::SetEventForPlotter()
{
	if (eventViewerPlotterSync)
	{
		eventViewerPlotterSync->SetShareEvent();
	}
}

/***********************************

Plot Wnd Area

************************************/

PlotWndAreaProp::PlotWndAreaProp()
{
	gridSize = 20;
	autoLayoutSize = 100;
	initialPlotWndWidth = 400;
	initialPlotWndHeight = 400;

	hMutexTree = ::CreateMutex(NULL, FALSE, NULL);
}

PlotWndAreaProp::~PlotWndAreaProp()
{
	::WaitForSingleObject(hMutexTree, INFINITE);
	this->plotWndsInPlotWndAreaProp.clear();
	::ReleaseMutex(hMutexTree);
	::CloseHandle(hMutexTree);
}

BaseProperty * PlotWndAreaProp::GetPropertyPage()
{
	BaseProperty * property = new PlotWndAreaProperty;
	property->SetTarget(this);
	return property;
}

int PlotWndAreaProp::GetGridSize()
{
	return gridSize;
}

void PlotWndAreaProp::PlotGraphToScreen()
{
	std::vector<PlotWndProp *>::iterator iterPlotWnd;
	for (iterPlotWnd = this->plotWndsInPlotWndAreaProp.begin();
		iterPlotWnd != this->plotWndsInPlotWndAreaProp.end();
		iterPlotWnd++ )
	{
		PlotWndProp * plotWndProp = *iterPlotWnd;
		plotWndProp->PlotGraphToScreen();
	}
}

void PlotWndAreaProp::AddPlotWnd(PlotWndProp * prop)
{
	::WaitForSingleObject(hMutexTree, INFINITE);
	this->plotWndsInPlotWndAreaProp.push_back(prop);
	::ReleaseMutex(hMutexTree);
}

#include "stdafx.h"
#include <memory>
#include "ProcessOpened.h"
#include "TaskSimple.h"
#include "AppSettings.h"
#include "HelperFunc.h"

using namespace std;
using namespace SoraDbgPlot::Task;
using namespace SoraDbgPlot::SharedObj;

volatile unsigned long ProcessOpened::__sid = 0;

ProcessOpened::ProcessOpened()
{
	_viewWndStart = 0;
	_viewWndWidth = ::SettingGetSourceBufferComplex16Count();
	_sid = ::InterlockedIncrement(&__sid);
	_pid = -1;
}

ProcessOpened::ProcessOpened(const wstring & name)
{
	_viewWndStart = 0;
	_viewWndWidth = ::SettingGetSourceBufferComplex16Count();
	_sid = ::InterlockedIncrement(&__sid);
	_name = name;
	_pid = -1;
}

shared_ptr<BaseProperty> ProcessOpened::CreatePropertyPage()
{
	return 0;
}

shared_ptr<TaskSimple>
	ProcessOpened::TaskAttatchSharedProcess(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess> process)
{
	auto SThis = dynamic_pointer_cast<ProcessOpened, AsyncObject>(shared_from_this());
	return make_shared<TaskSimple>(TaskQueue(), [SThis, process](){
		SThis->_sharedProcess = process;
		SThis->_pid = process->Pid();
	});
}

shared_ptr<TaskSimple>
	ProcessOpened::TaskDeattatchSharedProcess()
{
	auto SThis = dynamic_pointer_cast<ProcessOpened, AsyncObject>(shared_from_this());
	return make_shared<TaskSimple>(TaskQueue(), [SThis](){
		SThis->_sharedProcess.reset();
		SThis->_pid = -1;
	});
}

void ProcessOpened::AttatchSharedProcessSync(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess> sharedProcess)
{
	auto SThis = dynamic_pointer_cast<ProcessOpened, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([SThis, sharedProcess](){
		SThis->_sharedProcess = sharedProcess;
		SThis->_pid = sharedProcess->Pid();
		SThis->_name = sharedProcess->ModuleName();
	});
}

void ProcessOpened::DeattatchSharedProcessSync()
{
	auto SThis = dynamic_pointer_cast<ProcessOpened, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([SThis](){
		SThis->_sharedProcess.reset();
		SThis->_pid = -1;
	});
}

int ProcessOpened::Pid()
{
	int ret = 0;
	auto SThis = dynamic_pointer_cast<ProcessOpened, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([SThis, &ret](){
		ret = SThis->_pid;
	});

	return ret;
}

unsigned long ProcessOpened::Sid()
{
	return _sid;
}

std::wstring ProcessOpened::Name()
{
	wstring ret;
	auto SThis = dynamic_pointer_cast<ProcessOpened, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([SThis, &ret](){
		ret = SThis->_name;
	});

	return ret;
}

void ProcessOpened::PollPauseEvent(const std::function<void(bool)> & f)
{
	auto SThis = dynamic_pointer_cast<ProcessOpened, AsyncObject>(shared_from_this());
	this->DoLater([SThis, f](){
		bool isPause = false;
		if (SThis->_sharedProcess)
		{
			isPause = SThis->_sharedProcess->TestPauseFlag();
		}

		f(isPause);
	});
}

void ProcessOpened::TraceBarPlayPause(bool bPlay)
{
	auto SThis = dynamic_pointer_cast<ProcessOpened, AsyncObject>(shared_from_this());
	this->DoLater([SThis, bPlay](){
		if (SThis->_sharedProcess)
		{
			if (bPlay)
				SThis->_sharedProcess->RawDataPlay();
			else
				SThis->_sharedProcess->RawDataPause();
		}
	});
}

void ProcessOpened::TraceBarSingleStep()
{
	auto SThis = dynamic_pointer_cast<ProcessOpened, AsyncObject>(shared_from_this());
	this->DoLater([SThis](){
		if (SThis->_sharedProcess)
			SThis->_sharedProcess->RawDataSingleStep();
	});
}

void ProcessOpened::TrackBarSeek(double pos)
{
	auto SThis = dynamic_pointer_cast<ProcessOpened, AsyncObject>(shared_from_this());
	this->DoLater([SThis, pos](){
		double pos2 = pos;
		pos2 = max(0.0, pos2);
		pos2 = min(1.0, pos2);
		int seekPos = (int)(SThis->_viewWndStart + SThis->_viewWndWidth * pos2);
		if (SThis->_sharedProcess)
			SThis->_sharedProcess->SetReadPos(seekPos);
	});
}

void ProcessOpened::TrackBarWheel(bool bUp)
{
	auto SThis = dynamic_pointer_cast<ProcessOpened, AsyncObject>(shared_from_this());
	this->DoLater([SThis, bUp](){
		if (bUp)
		{
			if (SThis->_viewWndWidth > 4)
				SThis->_viewWndWidth /= 2;
			else
				SThis->_viewWndWidth = 2;
		}
		else
		{
			if (SThis->_viewWndWidth < ::SettingGetSourceBufferComplex16Count() / 2)
				SThis->_viewWndWidth *= 2;
			else
				SThis->_viewWndWidth = ::SettingGetSourceBufferComplex16Count();
		}
	});
}

void ProcessOpened::SetBitmapRect(const CRect & rect)
{
	auto SThis = dynamic_pointer_cast<ProcessOpened, AsyncObject>(shared_from_this());
	this->DoLater([SThis, rect](){
		SThis->_rectBitmap = rect;
	});
}

void ProcessOpened::GenUpdateData(const std::function<void(Bitmap *, bool, shared_ptr<ProcessOpened>)> & f)
{
	auto SThis = dynamic_pointer_cast<ProcessOpened, AsyncObject>(shared_from_this());
	this->DoLater([SThis, f](){
		if (SThis->_sharedProcess == 0)
			return;

		Bitmap * bitmap = SThis->CreateBitmap();
		bool isInUse = SThis->_sharedProcess->IsRawDataBufferUsed();
		f(bitmap, isInUse, SThis);
	});	
}

Bitmap * ProcessOpened::CreateBitmap()
{
	// tbd
	//assert(false);
	Bitmap * bitmap = 0;
	auto SThis = dynamic_pointer_cast<ProcessOpened, AsyncObject>(shared_from_this());

	CRect rectBottomBar = SThis->_rectBitmap;
	rectBottomBar.top += 5;
	rectBottomBar.bottom -= 1;

	if (SThis->_rectBitmap.Width() == 0 ||
		SThis->_rectBitmap.Height() == 0)
		return 0;

	if (SThis->_sharedProcess == 0)
		return 0;

	int rectWidth = SThis->_rectBitmap.Width();
	int bufSize = min(SThis->_viewWndWidth, rectWidth);

	COMPLEX16 * buf = new COMPLEX16[bufSize];
	float * dataY = new float[bufSize];

	SolidBrush brush(Color(0, 255, 0));
	Pen pen(&brush, 1.0f);
	SolidBrush brushReadPos(Color(200, 200, 200));
	Pen penReadPos(&brushReadPos, 3.0f);

	int sizeFilled = 0;
	int readPos;
	SThis->_sharedProcess->PeekRawData(buf, bufSize, SThis->_viewWndStart, SThis->_viewWndWidth, readPos, sizeFilled);

	//CString str;
	//str.Format(L"%d, %d, %d, %d, %d\n", bufSize, SThis->_viewWndStart, SThis->_viewWndWidth, readPos, sizeFilled);
	//::OutputDebugString(str);

	if (sizeFilled == 0)
	{
		delete [] dataY;
		delete [] buf;
		return 0;
	}

	bitmap = new Bitmap(SThis->_rectBitmap.Width(), SThis->_rectBitmap.Height());
	Graphics * g = Graphics::FromImage(bitmap);
	//SolidBrush bgBrush(Color::Black);
	//g->FillRectangle(&bgBrush, 0, 0, SThis->_rectBitmap.Width(), SThis->_rectBitmap.Height());

	// normalize
	float maxValue, minValue, range;
	bool bInit = false;
	for (int i = 0; i < sizeFilled; ++i)
	{
		COMPLEX16 data = buf[i];
		dataY[i] = (float)sqrt((data.re * data.re) + (float)(data.im * data.im));
		if (!bInit)
		{
			bInit = true;
			maxValue = minValue = dataY[i];
		}
		else
		{
			maxValue = max(maxValue, dataY[i]);
			minValue = min(minValue, dataY[i]);
		}
	}


	range = maxValue - minValue;
	if (range == 0.0f)
	{
		maxValue += 1.0f;
		minValue -= 1.0f;
		range = 2.0f;
	}

	for (int i = 0; i < sizeFilled; ++i)
	{
		float fromBottom = (dataY[i] - minValue) * rectBottomBar.Height() / range;
		dataY[i] = rectBottomBar.bottom - fromBottom;
	}

	int lastX, lastY;
	bool bFirstPoint = true;


	if (bufSize < rectWidth) // iter over data
	{
		for (int i = 0; i < sizeFilled; ++i)
		{
			int x = (i * rectWidth / (SThis->_viewWndWidth - 1));
			if (bFirstPoint)
			{
				bFirstPoint = false;
			}
			else
			{
				g->DrawLine(&pen, Point(lastX, lastY), Point(x, (int)dataY[i]));
			}

			lastX = x;
			lastY = (int)dataY[i];
		}
	}
	else					// iter over pixel
	{
		for (int i = 0; i < sizeFilled; i++)
		{
			int x = i;
			if (bFirstPoint)
			{
				bFirstPoint = false;
			}
			else
			{
				g->DrawLine(&pen, Point(lastX, lastY), Point(x, (int)dataY[i]));
			}

			lastX = x;
			lastY = (int)dataY[i];
		}
	}

	// draw read pos
	if (readPos >= SThis->_viewWndStart && readPos < SThis->_viewWndStart + SThis->_viewWndWidth)
	{
		int xReadPos = (int)((__int64)(readPos - SThis->_viewWndStart) * rectWidth / (SThis->_viewWndWidth - 1));
		g->DrawLine(&penReadPos, Point(xReadPos, rectBottomBar.top), Point(xReadPos, rectBottomBar.bottom));
	}
	
	// draw top bar
	CRect rectTopBar = SThis->_rectBitmap;
	rectTopBar.bottom = rectTopBar.top + 5;

	CRect rectTopBarIndicator = rectTopBar;
	rectTopBarIndicator.top += 1;
	rectTopBarIndicator.bottom -= 1;
	rectTopBarIndicator.left = _viewWndStart * rectTopBar.Width() / ::SettingGetSourceBufferComplex16Count();
	rectTopBarIndicator.right = (int)(rectTopBarIndicator.left + (__int64)(_viewWndWidth - 1) * rectTopBar.Width() / ::SettingGetSourceBufferComplex16Count());
	rectTopBarIndicator.right = max(rectTopBarIndicator.right, rectTopBarIndicator.left + 1);
	SolidBrush brushTopBarIndicator(Color(200, 200, 200));
	g->FillRectangle(&brushTopBarIndicator, rectTopBarIndicator.left, rectTopBarIndicator.top, rectTopBarIndicator.Width(), rectTopBarIndicator.Height());

	delete g;
	delete [] dataY;
	delete [] buf;

	return bitmap;
}

HRESULT ProcessOpened::CreateXmlElement(IXMLDOMDocument *pDom, IXMLDOMElement *pParent)
{
	CString name;
	int pid;

	auto SThis = dynamic_pointer_cast<ProcessOpened, AsyncObject>(shared_from_this());
	this->DoNow([SThis, &name, &pid]() mutable {
		name = SThis->_name.c_str();
		pid = SThis->_pid;
	});

	IXMLDOMElement *pElement=NULL;

	CreateAndAddElementNode(pDom, L"Process", L"\n\t\t", pParent, &pElement);
	pElement->put_text((_bstr_t)name);
	CreateAndAddElementNode(pDom, L"ProcessName", L"\n\t\t", pParent,&pElement);
	pElement->put_text((_bstr_t)(Sid()));

	return S_OK;
}

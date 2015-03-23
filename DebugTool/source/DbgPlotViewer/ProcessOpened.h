#pragma once

#include "PropObject.h"
#include <string>
#include <functional>
#include "TaskQueue.h"
#include "Event.h"
#include "SharedProcess.h"
#include "TaskSimple.h"

class ProcessOpened : public PropObject
{
public:
	ProcessOpened();
	ProcessOpened(const std::wstring & name);

	virtual std::shared_ptr<BaseProperty> CreatePropertyPage();

	std::shared_ptr<SoraDbgPlot::Task::TaskSimple>
	TaskAttatchSharedProcess(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess>);
	
	std::shared_ptr<SoraDbgPlot::Task::TaskSimple>
	TaskDeattatchSharedProcess();

	void AttatchSharedProcessSync(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess>);
	void DeattatchSharedProcessSync();

	int Pid();
	std::wstring Name();

	void PollPauseEvent(const std::function<void(bool)> & f);

	void TraceBarPlayPause(bool bPlay);
	void TraceBarSingleStep();
	void TrackBarSeek(double pos);
	void TrackBarWheel(bool bUp);
	void SetBitmapRect(const CRect & rect);

	void GenUpdateData(const std::function<void(Bitmap *, bool, std::shared_ptr<ProcessOpened>)> & f);
	virtual HRESULT CreateXmlElement(IXMLDOMDocument *pDom, IXMLDOMElement *pe);

	unsigned long Sid();

private:
	Bitmap * CreateBitmap();

private:
	static volatile unsigned long __sid;
	unsigned long _sid;

private:
	int _viewWndStart;
	int _viewWndWidth;

	CRect _rectBitmap;
	int _pid;
	std::wstring _name;
	std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess> _sharedProcess;
};

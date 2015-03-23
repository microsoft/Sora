#pragma once
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <list>

#include "CustomProperties.h"
#include "SharedStruct.h"
#include "ShareMemHelper.h"
//#include "_share_mem_if.h"
#include "_share_lock_if.h"
#include "ILog.h"
#include "IChannelBuffer.h"
#include "FrameWithSizeFilter.h"
#include "RingBufferWithTimeStamp.h"
#include "WaitableLatestTaskQueue.h"
#include "Event.h"
#include "DataQueue.h"
#include "CSLock.h"

#import <msxml6.dll>

class BaseProperty;
//class CPlotWnd;
class SubPlotWnd;
class DebuggingProcessProp;
class PlotWndProp;

class ObjProp
{
public:
	ObjProp();
	virtual ~ObjProp();

	virtual BaseProperty * GetPropertyPage() = 0;
	virtual void UpdatePropertyPage();

	void IncRefCount();
	void DecRefCount();
	int GetRefCount();

public:
	bool alive;

private:
	volatile unsigned int refCount;
};

struct SnapshotData
{
	SnapshotData() : data(0), len(0), bufLen(0) {}
	~SnapshotData() {
		if (data)
			delete [] data;
	}
	char * data;
	int bufLen;
	int len;
};

class SeriesProp : public ObjProp
{
public:
	bool UpdateFromDataQueue();

public:	// serialize
	CString name;
	CRect rect;

private:	// serialize(ptr)
	PlotWndProp * plotWnd;					// weak ptr, parent
public:
	void SetPlotWndProp(PlotWndProp * prop);

protected:	// serialize(ptr)
	DebuggingProcessProp * process;		// strong ptr
public:
	DebuggingProcessProp * GetProcess();
	void SetProcess(DebuggingProcessProp * process);
	void ReleaseProcess();
	unsigned int SerialNum();

public:
	SoraDbgPlot::Event::Event<bool> EventWriteData;

	void WriteData(const void * ptr, size_t length);
protected:
	virtual void Write(const void * ptr, size_t length) = 0;
	virtual void ClearData() = 0;
private:
	void ReadData(const function<void(const char * ptr, const size_t length)> &);
private:
	SoraDbgPlot::DataQueue::DataQueue * _dataQueue;

private:
	CWnd * pWnd;
public:
	CWnd * GetTargetWnd();
	void SetTargetWnd(CWnd * wnd);

public:
	void FreeSharedMem();
	void SetSmInfo(ShareMem * sm);

private:	// active property
	ShareMem * smSeriesInfo;				// weak ptr
	SoraDbgPlot::Lock::CSLock _lockSmInfo;
	void OnSmInfoSetMe();
	virtual void OnSmInfoSet(SharedSeriesInfo * sharedSeriesInfo);
	virtual void OnSmInfoRemoved();
	void OnSmInfoRemovedMe();

public:
	//ShareMem * smSeriesData;				// weak ptr
	ShareLock * sharedLock;

public:
	SeriesProp();
	~SeriesProp();

public:
	void ForceUpdateGraph();
	PlotWndProp * GetPlotWndProp();
	virtual HRESULT CreateElementSeries(IXMLDOMDocument *pDom, IXMLDOMElement *pe)= 0;
	//virtual void UpdateView();
	virtual void Close();
	void NotifyClosed();
	void EraseFromProcess();

	bool isActive;
	bool isOpened;

	int lastReplayIdx;

public:	// replay
	//static const int PEPLAY_BUF_FRAME_SIZE = 4*1024;
	std::vector<SnapshotData> replayBuffer;

// lock
public:
	void Lock();
	void Unlock();
private:
	CRITICAL_SECTION csSeries;

// play/pause
public:
	void EnableWrite(bool enable);
	bool IsWriteEnabled();
private:
	bool _writeEnabled;

public:
	void LockData();
	void UnlockData();
private:
	SoraDbgPlot::Lock::CSLock _lockData;

public:
	virtual bool Export(const CString &, bool bAll) = 0;
};



class DebuggingProcessProp : public ObjProp
{
public:	// serialize
	CString moduleName;
	enum {
		MODE_TRACE_SOURCE,
		MODE_TRACE_GRAPH,
	} traceMode;		// new_for_lili

public:	// serialize(ptr)
	std::vector<SeriesProp *> series;	// weak ptr

	SoraDbgPlot::Event::Event<bool> EventPlayPauseAll;

public:
	DebuggingProcessProp();
	~DebuggingProcessProp();

	int pid;
	int openCount;
	bool isActive;

public:	// replay
	bool ReplayIsPaused();
	void ReplayPause();
	void ReplayPlay();
	HRESULT ReplaySeek(int rPos);
	void PlayAFrame();
	bool ShouldUpdateReplayBuffer();
	int replayCapacity;
	int replayReadPosInProcess;
	double replayReadPosRatio;
	__int64 replayReadPosMs;
	double replayStep;
	int replayWritePos;
	bool replayPaused;
	bool updateReplayBuffer;
	bool replayAutoPlayActionPerformed;
private:

public:
	static const int REPLAY_BUF_LEN = 10*60*3;
	void SetPID(int pid);
	int GetPID();
	void GetModuleName(CString & name);
	void SetModuleName(CString & name);
	virtual BaseProperty * GetPropertyPage();
	void FreeSharedMem();
	void RemoveSeries(SeriesProp * seriesProp);
	void AddSeries(SeriesProp *);
	void SetEventForPlotter();

	CWnd * trackbarWnd;

public: // Source Control
	int viewWndStart;
	int viewWndSize;

public:
	ShareMem * smProcess;					// weak ptr
	ShareLock * eventViewerPlotterSync;		// weak ptr

	ShareMem * smSourceInfo;
	ShareMem * smSourceData;
};

class PlotWndAreaProp : public ObjProp
{
public:	// serialize
	int gridSize;
	int autoLayoutSize;

public:	// serialize(ptr)
	std::vector<PlotWndProp *> plotWndsInPlotWndAreaProp;		// weak ptr

public:
	CWnd * targetWnd;

public:
	PlotWndAreaProp();
	virtual ~PlotWndAreaProp();

	virtual BaseProperty * GetPropertyPage();
	int GetGridSize();
	void SetGridSize(int gridSize);
	int GetAutoLayoutSize();
	void SetAutoLayoutSize(int autoLayoutSize);
	int GetPlotWndSize(CRect & rect);
	void SetPlotWndSize(CRect & rect);
	//void UpdatePlotGraph();
	void PlotGraphToScreen();
	void AddPlotWnd(PlotWndProp * prop);

public:	// const
	int initialPlotWndWidth;
	int initialPlotWndHeight;

public:
	HANDLE hMutexTree;

};




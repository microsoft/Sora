#pragma once

#include <vector>
#include "dsplib.h"
#include "simplewnd.h"

class PlotData
{
public:
	PlotData(int bufsize);
	~PlotData();
	int AddData(int * input, int size);
	int GetNewestData(int * output, int size);

private:
	int * buffer;
	int * writePointer;
	int * readPointer;
	int bufsize;
	int readableCnt;
	int writableCnt;
};

class Graph;

class Series
{
	friend class Graph;
public:
	Series(int len, int dataSize);
	~Series();

	int GetDataSize();
	int GetDataCount();

	int PushData(int *, int cnt);
	int * GetPlotBuf();
private:

	void SetGraph(Graph * graph);

	int len;
	int dataSize;

	int * buf;
	int * curPtr;

	Graph * graph;
};

class Graph
{
public:
	virtual void Draw() = 0;
	int AddSeries();

protected:
	std::vector<Series *> series;
};

class OverviewGraph : public Graph
{
public:
	void Draw();
	OverviewGraph(int windowsize);
	~OverviewGraph();
	
	double * plotBuf;

private:
	CAnimWnd * wndOverview;
	//double * buf;
	//double * curPtr;
	//double * plotBuf;
	//int bufLen;
};

class ConstelGraph : public Graph
{
public:
	ConstelGraph(int windowsize);
	~ConstelGraph();
	void PushData(int * data);
	int GetDataSize();

private:
	CAnimWnd * wndConstel;
	COMPLEX * buf;
	COMPLEX * curPtr;
	COMPLEX * plotBuf;
	int bufLen;
};

class Ploter
{
public:
	Ploter(PlotData * data, int bufsize);
	void AddGraph(Graph * graph);
	void Plot();
private:
	PlotData * data;

	double * bufferDouble;
	int * buffer;
	int bufsize;
	double * curPointerDouble;
	int * curPointer;
	int windowSize;
	int validDataLen;

	std::vector<Graph*> graphs;
};

class PlotPlayer
{
public:
	PlotPlayer();
	void AddPloter(Ploter * ploter);
	void Play();
	void Stop();

private:
	static DWORD WINAPI PlotThread(PVOID);
	std::vector<Ploter *> ploters;
	HANDLE threadPlayer;
};

class Sync
{
public:
	Sync();
	~Sync();

	void Enable();
	void Disable();
	void Wait();
	void Signal();

private:
	HANDLE hEvent;
	bool enabled;
}

class SoraThread
{
public:
	SoraThread();
	HRESULT Start(bool sync);
	HRESULT Stop(bool sync);
	void Suspend();
	void Resume();
	void SetCallback(ThreadCallback *);
	
	bool CheckStatus();

	virtual void Threadfunc() = 0;
	

protected:
	bool CheckThreadEvent();

private:
	static DWORD WINAPI ThreadProc(PVOID);
	HANDLE thread;
	DWORD threadID;

	Sync syncStartObj;
	Sync syncEndObj;
	Sync syncSuspendObj;
	
	bool flagStop;
	bool flag

	void RunThreadStartCallback();
	void RunThreadEndCallback();
	void RunThreadFunction();

};


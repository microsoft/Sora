#pragma once

#include "SoraThread.h"
#include <vector>
#include <deque>
#include "dsplib.h"
#include "simplewnd.h"

template <typename T>
class DrawMethod
{
public:
	virtual void Draw(std::deque<T> * deque, std::deque<bool> * dequeValid, unsigned size) = 0;
};

class DrawLine : public DrawMethod<double>
{
public:
	DrawLine(CAnimWnd * wnd, HWND mainThreadWnd);
	void SetColor(COLORREF color);
	void Draw(std::deque<double> * deque, std::deque<bool> * dequeValid, unsigned int size);
private:
	CAnimWnd * wnd;
	COLORREF color;
	HWND mainThreadWnd;
};

class DrawDots : public DrawMethod<COMPLEX>
{
public:
	DrawDots(CAnimWnd * wnd, HWND mainThreadWnd);
	void SetColor(COLORREF color);
	void Draw(std::deque<COMPLEX> * deque, std::deque<bool> * dequeValide, unsigned int size);
private:
	CAnimWnd * wnd;
	COLORREF color;
	HWND mainThreadWnd;
};

template <typename T>
class Series
{
public:
	Series(int size);
	~Series();
	void Clear();
	void PushData(const T & data);
	void PushVoid();
	void SetDrawMethod(DrawMethod<T> * drawMethod);
private:
	DrawMethod<T> * drawMethod;
	std::deque<T> deque;
	std::deque<bool> dequeValid;
	int size;
};

template <typename T>
Series<T>::Series(int size)
{
	drawMethod = 0;
	this->size = size;
	deque.resize(size);
	dequeValid.resize(size);
	Clear();
}

template <typename T>
Series<T>::~Series()
{
}

template <typename T>
void Series<T>::PushData(const T & data)
{
	deque.push_back(data);
	deque.pop_front();
	dequeValid.push_back(true);
	dequeValid.pop_front();

	//int sizeDeque = deque.size();
	//deque.push_back(data);
	//sizeDeque = deque.size();
	//if (deque.size() > size)
	//	deque.pop_front();

	if (drawMethod)
		drawMethod->Draw(&deque, &dequeValid, size);
}

template <typename T>
void Series<T>::SetDrawMethod(DrawMethod<T> * drawMethod)
{
	this->drawMethod = drawMethod;
}

//template <typename T>
//void Series<T>::Clear()
//{
//	std::deque<bool>::iterator itrValid;
//	for (itrValid = dequeValid.begin(); itrValid != dequeValid.end(); ++itrValid)
//	{
//		*itrValid = false;
//	}
//
//	std::deque<T>::iterator itr;
//	for (itr = dequeValid.begin(); itr != dequeValid.end(); ++itr)
//	{
//		*itr = T();
//	}
//}	

class Plot
{
public:
	virtual void PlotData() = 0 ;
	virtual void SetTimeIntervalMS(int timems) = 0;
	virtual int GetTimeIntervalMS() = 0;
	virtual void SetTimeElapsedMS(int timems) = 0;
	virtual int GetTimeElapsedMS() = 0;
};

template <typename T>
class PlotImpl : public Plot
{
public:
	PlotImpl(Series<T> * series, int snapshotBufSize);
	~PlotImpl();
	HRESULT PushData(double * data, int len);
	HRESULT PushPackage(double * data, int len);
	void PlotData();
	void SetTimeIntervalMS(int timems);
	int GetTimeIntervalMS();
	void SetTimeElapsedMS(int timems);
	int GetTimeElapsedMS();
	void Clear();
private:

	std::deque<double> deques[2];

	int timems;
	int timemsElapsed;

	int pushBufSel;
	int plotBufSel;

	//double * bufs[2];
	//int bufsLen[2];
	unsigned int snapshotBufSize;
	unsigned int plotBufIdx;
	//int pushBufIdx;

	bool newPackageAvailable;

	HANDLE mutex;

	Series<T> * series;

	HRESULT SeriesPlotData();
};

template <typename T>
PlotImpl<T>::PlotImpl(Series<T> * series, int snapshotBufSize)
{
	this->series = series;
	this->snapshotBufSize = snapshotBufSize;

	timems = 1;
	timemsElapsed = 0;

	pushBufSel = 0;
	plotBufSel = 1;

	plotBufIdx = 0;

	newPackageAvailable = false;

    mutex = CreateMutex( 
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex
}

template <typename T>
PlotImpl<T>::~PlotImpl()
{
	CloseHandle(mutex);
}

template <typename T>
void PlotImpl<T>::Clear()
{
	WaitForSingleObject(mutex, INFINITE);	

	deques[0].clear();
	deques[1].clear();
	pushBufSel = 0;
	plotBufSel = 1;
	plotBufIdx = 0;

	series->Clear();

	ReleaseMutex(mutex);
}


template <typename T>
void PlotImpl<T>::SetTimeIntervalMS(int timems)
{
	if (timems < 0)
		return;

	this->timems = timems;
}

template <typename T>
int PlotImpl<T>::GetTimeIntervalMS()
{
	return timems;
}

template <typename T>
void PlotImpl<T>::SetTimeElapsedMS(int timems)
{
	if (timems < 0)
		return;

	this->timemsElapsed = timems;
}

template <typename T>
int PlotImpl<T>::GetTimeElapsedMS()
{
	return timemsElapsed;
}

template <typename T>
HRESULT PlotImpl<T>::PushData(double * data, int len)
{
	int ret = S_OK;

	if (len <= 0)
	{
		ret = -1;
		goto EXIT;
	}

	WaitForSingleObject(mutex, INFINITE);

	if (len >= (int)snapshotBufSize)
	{
		deques[pushBufSel].clear();
		for (int i = 0; i < len; i++)
			deques[pushBufSel].push_back(data[i]);
	}
	else
	{
		unsigned size = deques[pushBufSel].size();
		if (size < snapshotBufSize)
			for (int i = 0; i < len; i++)
				deques[pushBufSel].push_back(data[i]);
	}

	ReleaseMutex(mutex);
EXIT:
	return ret;
}

template <typename T>
HRESULT PlotImpl<T>::PushPackage(double * data, int len)
{
	int ret = S_OK;

	if (len <= 0)
	{
		ret = -1;
		goto EXIT;
	}

	WaitForSingleObject(mutex, INFINITE);

	deques[pushBufSel].clear();
	for (int i = 0; i < len; i++)
		deques[pushBufSel].push_back(data[i]);

	newPackageAvailable = true;
	//for (int i = 0; i < len; i++)
	//	deques[pushBufSel].push_back(data[i]);

	//unsigned size = deques[pushBufSel].size();

	//if ( size > snapshotBufSize * 8 )
	//{
	//	for (unsigned int i = 0; i < snapshotBufSize; i++)
	//	{
	//		deques[pushBufSel][i] = deques[pushBufSel][size - snapshotBufSize + i];
	//	}
	//	deques[pushBufSel].resize(snapshotBufSize);
	//}

	ReleaseMutex(mutex);
EXIT:
	return ret;
}

template <typename T>
void PlotImpl<T>::PlotData()
{
	HRESULT res = SeriesPlotData();

	bool dataAvailable = false;

	if (FAILED(res))
	{
		WaitForSingleObject(mutex, INFINITE);

		if (
			newPackageAvailable ||
			(deques[pushBufSel].size() >= snapshotBufSize)
			)
		{
			newPackageAvailable = false;

			if (pushBufSel == 0)
			{
				pushBufSel = 1;
				plotBufSel = 0;
			}
			else
			{
				pushBufSel = 0;
				plotBufSel = 1;
			}

			plotBufIdx = 0;

			deques[pushBufSel].clear();

			dataAvailable = true;
		}

		ReleaseMutex(mutex);

		if (dataAvailable)
			SeriesPlotData();
		else
			series->PushVoid();
	}
}



class PlotPlayer : public SoraThread
{
public:
	void Threadfunc();

	PlotPlayer();
	void AddPlot(Plot * plot);
	//void AddWindow(CAnimWnd * window);
	void SetTimeIntervalMS(int timems);
private:
	std::vector<Plot *> plots;
	std::vector<CAnimWnd *> windows;
	std::vector<int> timeRecord;
	int timems;
};


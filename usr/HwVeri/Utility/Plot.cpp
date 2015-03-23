#include "Plot.h"
#include "Message.h"

template <>
void Series<double>::Clear()
{
	std::deque<bool>::iterator itrValid;
	std::deque<double>::iterator itr;

	for (itrValid = dequeValid.begin(); itrValid != dequeValid.end(); ++itrValid)
	{
		*itrValid = false;
	}

	for (itr = deque.begin(); itr != deque.end(); ++itr)
	{
		*itr = 0.0;
	}
	//deque.resize(size, 0);
	//dequeValid.resize(size, false);
}

template <>
void Series<COMPLEX>::Clear()
{
	std::deque<bool>::iterator itrValid;
	std::deque<COMPLEX>::iterator itr;

	for (itrValid = dequeValid.begin(); itrValid != dequeValid.end(); ++itrValid)
	{
		*itrValid = false;
	}

	for (itr = deque.begin(); itr != deque.end(); ++itr)
	{
		*itr = COMPLEX(0, 0);
	}
	//deque.resize(size, COMPLEX(0,0));
	//dequeValid.resize(size, false);
}

DrawLine::DrawLine(CAnimWnd * wnd, HWND mainThreadWnd)
{
	this->wnd = wnd;
	this->color = RGB(255,255,255);
	this->mainThreadWnd = mainThreadWnd;
}

void DrawLine::SetColor(COLORREF color)
{
	this->color = color;
}

void DrawLine::Draw(std::deque<double> * deque, std::deque<bool> * dequeValid, unsigned size)
{
	if (!IsWindow(wnd->HWnd()))
		return;

	wnd->EraseCanvas();

	//printf("%x\n", wnd);

	int sizeDeque = deque->size();
	if (deque->size() < size)
		return;
	else
	{
		wnd->PlotLine(*deque, *dequeValid, color);
	}

	if (mainThreadWnd)
	{
		::PostMessage(mainThreadWnd, WM_UPDATE_CANVAS, (WPARAM)wnd, 0);
	}
	else
		wnd->UpdateCanvas();
}


DrawDots::DrawDots(CAnimWnd * wnd, HWND mainThreadWnd)
{
	this->wnd = wnd;
	this->color = RGB(255,255,255);
	this->mainThreadWnd = mainThreadWnd;
}

void DrawDots::SetColor(COLORREF color)
{
	this->color = color;
}

void DrawDots::Draw(std::deque<COMPLEX> * deque, std::deque<bool> * dequeValid, unsigned int size)
{
	if (!IsWindow(wnd->HWnd()))
		return;

	wnd->EraseCanvas();

	int scale = 0;

	for (unsigned int i = 0; i < deque->size(); i++)
	{
		if ((*dequeValid)[i] == false)
			continue;
		int red = GetRValue(this->color) * scale / deque->size();
		int green = GetGValue(this->color) * scale / deque->size();
		int blue = GetBValue(this->color) * scale / deque->size();
		COLORREF color = RGB(red, green, blue);
		COMPLEX complex = (*deque)[i];
		wnd->PlotDots(&complex, 1, color);
		scale++;
	}

	if (mainThreadWnd)
	{
		::PostMessage(mainThreadWnd, WM_UPDATE_CANVAS, (WPARAM)wnd, 0);
	}
	else
		wnd->UpdateCanvas();
	//**wnd->DoYield();
}

template <>
HRESULT PlotImpl<double>::SeriesPlotData()
{
	int size = deques[plotBufSel].size();
	int size2 = deques[pushBufSel].size();
	if (plotBufIdx < deques[plotBufSel].size())
	{
		series->PushData(deques[plotBufSel][plotBufIdx++]);
		return S_OK;
	}

	return -1;
}

template <>
HRESULT PlotImpl<COMPLEX>::SeriesPlotData()
{
	unsigned int size = deques[plotBufSel].size();

	if ( (size > 0) && (plotBufIdx < (size - 1)) ) // at least two double ramains
	{

		COMPLEX complex;
		complex.re = deques[plotBufSel][plotBufIdx++];
		complex.im = deques[plotBufSel][plotBufIdx++];

		series->PushData(complex);

		return S_OK;
	}
	return -1;
}


PlotPlayer::PlotPlayer()
{
	timems = 1;
}

void PlotPlayer::SetTimeIntervalMS(int timems)
{
	if (timems <= 0)
		return;
	this->timems = timems;
}

void PlotPlayer::AddPlot(Plot * plot)
{
	plots.push_back(plot);
	timeRecord.push_back(0);
}

void PlotPlayer::Threadfunc()
{
	while(!CheckStatus())
	{
		::Sleep(15);

		for (unsigned int i = 0; i < plots.size(); i++)
		{
			plots[i]->PlotData();
		}
	}
}

template <>
void Series<double>::PushVoid()
{
	deque.push_back(0);
	deque.pop_front();
	dequeValid.push_back(false);
	dequeValid.pop_front();

	if (drawMethod)
		drawMethod->Draw(&deque, &dequeValid, size);	
}

template <>
void Series<COMPLEX>::PushVoid()
{
	deque.push_back(COMPLEX(0,0));
	deque.pop_front();
	dequeValid.push_back(false);
	dequeValid.pop_front();

	if (drawMethod)
		drawMethod->Draw(&deque, &dequeValid, size);	
}


//void PlotPlayer::AddWindow(CAnimWnd * window)
//{
//	windows.push_back(window);
//}
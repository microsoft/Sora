#include "stdafx.h"
#include <memory>
#include "TaskQueue.h"
#include "ChannelOpenedDots.h"

using namespace std;
using namespace SoraDbgPlot::Task;

shared_ptr<BaseProperty> ChannelOpenedDots::CreatePropertyPage()
{
	return ChannelOpened::CreatePropertyPage();
}


void ChannelOpenedDots::Task_ProcessDrawDotsParam(std::shared_ptr<DrawDotsParam> param, bool bDrawOrGet)
{
	if ( (bDrawOrGet == true) && (param->_bMaxInitialized == false) )
		return;

	if (bDrawOrGet)
	{
		this->_latestIndex = param->_index;
		this->_range = param->_range;
	}

	if (bDrawOrGet == false)
	{
		unsigned long long timestamp;
		bool succ = this->IndexToTimestamp(param->_index, timestamp);
		if (succ)
		{
			if (!param->_bTimeStampTaken)
			{
				param->_timeStamp = timestamp;
				param->_bTimeStampTaken = true;
			}
			else
			{
				param->_timeStamp = max(param->_timeStamp, timestamp);
			}
		}
	}

	Graphics * g = param->_g;
	CRect rect = param->_rect;
	size_t start = param->_index;
	size_t size = param->_range;
	bool luminescence = param->_luminescence;
	double dispMax = param->_dispMaxValue;

	CRect rectClient = rect;

	double oldRange = dispMax * 2;
	int newRangeX = rectClient.Width();
	int newRangeY = rectClient.Height();

	Gdiplus::Color color;
	color.SetFromCOLORREF(this->_color);

	int redColor = color.GetRed();
	int greenColor = color.GetGreen();
	int blueColor = color.GetBlue();

	for (size_t i = 0; i < size; i++)
	{
		size_t mappedIdx = start + i;
		if (mappedIdx >= _ringBuffer->Size() || mappedIdx < 0)
			continue;
		COMPLEX16 y = (*_ringBuffer)[mappedIdx];

		if (bDrawOrGet)
		{
			Gdiplus::REAL re = float((y.re - (-dispMax)) * newRangeX / oldRange + rectClient.left);
			Gdiplus::REAL im = float((rectClient.bottom - (y.im - (-dispMax)) * newRangeY / oldRange));


			int alphaDot;
			if (luminescence)
				alphaDot = 255 * (size - i) / size;
			else
				alphaDot = 255;

			Gdiplus::Color colorFrame(alphaDot, redColor, greenColor, blueColor);

			Pen pen(colorFrame);

			g->DrawLine(&pen, re-1.0f, im, re+1.0f, im);
			g->DrawLine(&pen, re, im-1.0f, re, im+1.0f);
		}
		else
		{
			if (!param->_bMaxInitialized)
			{
				param->_dispMaxValue = y.re * y.re + y.im * y.im;
				param->_bMaxInitialized = true;
			}
			else
			{
				param->_dispMaxValue = max(param->_dispMaxValue, y.re * y.re + y.im * y.im);
			}
		}
	}

	if (bDrawOrGet == false)
		if (param->_dispMaxValue > 0)
		{
			param->_dispMaxValue = sqrt(param->_dispMaxValue);
		}
}


std::shared_ptr<SoraDbgPlot::Task::TaskSimple> ChannelOpenedDots::TaskProcessData(std::shared_ptr<DrawDotsParam> param, bool bDrawOrGet)
{
	auto SThis = dynamic_pointer_cast<ChannelOpenedDots, AsyncObject>(shared_from_this());
	return make_shared<TaskSimple>(TaskQueue(), [SThis, param, bDrawOrGet](){
		SThis->Task_ProcessDrawDotsParam(param, bDrawOrGet);
	});
}

const wchar_t * ChannelOpenedDots::GetTypeName()
{
	return L"Dots Channel";
}

bool ChannelOpenedDots::Export(const CString & filename, bool bAll)
{
	auto SThis = dynamic_pointer_cast<ChannelOpenedDots, AsyncObject>(shared_from_this());
	this->DoLater([SThis, filename, bAll](){
		FILE * fp;
		errno_t ret = _wfopen_s(&fp, filename, L"wb");

		if (ret == 0)
		{
			char * digitBuf = new char[128];

			if (bAll)
			{
				SThis->_ringBuffer->Export([fp, digitBuf](const COMPLEX16 * ptr, size_t length){
					while(length > 0)
					{
						fprintf(fp, "%d %d\r\n", (*ptr).re, (*ptr).im);
						ptr++;
						length --;
					}
				});
			}
			else
			{
				size_t start = SThis->_latestIndex;
				size_t length = SThis->_range;
				length = min(length, SThis->_ringBuffer->Size());

				SThis->_ringBuffer->ExportRange(start, length, [fp, digitBuf](const COMPLEX16 * ptr, size_t length){
					while(length > 0)
					{
						fprintf(fp, "%d %d\r\n", (*ptr).re, (*ptr).im);
						ptr++;
						length --;
					}
				});		
			}

			delete [] digitBuf;

			fclose(fp);
		}	
	});

	return true;
}

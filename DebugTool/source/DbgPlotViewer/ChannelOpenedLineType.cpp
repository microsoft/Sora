#include "stdafx.h"
#include <Windows.h>
#include "ChannelOpenedLineType.h"
#include "Event.h"
#include "TaskQueue.h"
#include "HelperFunc.h"

using namespace std;
using namespace SoraDbgPlot::Task;

ChannelOpenedLineType::ChannelOpenedLineType()
{
	_drawUsingSample = true;	// defaut using sample
}

shared_ptr<SoraDbgPlot::Task::TaskSimple> ChannelOpenedLineType::LineDataTask(std::shared_ptr<LineDataParam> param)
{
	auto shared_me = dynamic_pointer_cast<ChannelOpenedLineType, AsyncObject>(shared_from_this());
	return make_shared<TaskSimple>(TaskQueue(), [shared_me, param](){
		shared_me->ProcessDrawLineParam(param);
	});
}

void ChannelOpenedLineType::ProcessDrawLineParam(std::shared_ptr<LineDataParam> param)
{
	size_t clientWidth = param->_clientWidth;
	size_t start = param->_index;
	size_t size = param->_range;

	bool bDrawDot = (size > 0) && (clientWidth / size >= 5);
	bool bDrawType2 = (size > 0) && (clientWidth / size >= 1);

	int width = clientWidth;

	if (width == 0)
		return;

	param->_color = Color();

	if (!bDrawType2)
	{
		bool firstPoint = true;
		for (int i = 0; i < width; i++)
		{
			size_t mappedIdx = start + size - 1 - i * size / width;
			if (mappedIdx >= _ringBuffer->Size())
				continue;

			int y = (*_ringBuffer)[mappedIdx];
			param->_list.push_back(y);
		}
	}

	if (bDrawType2)
	{
		for (size_t i = 0; i < size; i++)
		{
			if (i + start >= _ringBuffer->Size())
				break;

			int y = (*_ringBuffer)[i + start];
			param->_list.push_back(y);
		}
	}
}

std::shared_ptr<SoraDbgPlot::Task::TaskSimple> ChannelOpenedLineType::TaskProcessData(std::shared_ptr<DrawLineParam> param, bool bDrawOrGet)
{
	auto SThis = dynamic_pointer_cast<ChannelOpenedLineType, AsyncObject>(shared_from_this());
	return make_shared<TaskSimple>(TaskQueue(), [SThis, param, bDrawOrGet](){
		SThis->Task_ProcessDrawLineParam(param, bDrawOrGet);
		//TRACE1("task process data %d\n", bDrawOrGet ? 1 : 0);
	});
}

void ChannelOpenedLineType::DrawUsingSample(bool bUseSample)
{
	_drawUsingSample = bUseSample;
}

void ChannelOpenedLineType::ProcessDrawLineParam(std::shared_ptr<DrawLineParam> param, bool bDrawOrGet)
{
	auto shared_me = dynamic_pointer_cast<ChannelOpenedLineType, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([shared_me, param, bDrawOrGet](){
		shared_me->Task_ProcessDrawLineParam(param, bDrawOrGet);
	});
}

void ChannelOpenedLineType::DrawLine(Gdiplus::Graphics * g, const Pen* pen, float x1, float y1, float x2, float y2, float maxY, float minY)
{
	float deltaY = y2 - y1;
	float deltaX = x2 - x1;
	float baseX = x1;
	float baseY = y1;

	y1 = min(maxY, y1);
	y1 = max(minY, y1);
	if (deltaY != 0.0)
		x1 = (y1 - baseY) * deltaX / deltaY + baseX;

	y2 = min(maxY, y2);
	y2 = max(minY, y2);
	if (deltaY != 0.0)
		x2 = (y2 - baseY) * deltaX / deltaY + baseX;

	g->DrawLine(pen, x1, y1, x2, y2);
}

void ChannelOpenedLineType::Task_ProcessDrawLineParam(std::shared_ptr<DrawLineParam> param, bool bDrawOrGet)
{
	if ( (bDrawOrGet == true) && (param->_bMaxMinInitialized == false) )
		return;

	CRect rect =  param->_rect;
	size_t start = IndexToSize(param->_index);
	size_t size = RangeToSize(param->_range);

	if (bDrawOrGet)
	{
		this->_latestIndex = start;
		this->_range = size;
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

	bool bDrawDot = (size > 0) && (rect.Width() / size >= 5);
	bool bDrawType2 = (size > 0) && (rect.Width() / size >= 1);

	int width = rect.Width();

	if (width == 0)
		return;

	Gdiplus::REAL lastDispX, lastDispY;

	Gdiplus::Color color;
	color.SetFromCOLORREF(this->Color());

	Gdiplus::Color colorLine;

	if (bDrawDot)
		colorLine = Gdiplus::Color(64, color.GetR(), color.GetG(), color.GetB());
	else
		colorLine = Gdiplus::Color(255, color.GetR(), color.GetG(), color.GetB());

	Pen penLine(colorLine);

	if (!bDrawType2)		// more dots than pixels
	{
		if (_drawUsingSample)
		{
			bool firstPoint = true;
			for (int i = 0; i < width; i++)
			{
				size_t mappedIdx = start + size - 1 - i * size / width;
				if (mappedIdx >= _ringBuffer->Size())
					continue;

				int y = (*_ringBuffer)[mappedIdx];
				double dataT = ::TransformCoordinate(y, param->_bIsLog);

				if (bDrawOrGet)
				{
					Gdiplus::REAL dataY = this->GetClientY(dataT, rect, param->_dispMaxValue, param->_dispMinValue);
					Gdiplus::REAL dataX = (Gdiplus::REAL)(i + rect.left);

					if (!firstPoint)
						this->DrawLine(param->_g, &penLine, lastDispX, lastDispY, dataX, dataY, (float)rect.bottom, (float)rect.top);
					else
						firstPoint = false;

					lastDispX = dataX;
					lastDispY = dataY;
				}
				else
				{
					if (!param->_bMaxMinInitialized)
					{
						param->_dispMaxValue = dataT;
						param->_dispMinValue = dataT;
						param->_bMaxMinInitialized = true;
					}
					else
					{
						param->_dispMaxValue = max(param->_dispMaxValue, dataT);
						param->_dispMinValue = min(param->_dispMinValue, dataT);
					}
				}
			}

		}
		else
		{
			int rectWidth = max(0, rect.Width() - 1);

			float maxValue;
			float minValue;
			float lastX = -1.0;
			float lastY;

			bool firstPoint = true;
			Gdiplus::REAL dataY;
			Gdiplus::REAL dataX;

			for (size_t i = 0; i < size; i++)
			{
				if (i + start >= _ringBuffer->Size())
					break;

				int y = (*_ringBuffer)[i + start];
				double dataT = ::TransformCoordinate(y, param->_bIsLog);
				dataY = GetClientY(dataT, rect, param->_dispMaxValue, param->_dispMinValue);
				dataX = (Gdiplus::REAL)((size - i - 1) * rectWidth / (size - 1) + rect.left);

				if (bDrawOrGet)
				{
					if (dataX != lastX)
					{
						if (lastX != -1.0)
						{
							this->DrawLine(param->_g, &penLine, lastX, minValue, lastX, maxValue, (float)rect.bottom, (float)rect.top);
							this->DrawLine(param->_g, &penLine, lastX, lastY, dataX, dataY, (float)rect.bottom, (float)rect.top);
						}

						maxValue = dataY;
						minValue = dataY;
						lastX =  dataX;
					}
					else
					{
						maxValue = max(dataY, maxValue);
						minValue = min(dataY, minValue);
					}

					lastY = dataY;
				}
				else
				{
					if (!param->_bMaxMinInitialized)
					{
						param->_dispMaxValue = dataT;
						param->_dispMinValue = dataT;
						param->_bMaxMinInitialized = true;
					}
					else
					{
						param->_dispMaxValue = max(param->_dispMaxValue, dataT);
						param->_dispMinValue = min(param->_dispMinValue, dataT);
					}
				}
			}

			if (lastX != -1.0)
			{
				this->DrawLine(param->_g, &penLine, lastX, minValue, lastX, maxValue, (float)rect.bottom, (float)rect.top);
				this->DrawLine(param->_g, &penLine, lastX, lastY, dataX, dataY, (float)rect.bottom, (float)rect.top);
			}
		}
	}

	if (bDrawType2)
	{
		Pen penDot(color, 1.5);
		int rectWidth = max(0, rect.Width() - 1);

		if (size == 1)
		{
			if (start < _ringBuffer->Size())
			{
				int y = (*_ringBuffer)[start];
				double dataT = ::TransformCoordinate(y, param->_bIsLog);
				if (bDrawOrGet)
				{
					Gdiplus::REAL dataY = GetClientY(dataT, rect, param->_dispMaxValue, param->_dispMinValue);
					Gdiplus::REAL dataX = (Gdiplus::REAL)(rectWidth / 2 + rect.left);
					this->DrawDot(param->_g, dataX, dataY, penDot);
				}
				else
				{
					if (!param->_bMaxMinInitialized)
					{
						param->_dispMaxValue = dataT;
						param->_dispMinValue = dataT;
						param->_bMaxMinInitialized = true;
					}
					else
					{
						param->_dispMaxValue = max(param->_dispMaxValue, dataT);
						param->_dispMinValue = min(param->_dispMinValue, dataT);
					}			
				}
			}

		}
		else
		{
			bool firstPoint = true;
			for (size_t i = 0; i < size; i++)
			{
				if (i + start >= _ringBuffer->Size())
					break;

				int y = (*_ringBuffer)[i + start];
				double dataT = ::TransformCoordinate(y, param->_bIsLog);
				Gdiplus::REAL dataY = GetClientY(dataT, rect, param->_dispMaxValue, param->_dispMinValue);
				Gdiplus::REAL dataX = (Gdiplus::REAL)((size - i - 1) * rectWidth / (size - 1) + rect.left);

				if (bDrawDot)
					this->DrawDot(param->_g, dataX, dataY, penDot, i == size - 1 ? 1 : 0);

				if (bDrawOrGet)
				{
					if (firstPoint)
					{
						firstPoint = false;
					}
					else
					{
						this->DrawLine(param->_g, &penLine, lastDispX, lastDispY, dataX, dataY, (float)rect.bottom, (float)rect.top);
					}
					lastDispX = dataX;
					lastDispY = dataY;
				}
				else
				{
					if (!param->_bMaxMinInitialized)
					{
						param->_dispMaxValue = dataT;
						param->_dispMinValue = dataT;
						param->_bMaxMinInitialized = true;
					}
					else
					{
						param->_dispMaxValue = max(param->_dispMaxValue, dataT);
						param->_dispMinValue = min(param->_dispMinValue, dataT);
					}
				}
			}
		}
	}
}

Gdiplus::REAL ChannelOpenedLineType::GetClientY(double y, const CRect & clientRect, double dispMaxValue, double dispMinValue)
{
	double rangeOld = dispMaxValue - dispMinValue;
	int rangeNew = clientRect.bottom - clientRect.top;
	return float(clientRect.bottom - 
		(y - dispMinValue) * rangeNew / rangeOld);
}

void ChannelOpenedLineType::DrawDot(Graphics * g, double x, double y, const Pen & pen, int type)
{
	int xx = (int)x;
	int yy = (int)y;

	switch (type)
	{
	case 0:
		g->DrawLine(&pen, xx - 3, yy, xx + 3, yy);
		g->DrawLine(&pen, xx, yy - 3, xx, yy + 3);
		break;
	case 1:
		g->DrawLine(&pen, xx, yy, xx + 3, yy);
		g->DrawLine(&pen, xx, yy - 3, xx, yy + 3);
		break;
	default:
		;
	}
}

size_t ChannelOpenedLineType::RangeToSize(size_t range)
{
	return range;	
}


size_t ChannelOpenedLineType::IndexToSize(size_t index)
{
	return index;
}

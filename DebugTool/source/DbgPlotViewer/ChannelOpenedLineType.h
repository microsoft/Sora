#pragma once

#include "ChannelOpenedFixedLengthType.h"
#include "Event.h"
#include "TaskQueue.h"
#include "TaskSimple.h"
#include "FrameWithSizeFilter.h"
#include "RingBufferWithTimeStamp.h"

struct DrawLineParam
{
	Graphics * _g;
	bool _bIsLog;
	CRect _rect;
	double _dispMaxValue;
	double _dispMinValue;
	size_t _index;
	size_t _range;
	bool _bMaxMinInitialized;
	unsigned long long _timeStamp;
	bool _bTimeStampTaken;
};

struct LineInfoTask
{
	size_t size;
};

struct LineDataParam
{
	size_t _index;
	size_t _range;
	COLORREF _color;
	size_t _clientWidth;
	std::list<int> _list;
};

class ChannelOpenedLineType : public ChannelOpenedFixedLengthType<int>
{
public:
	ChannelOpenedLineType();
	std::shared_ptr<SoraDbgPlot::Task::TaskSimple> LineInfoTask(std::shared_ptr<LineInfoTask>);
	std::shared_ptr<SoraDbgPlot::Task::TaskSimple> LineDataTask(std::shared_ptr<LineDataParam>);
	void DrawUsingSample(bool bUseSample);

	void ProcessDrawLineParam(std::shared_ptr<DrawLineParam> param, bool bDrawOrGet);
	std::shared_ptr<SoraDbgPlot::Task::TaskSimple> TaskProcessData(std::shared_ptr<DrawLineParam>, bool bDrawOrGet);


protected:
	virtual size_t RangeToSize(size_t range);
	virtual size_t IndexToSize(size_t index);

private:
	void ProcessDrawLineParam(std::shared_ptr<LineDataParam> param);
	void ChannelOpenedLineType::Task_ProcessDrawLineParam(std::shared_ptr<DrawLineParam> param, bool bDrawOrGet);
	Gdiplus::REAL GetClientY(double y, const CRect & clientRect, double dispMaxValue, double dispMinValue);
	void DrawDot(Graphics * g, double x, double y, const Pen & pen, int type = 0);
	void DrawLine(Gdiplus::Graphics * g, const Pen* pen, float x1, float y1, float x2, float y2, float maxY, float minY);

	bool _drawUsingSample;
};

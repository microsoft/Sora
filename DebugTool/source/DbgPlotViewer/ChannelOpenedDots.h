#pragma once

#include "ChannelOpenedFixedLengthType.h"
#include "Event.h"
#include "TaskQueue.h"
#include "TaskSimple.h"
#include "FrameWithSizeFilter.h"
#include "RingBufferWithTimeStamp.h"

#pragma once

#ifndef _SORA_
typedef struct tagComplex16
{
	short re;
	short im;
} COMPLEX16;
#endif

struct DrawDotsParam
{
	Graphics * _g;
	bool _bIsLog;
	CRect _rect;
	double _dispMaxValue;
	size_t _index;
	size_t _range;
	bool _luminescence;
	bool _bMaxInitialized;
	bool _bTimeStampTaken;
	unsigned long long _timeStamp;
};

class ChannelOpenedDots : public ChannelOpenedFixedLengthType<COMPLEX16>
{
public:

	std::shared_ptr<SoraDbgPlot::Task::TaskSimple> TaskProcessData(std::shared_ptr<DrawDotsParam>, bool bDrawOrGet);

	virtual std::shared_ptr<BaseProperty> CreatePropertyPage();
	virtual const wchar_t * GetTypeName();
	virtual bool Export(const CString &, bool bAll);

private:
	void Task_ProcessDrawDotsParam(std::shared_ptr<DrawDotsParam> param, bool bDrawOrGet);
};

#pragma once
#include "ChannelOpenedLineType.h"
#include "TaskQueue.h"

class ChannelOpenedSpectrum : public ChannelOpenedLineType
{
public:
	ChannelOpenedSpectrum();
	virtual std::shared_ptr<BaseProperty> CreatePropertyPage();
	virtual std::shared_ptr<SoraDbgPlot::Task::TaskSimple> TaskUpdateDataSize(std::shared_ptr<size_t>);
	virtual const wchar_t * GetTypeName();
	virtual bool Export(const CString &, bool bAll);

protected:
	virtual size_t RangeToSize(size_t range);
	virtual size_t IndexToSize(size_t index);
};

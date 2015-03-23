#pragma once
#include "ChannelOpenedLineType.h"
#include "TaskQueue.h"

class ChannelOpenedLine : public ChannelOpenedLineType
{
public:
	virtual std::shared_ptr<BaseProperty> CreatePropertyPage();
	virtual const wchar_t * GetTypeName();
	virtual bool Export(const CString &, bool bAll);
};

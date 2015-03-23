#pragma once

#include <memory>
#include "ChannelOpenedTextType.h"
#include "FrameWithSizeFilter.h"
#include "ILog.h"
#include "TaskSimple.h"
#include "TempBuffer.h"

class ChannelOpenedLog : public ChannelOpenedTextType
{
private:
	static int logObjIdx;

public:
	ChannelOpenedLog();
	~ChannelOpenedLog();

	virtual std::shared_ptr<SoraDbgPlot::Task::TaskSimple> TaskGetSize(std::shared_ptr<size_t>);
	virtual const wchar_t * GetTypeName();
	virtual bool Export(const CString &, bool bAll);

protected:
	virtual void WriteData(const char * data, size_t length);
	virtual size_t DataSize();
	virtual char * GetData(size_t index, bool bFromOldest);
	virtual void ClearData();

private:
	ILog * _logObj;
	COLORREF _color;
	SoraDbgPlot::FrameWithSizeInfoWriter<char> * _writeFilter;

	SoraDbgPlot::Buffer::TempBuffer _newLineFilterBuffer;
};

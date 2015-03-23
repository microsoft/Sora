#pragma once

#include <memory>
#include "TaskQueue.h"
#include "ChannelOpenedTextType.h"
#include "SubPlotWnd.h"
#include "FrameWithSizeFilter.h"
#include "RingBufferWithTimeStamp.h"
#include "TempBuffer.h"

class ChannelOpenedText : public ChannelOpenedTextType
{
public:
	ChannelOpenedText();
	~ChannelOpenedText();
	void SetRect(const CRect & rect);
	SubPlotWnd * ChannelOpenedText::CreateSubPlotWnd();
	void CloseSubPlotWnd();
	void UpdateSubPlotWnd();
	void SeekTimeStamp(unsigned long long timestamp);
	virtual HRESULT AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent);
	virtual HRESULT LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement);
	virtual void GetRect(CRect & rect);
	virtual const wchar_t * GetTypeName();
	virtual bool Export(const CString &, bool bAll);

	//virtual void CloseChannel();

protected:
	virtual void WriteData(const char * data, size_t length);
	virtual std::shared_ptr<SoraDbgPlot::Task::TaskSimple> TaskGetSize(std::shared_ptr<size_t>);
	virtual size_t DataSize();
	virtual char * GetData(size_t index, bool bFromOldest);
	virtual void OnColorUpdated();
	virtual void ClearData();

private:
	SoraDbgPlot::FrameWithSizeInfoWriter<char> * _filter;
	RingBufferWithTimeStamp<char> * _ringBuffer;
	SubPlotWnd * _plotWnd;
	CRect _rect;
	size_t _latestTimeIdx;

	SoraDbgPlot::Buffer::TempBuffer _newLineFilterBuffer;
};

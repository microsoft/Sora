#pragma once

#include <Windows.h>
#include <vector>
#include "SeriesTextType.h"
#include "FrameWithSizeFilter.h"
#include "RingBufferWithTimeStamp.h"
#include "SubPlotWnd.h"

class TextSeriesProp : public TextTypeSeriesProp
{
public:	// serialize
	bool replaceMode;
	bool isLog;

	//bool colorIsSet;
	COLORREF color;

public:
	TextSeriesProp();
	~TextSeriesProp();
	void Close();
	virtual BaseProperty * GetPropertyPage();
	virtual HRESULT CreateElementSeries(IXMLDOMDocument *pDom, IXMLDOMElement *pe);
	//virtual void UpdateView();

	CRITICAL_SECTION csLogBuf;
	CString logBuf;

	// data management
public:
	virtual void Write(const void * ptr, size_t length);
	size_t DataSize();

	char * GetData(size_t index);
	void SeekTimeStamp(const std::vector<unsigned long long> & vecTimeStamp);
	void SeekDataRanage(size_t idx, size_t range);
	void UpdateSubWnd();

protected:
	virtual void ClearData();

private:
	SoraDbgPlot::FrameWithSizeInfoWriter<char> * _writeFilter;
	RingBufferWithTimeStamp<char> * _ringBuffer;

	size_t _latestTimeIdx;
	size_t _dataRange;

public:
	SubPlotWnd * CreateSubPlotWnd();
public:
	virtual bool Export(const CString &, bool bAll);
};

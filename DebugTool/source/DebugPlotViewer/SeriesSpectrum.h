#pragma once

#include "SeriesObj.h"
#include "FrameWithSizeFilter.h"
#include "RingBufferWithTimeStamp.h"
#include "SeriesLineType.h"

class SpectrumSeriesProp : public LineTypeSeriesProp
{
public:	// serialize
	//bool colorIsSet;
	COLORREF color;

public:
	int maxValue;
	int minValue;
	int spectrumDataSize;

public:
	SpectrumSeriesProp();
	~SpectrumSeriesProp();
	void Close();
	virtual BaseProperty * GetPropertyPage();
	virtual HRESULT CreateElementSeries(IXMLDOMDocument *pDom, IXMLDOMElement *pe);
	virtual void Write(const void * ptr, size_t length);
	
	void Draw(Graphics * g, const CRect & rect, size_t start, size_t size);
	void DrawDot(Graphics * g, double x, double y, const Pen & pen);

	size_t DataSize();

	bool GetTimeStamp(size_t index, unsigned long long & out);
	virtual bool GetData(size_t index, int & out);

protected:
	virtual void ClearData();

	// data management
private:
	SoraDbgPlot::FrameWithSizeInfoWriter<int> * _writeFilter;
	RingBufferWithTimeStamp<int> * _ringBuffer;

public:
	virtual bool GetMaxMinRange(size_t in, size_t & out);

private:
	virtual void OnSmInfoSet(SharedSeriesInfo * sharedSeriesInfo);
	virtual void OnSmInfoRemoved();

public:
	size_t GetSpectrumDataWidth();
private:
	void SetSpectrumDataWidth(size_t dataWidth);
	size_t _dataWidth;
public:
	virtual bool Export(const CString &, bool bAll);
};


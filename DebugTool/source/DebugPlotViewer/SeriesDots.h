#pragma once

#include "SeriesObj.h"
#include "SeriesBitmapType.h"

class DotsSeriesProp : public BitmapTypeSeriesProp
{
public:	// serialize
	//bool colorIsSet;
	COLORREF color;

public:
	DotsSeriesProp();
	~DotsSeriesProp();
	void Close();
	virtual BaseProperty * GetPropertyPage();
	virtual HRESULT CreateElementSeries(IXMLDOMDocument *pDom, IXMLDOMElement *pe);

public:
	void Draw(Graphics * g, const CRect & rect, size_t start, size_t size, bool luminescence, double disMax);
	bool CalcMax(const CRect & rect, size_t start, size_t size, double & max);


	//data
public:
	virtual void Write(const void * ptr, size_t length);
protected:
	virtual void ClearData();

private:
	SoraDbgPlot::FrameWithSizeInfoWriter<COMPLEX16> * _writeFilter;
	RingBufferWithTimeStamp<COMPLEX16> * _ringBuffer;

public:
	virtual size_t DataSize();
	virtual bool GetTimeStamp(size_t index, unsigned long long & out);
public:
	virtual bool Export(const CString &, bool bAll);
};

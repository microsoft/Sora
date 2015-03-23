#pragma once

#include "SeriesTextType.h"

class LogSeriesProp : public TextTypeSeriesProp
{
public:
	LogSeriesProp();
	~LogSeriesProp();
	virtual BaseProperty * GetPropertyPage();
	virtual HRESULT CreateElementSeries(IXMLDOMDocument *pDom, IXMLDOMElement *pe);
	//virtual void UpdateView();
	virtual void Close();

	COLORREF GetColor();
	void SetColor(COLORREF color);

	char * Record(int index);
	size_t RecordCount();

private:
	//bool colorIsSet;
	COLORREF color;
	//CRITICAL_SECTION csLogBuf;
	ILog * logObj;	

private:
	static int logObjIdx;
	SoraDbgPlot::FrameWithSizeInfoWriter<char> * _writeFilter;

protected:
	virtual void Write(const void * ptr, size_t length);
public:
	virtual size_t DataSize();
	virtual char * GetData(size_t);
protected:
	virtual void ClearData();
public:
	virtual bool Export(const CString &, bool bAll);
};

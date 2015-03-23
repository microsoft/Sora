#pragma once

#include <memory>
#include "SharedChannel.h"
#include "TaskSimple.h"
#include "PropObject.h"
#include "BaseProperty.h"

class ChannelOpened : public PropObject
{
public:
	ChannelOpened();
	~ChannelOpened();

	void AttatchSharedChannelSync(std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel>);
	void DeattatchSharedChannelSync();
	int Id();
	int Pid();
	int SpectrumDataSize();
	std::wstring Name();
	ChannelType Type();
	COLORREF Color();
	void Release();
	bool IsAttatched();
	virtual void Clear();

	virtual bool Export(const CString &, bool bAll);

	virtual std::shared_ptr<SoraDbgPlot::Task::TaskSimple> TaskUpdateData(std::shared_ptr<bool>);
	virtual void WriteData(const char * data, size_t length);
	virtual void GetRect(CRect & rect);

	void SetColor(COLORREF color);

	void TaskFunc_Release();

	virtual HRESULT CreateXmlElement(IXMLDOMDocument *pDom, IXMLDOMElement *pe);
	virtual HRESULT AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent);
	virtual HRESULT LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement);

	void SetOpenState(bool bOpened);
	bool GetOpenState();

private:
	std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel> _sharedChannel;
	volatile long _openCount;
	bool _bAttatched;

protected:
	int _id;
	int _pid;
	ChannelType _type;
	std::wstring _name;

protected:
	COLORREF _color;
	int _spectrumSize;

protected:
	std::shared_ptr<BaseProperty> CreatePropertyPage();
	virtual void OnColorUpdated();
	virtual void ClearData() = 0;
};

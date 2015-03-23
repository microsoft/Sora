#pragma once

#include <memory>
#include <functional>
#include "BaseProperty.h"
#include "TaskQueue.h"
#include "TaskSimple.h"
#include "Event.h"
#include "AsyncObject.h"

#import <msxml6.dll>

class PropObject : public AsyncObject
{
public:
	~PropObject();
	std::shared_ptr<BaseProperty> GetPropertyPage();
	void Later_PropertyPage(const std::function<void(std::shared_ptr<BaseProperty>)> f);
	SoraDbgPlot::Event::Event<BaseProperty *> EventPropertyPageChanged;
	void Clear();
	virtual HRESULT CreateXmlElement(IXMLDOMDocument *pDom, IXMLDOMElement *pe);
	virtual HRESULT LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement);
	virtual const wchar_t * GetTypeName();

protected:
	virtual std::shared_ptr<BaseProperty> CreatePropertyPage() = 0;
	std::shared_ptr<BaseProperty> _propertyPage;
};

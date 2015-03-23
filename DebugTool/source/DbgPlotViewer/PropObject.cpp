#include "stdafx.h"
#include "PropObject.h"

using namespace std;

PropObject::~PropObject()
{

}

shared_ptr<BaseProperty> PropObject::GetPropertyPage()
{
	shared_ptr<BaseProperty> propertyPage;
	auto SThis = dynamic_pointer_cast<PropObject, AsyncObject>(shared_from_this());

	TaskQueue()->DoTask([SThis, &propertyPage](){
		SThis->_propertyPage = SThis->CreatePropertyPage();
	});

	return _propertyPage;
}

void PropObject::Later_PropertyPage(const std::function<void(std::shared_ptr<BaseProperty>)> f)
{
	auto SThis = dynamic_pointer_cast<PropObject, AsyncObject>(shared_from_this());
	this->DoLater([SThis, f](){
		f(SThis->_propertyPage);
	});
}


void PropObject::Clear()
{
	auto SThis = dynamic_pointer_cast<PropObject, AsyncObject>(shared_from_this());

	auto clearTask = [SThis](){
		SThis->EventPropertyPageChanged.Reset();
	};

	this->DoLater(clearTask, clearTask);
}


HRESULT PropObject::CreateXmlElement(IXMLDOMDocument *pDom, IXMLDOMElement *pe)
{
	// not implemented
	assert(false);
	return S_OK;
}


HRESULT PropObject::LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement)
{
	assert(false);
	return S_OK;
}

const wchar_t * PropObject::GetTypeName()
{
	return L"PropObject";
}

#include "stdafx.h"
#include "HelperFunc.h"

double TransformCoordinate(double ori, bool isLog)
{
	if (!isLog)
		return ori;

	if (ori == 0.0)
		return 0.0;

	ori = ori > 0 ? ori : -ori;
	return log10(ori);
}

double ReverseTransformCoordinate(double ori, bool isLog)
{
	if (!isLog)
		return ori;

	return pow(10, ori);
}

HRESULT AppendChildToParent(IXMLDOMNode *pChild, IXMLDOMNode *pParent)
{
    HRESULT hr = S_OK;
    IXMLDOMNode *pChildOut = NULL;
    CHK_HR(pParent->appendChild(pChild, &pChildOut));

CleanUp:
    SAFE_RELEASE(pChildOut);
    return hr;
}

HRESULT CreateAndAddTextNode(IXMLDOMDocument *pDom, PCWSTR wszText, IXMLDOMNode *pParent)
{
    HRESULT hr = S_OK;    
    IXMLDOMText *pText = NULL;

    BSTR bstrText = SysAllocString(wszText);
    CHK_ALLOC(bstrText);

    CHK_HR(pDom->createTextNode(bstrText, &pText));
    CHK_HR(AppendChildToParent(pText, pParent));

CleanUp:
    SAFE_RELEASE(pText);
    SysFreeString(bstrText);
    return hr;
}

HRESULT CreateElement(IXMLDOMDocument *pXMLDom, PCWSTR wszName, IXMLDOMElement **ppElement)
{
    HRESULT hr = S_OK;
    *ppElement = NULL;

    BSTR bstrName = SysAllocString(wszName);
    CHK_ALLOC(bstrName);
    CHK_HR(pXMLDom->createElement(bstrName, ppElement));

CleanUp:
    SysFreeString(bstrName);
    return hr;
}

HRESULT CreateAndAddElementNode(IXMLDOMDocument *pDom, PCWSTR wszName, PCWSTR wszNewline, IXMLDOMNode *pParent, IXMLDOMElement **ppElement = NULL)

{
	HRESULT hr = S_OK;
	IXMLDOMElement* pElement = NULL;

	CHK_HR(CreateElement(pDom, wszName, &pElement));

	CHK_HR(CreateAndAddTextNode(pDom, wszNewline, pParent));

	CHK_HR(AppendChildToParent(pElement, pParent));

CleanUp:
	if (ppElement)
		*ppElement = pElement;  
	else
		SAFE_RELEASE(pElement); 

	return hr;
}

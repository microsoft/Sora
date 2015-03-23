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

void FormatCommaString(CString & str, double num, double resolution)
{
	if (num == 0.0f)
	{
		str = L"0.0";
		return;
	}

	bool bMinus = num < 0 ? true : false;

	if (num < 0)
		num = -num;

	double log1 = log(num) / log(1000.0);
	int level = (int)log1;

	double prefix = num / pow(1000.0, level);
	double prefixResolution = resolution / pow(1000.0, level);

	const wchar_t * unitTable[] = {L"n", L"u", L"m", L"", L"k", L"M", L"G"}; 

	if (level < -3)
	{
		str = L"0.0";
		return;
	}

	const wchar_t * unit = unitTable[level + 3];

	CString strFormat;
	int digit = max(1, int(ceil(-log10(prefixResolution))));
	strFormat.Format(L"%%s%%.%df%%s", digit);

	str.Format( strFormat,
		bMinus ? L"-" : L"",
		prefix,
		unit
		);
}

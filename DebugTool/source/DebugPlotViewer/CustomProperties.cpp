#include "stdafx.h"
#include "CustomProperties.h"

////////////////////////////////////////////////////////////////////////////////
// CSliderProp class

CSliderProp::CSliderProp(const CString& strName, long nValue, int rangeMin, int rangeMax, LPCTSTR lpszDescr, DWORD dwData) :
	CMFCPropertyGridProperty(strName, nValue, lpszDescr, dwData)
{
	if (rangeMin >= rangeMax)
	{
		rangeMin = 0;
		rangeMax = 100;
	}

	if (nValue < rangeMin)
	{
		nValue = rangeMin;
	}
	else if (nValue > rangeMax)
	{
		nValue =rangeMax;
	}

	this->rangeMin = rangeMin;
	this->rangeMax = rangeMax;
	m_varValue.lVal = nValue;
}

CWnd* CSliderProp::CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat)
{
	CPropSliderCtrl* pWndSlider = new CPropSliderCtrl(this, m_pWndList->GetBkColor());

	rectEdit.left += rectEdit.Height() + 5;

	pWndSlider->Create(WS_VISIBLE | WS_CHILD, rectEdit, m_pWndList, AFX_PROPLIST_ID_INPLACE);
	
	long pos = (m_varValue.lVal - this->rangeMin) * 100 / (this->rangeMax - this->rangeMin);
	
	pWndSlider->SetPos(pos);

	bDefaultFormat = TRUE;

	return pWndSlider;
}

BOOL CSliderProp::OnUpdateValue()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT_VALID(m_pWndList);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	long lCurrValue = m_varValue.lVal;

	CSliderCtrl* pSlider = (CSliderCtrl*) m_pWndInPlace;

	long value = (long) pSlider->GetPos();

	m_varValue = (long)
		(this->rangeMin + value * (this->rangeMax - this->rangeMin) / 100);

	if (lCurrValue != m_varValue.lVal)
	{
		m_pWndList->OnPropertyChanged(this);
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CPropSliderCtrl

CPropSliderCtrl::CPropSliderCtrl(CSliderProp* pProp, COLORREF clrBack)
{
	m_clrBack = clrBack;
	m_brBackground.CreateSolidBrush(m_clrBack);
	m_pProp = pProp;
}

CPropSliderCtrl::~CPropSliderCtrl()
{
}

BEGIN_MESSAGE_MAP(CPropSliderCtrl, CSliderCtrl)
	//{{AFX_MSG_MAP(CPropSliderCtrl)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_HSCROLL_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropSliderCtrl message handlers

HBRUSH CPropSliderCtrl::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	pDC->SetBkColor(m_clrBack);
	return m_brBackground;
}

void CPropSliderCtrl::HScroll(UINT /*nSBCode*/, UINT /*nPos*/)
{
	ASSERT_VALID(m_pProp);

	m_pProp->OnUpdateValue();
	m_pProp->Redraw();
}

// SeriesPropertySheet.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "SeriesPropertySheet.h"


// SeriesPropertySheet

IMPLEMENT_DYNAMIC(SeriesPropertySheet, CMFCPropertyGridCtrl)

SeriesPropertySheet::SeriesPropertySheet()
{

}

SeriesPropertySheet::~SeriesPropertySheet()
{
}


BEGIN_MESSAGE_MAP(SeriesPropertySheet, CMFCPropertyGridCtrl)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// SeriesPropertySheet message handlers




int SeriesPropertySheet::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMFCPropertyGridCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	this->AddProperty(
		new CMFCPropertyGridProperty( _T("Grid Size"), (_variant_t) 20, _T("Specifies the snapping grid size")));
	return 0;

	CMFCPropertyGridProperty * pProp = new CMFCPropertyGridProperty(_T("(Name)"), _T("IDD_ABOUT_BOX(dialog)"));
	pProp->Enable(FALSE);

	return 0;
}

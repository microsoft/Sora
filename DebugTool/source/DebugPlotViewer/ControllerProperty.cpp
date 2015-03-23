// ControllerProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "ControllerProperty.h"
#include "SeriesObj.h"
#include "AppMessage.h"
#include "AppSettings.h"

// ControllerProperty

IMPLEMENT_DYNAMIC(ControllerProperty, BaseProperty)

ControllerProperty::ControllerProperty()
{

}

ControllerProperty::~ControllerProperty()
{
}


BEGIN_MESSAGE_MAP(ControllerProperty, BaseProperty)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// ControllerProperty message handlers




int ControllerProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	DebuggingProcessProp * process = (DebuggingProcessProp *)this->target;


	CString strMode;
	if (process->traceMode == DebuggingProcessProp::MODE_TRACE_GRAPH)
		strMode.SetString(L"display data");
	else
		strMode.SetString(L"raw data");

	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(_T("Trace Type"), strMode.GetBuffer(), _T("Specifies the trace type of the track bar"));
	pProp->AddOption(_T("display data"));
	pProp->AddOption(_T("raw data"));
	pProp->AllowEdit(FALSE);

	this->AddProperty(pProp);

	//pProp = new CMFCPropertyGridProperty( _T("Threshold"), (_variant_t) process->threshold, _T("Specifies the threshold for the channel circular buffer"));
	//this->AddProperty(pProp);

	return 0;
}


void ControllerProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	// TODO: Add your specialized code here and/or call the base class

	DebuggingProcessProp * process = (DebuggingProcessProp *)this->target;

	if (wcscmp(pProp->GetName(), L"Trace Type") == 0)
	{
		_variant_t value = pProp->GetValue();
		BSTR bstr = value.bstrVal;
		if ( wcscmp(bstr, L"display data") == 0 )
		{
			process->traceMode = DebuggingProcessProp::MODE_TRACE_GRAPH;
		}
		else
		{
			process->traceMode = DebuggingProcessProp::MODE_TRACE_SOURCE;
		}
	}
	/*else if (wcscmp(pProp->GetName(), L"Threshold") == 0)
	{
		_variant_t value = pProp->GetValue();
		int threshold = value.intVal;
		threshold = max(0, threshold);
		threshold = min(::SettingGetSnapShotBufferSize(), threshold);
		if (value.intVal != threshold)
		{
			value.intVal = threshold;
			pProp->SetValue(value);
		}
		process->threshold = threshold;
		if (process->smProcess)
		{
			SharedProcessInfo * sharedProcessInfo = 
				(SharedProcessInfo *)process->smProcess->GetAddress();
			sharedProcessInfo->threshold = process->threshold;
		}
	}*/

	process->trackbarWnd->PostMessage(WM_APP, CMD_TRACE_TYPE_CHANGED, 0);

	return BaseProperty::OnPropertyChanged(pProp);
}


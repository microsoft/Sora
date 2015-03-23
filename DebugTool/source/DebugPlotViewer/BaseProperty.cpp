// BaseProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "BaseProperty.h"
#include "AppMessage.h"

// BaseProperty

IMPLEMENT_DYNAMIC(BaseProperty, CMFCPropertyGridCtrl)

BaseProperty::BaseProperty()
{
	this->target = 0;
}

BaseProperty::~BaseProperty()
{
}


BEGIN_MESSAGE_MAP(BaseProperty, CMFCPropertyGridCtrl)
	ON_WM_CREATE()
	ON_MESSAGE(WM_APP, OnApp)
END_MESSAGE_MAP()



// BaseProperty message handlers




int BaseProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMFCPropertyGridCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	this->EnableHeaderCtrl(FALSE);
	this->EnableDescriptionArea();
	this->SetVSDotNetLook();
	this->MarkModifiedProperties();

	return 0;
}

void BaseProperty::SetTarget(void * target)
{
	this->target = target;
}

void * BaseProperty::GetTarget()
{
	return this->target;
}

void BaseProperty::UpdatePropertyValue(const wchar_t * propertyName)
{
	return; // do nothing
}


CMFCPropertyGridProperty * BaseProperty::FindProperty(const wchar_t * name) const
{
	CMFCPropertyGridProperty * property = NULL;
	for (int i = 0; i < this->GetPropertyCount(); i++)
	{
		 CMFCPropertyGridProperty * aProperty = this->GetProperty(i);
		 if (wcscmp(aProperty->GetName(), name) == 0)
		 {
			 property = aProperty;
			 break;
		 }
	}
	return property;
}

LRESULT BaseProperty::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case CMD_UPDATE_PROPERTY_PANEL:
		this->UpdatePropertyValue((wchar_t *)lParam);
		break;
	}
	return 0;
}

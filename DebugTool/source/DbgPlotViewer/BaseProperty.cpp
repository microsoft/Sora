// BaseProperty.cpp : implementation file
//

#include "stdafx.h"
#include "BaseProperty.h"

// BaseProperty

IMPLEMENT_DYNAMIC(BaseProperty, CMFCPropertyGridCtrl)

BaseProperty::BaseProperty()
{
}

BaseProperty::~BaseProperty()
{
	_pvTaskQueue.Execute(true);
}


void BaseProperty::CloseProperty()
{
	this->OnCloseProperty();
}

BEGIN_MESSAGE_MAP(BaseProperty, CMFCPropertyGridCtrl)
	ON_WM_CREATE()
	ON_MESSAGE(WMME_EXE_TASKQUEUE, OnExeTaskQueue)
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


void BaseProperty::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	this->OnCloseProperty();
	EventClosed.Raise(this, true);

	CMFCPropertyGridCtrl::PostNcDestroy();
}

LRESULT BaseProperty::OnExeTaskQueue(WPARAM wParam, LPARAM lParam)
{
	_pvTaskQueue.Execute(false);

	return 0;
}

void BaseProperty::DoLater(std::function<void(bool)> f)
{
	_pvTaskQueue.Queue(f);
	HWND hWnd = m_hWnd;
	if (hWnd)
		::PostMessage(hWnd, WMME_EXE_TASKQUEUE, 0, 0);
}

#include "stdafx.h"
#include <memory>
#include "BitmapPlotWnd.h"
#include "PlotWindowOpenedLine.h"
#include "ChannelOpenedLine.h"
#include "ChannelOpenedText.h"
#include "TaskQueue.h"
#include "HelperFunc.h"
#include "PlotWindowLineTypeProperty.h"

using namespace std;
using namespace SoraDbgPlot::Task;

PlotWindowOpenedLine::PlotWindowOpenedLine(std::shared_ptr<ProcessOpened> process) :
	PlotWindowOpenedLineType(process)
{
}

PlotWindowOpenedLine::~PlotWindowOpenedLine()
{
	;
}

shared_ptr<BaseProperty> PlotWindowOpenedLine::CreatePropertyPage()
{
	return PlotWindowOpenedLineType::CreatePropertyPage();
}

bool PlotWindowOpenedLine::Accept(shared_ptr<ChannelOpened> channel, CPoint pointIn, CPoint & pointOut)
{
	bool bAccepted = false;
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedLine, AsyncObject>(shared_from_this());

	TaskQueue()->DoTask([&bAccepted, SThis, channel, &pointIn, &pointOut](){

		if (SThis->_processOpened == 0)
		{
			bAccepted = false;
			return;
		}

		if (SThis->_processOpened->Pid() != channel->Pid())
		{
			bAccepted = false;
			return;
		}

		if (SThis->_chMap.size() == 0)
		{
			bAccepted = false;
			return;
		}

		for (auto iterCh = SThis->_chMap.begin(); iterCh != SThis->_chMap.end(); ++iterCh)
		{
			if (*iterCh == channel)
			{
				bAccepted = false;
				return;
			}
		}

		if (dynamic_pointer_cast<ChannelOpenedLine, ChannelOpened>(channel) == 0 &&
			dynamic_pointer_cast<ChannelOpenedText, ChannelOpened>(channel) == 0 )
		{
			bAccepted = false;
			return;
		}

		if (SThis->_plotWnd == 0)
		{
			bAccepted = false;
			return;
		}

		pointOut = pointIn;
		auto bitmapPlotWnd = dynamic_cast<BitmapPlotWnd *>(SThis->_plotWnd);
		assert(bitmapPlotWnd);
		bitmapPlotWnd->MyScreenToClient(pointOut);

		bAccepted = true;
	});

	return bAccepted;
}

HRESULT PlotWindowOpenedLine::AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent)
{
	PlotWindowOpenedLineType::AppendXmlProperty(pDom, pParent);

	return S_OK;
}

HRESULT PlotWindowOpenedLine::LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement)
{
	PlotWindowOpenedLineType::LoadXmlElement(pElement);

	return S_OK;
}

const wchar_t * PlotWindowOpenedLine::GetTypeName()
{
	return L"Line Window";
}

bool PlotWindowOpenedLine::IsRangeSettingEnabled()
{
	return true;
}

void PlotWindowOpenedLine::OnMouseWheel(bool bIsUp)
{
	if (_propertyPage)
	{
		auto propertyPageLineType = dynamic_pointer_cast<PlotWindowLineTypeProperty, BaseProperty>(_propertyPage);
		propertyPageLineType->UpdateDataRange(_range);
	}
}

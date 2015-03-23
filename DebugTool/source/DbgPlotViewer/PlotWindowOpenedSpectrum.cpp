#include "stdafx.h"
#include <memory>
#include "PlotWindowOpenedSpectrum.h"
#include "ChannelOpened.h"
#include "ChannelOpenedSpectrum.h"
#include "ChannelOpenedText.h"
#include "TaskQueue.h"
#include "HelperFunc.h"
#include "BitmapPlotWnd.h"
#include "BitmapTypeSettings.h"

using namespace std;
using namespace SoraDbgPlot::Task;

PlotWindowOpenedSpectrum::PlotWindowOpenedSpectrum(std::shared_ptr<ProcessOpened> process) :
	PlotWindowOpenedLineType(process)
{
	_range = 1;
}

PlotWindowOpenedSpectrum::~PlotWindowOpenedSpectrum()
{
	;
}

shared_ptr<BaseProperty> PlotWindowOpenedSpectrum::CreatePropertyPage()
{
	return PlotWindowOpenedLineType::CreatePropertyPage();
}

bool PlotWindowOpenedSpectrum::Accept(shared_ptr<ChannelOpened> channel, CPoint pointIn, CPoint & pointOut)
{
	bool bAccepted = false;
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedSpectrum, AsyncObject>(shared_from_this());

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

		if (dynamic_pointer_cast<ChannelOpenedSpectrum, ChannelOpened>(channel) == 0 &&
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


void PlotWindowOpenedSpectrum::ModifyDataRange(size_t & range)
{
	range = 1;
}

HRESULT PlotWindowOpenedSpectrum::AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent)
{
	PlotWindowOpenedLineType::AppendXmlProperty(pDom, pParent);

	return S_OK;
}

HRESULT PlotWindowOpenedSpectrum::LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement)
{
	PlotWindowOpenedLineType::LoadXmlElement(pElement);

	return S_OK;
}

const wchar_t * PlotWindowOpenedSpectrum::GetTypeName()
{
	return L"Spectrum Window";
}

void PlotWindowOpenedSpectrum::OnMouseWheel(bool bIsUp)
{
	this->_bAutoScale = false;

	double dist = this->_maxValue - this->_minValue;
	double center = (this->_maxValue + this->_minValue) / 2.0;

	double minDist = 3.0;
	double maxValue = ::SettingGetBitmapRangeMax(this->_isLog);
	double minValue = ::SettingGetBitmapRangeMin(this->_isLog);

	if (bIsUp)
	{
		dist = max(minDist, this->_isLog ? dist - 0.5 : dist / 2.0);
	}
	else
	{
		dist = this->_isLog ? dist + 0.5 : dist * 2.0;
	}

	double halfDist = dist / 2.0;
	halfDist = min(halfDist, maxValue - center);
	halfDist = min(halfDist, center - minValue);

	this->_maxValue = center + halfDist;
	this->_minValue = center - halfDist;

	this->UpdatePropertyAutoScale(false);
	this->UpdatePropertyMaxMin(this->_maxValue, this->_minValue);
}

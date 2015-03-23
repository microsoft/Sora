#include "stdafx.h"
#include <memory>
#include "PlotWindowOpenedLog.h"
#include "ChannelOpenedLog.h"

using namespace std;
using namespace SoraDbgPlot::Task;

PlotWindowOpenedLog::PlotWindowOpenedLog(shared_ptr<ProcessOpened> process) :
	PlotWindowOpenedTextType(process)
{
}

PlotWindowOpenedLog::~PlotWindowOpenedLog()
{

}

bool PlotWindowOpenedLog::Accept(shared_ptr<ChannelOpened> channel, CPoint pointIn, CPoint & pointOut)
{
	bool bAccepted = false;
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedLog, AsyncObject>(shared_from_this());

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

		if (SThis->_chMap.size() == 1)
		{
			bAccepted = false;
			return;
		}
		
		if (dynamic_pointer_cast<PlotWindowOpenedLog, ChannelOpened>(channel) == 0)
		{
			bAccepted = false;
			return;
		}

		bAccepted = true;
	});

	return bAccepted;	
}

HRESULT PlotWindowOpenedLog::AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent)
{
	PlotWindowOpenedTextType::AppendXmlProperty(pDom, pParent);

	return S_OK;
}

HRESULT PlotWindowOpenedLog::LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement)
{
	PlotWindowOpenedTextType::LoadXmlElement(pElement);

	return S_OK;
}

const wchar_t * PlotWindowOpenedLog::GetTypeName()
{
	return L"Log Window";
}

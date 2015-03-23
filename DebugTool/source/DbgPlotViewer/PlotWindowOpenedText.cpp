#include "stdafx.h"
#include "PlotWindowOpenedText.h"
#include "ChannelOpenedText.h"

using namespace std;
using namespace SoraDbgPlot::Task;

PlotWindowOpenedText::PlotWindowOpenedText(std::shared_ptr<ProcessOpened> process) :
	PlotWindowOpenedTextType(process)
{
}

PlotWindowOpenedText::~PlotWindowOpenedText()
{

}

bool PlotWindowOpenedText::Accept(std::shared_ptr<ChannelOpened> channel, CPoint pointIn, CPoint & pointOut)
{
	bool bAccepted = false;
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedText, AsyncObject>(shared_from_this());

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
		
		if (dynamic_pointer_cast<ChannelOpenedText, ChannelOpened>(channel) == 0)
		{
			bAccepted = false;
			return;
		}

		bAccepted = true;
	});

	return bAccepted;
}


HRESULT PlotWindowOpenedText::AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent)
{
	PlotWindowOpenedTextType::AppendXmlProperty(pDom, pParent);

	return S_OK;
}

HRESULT PlotWindowOpenedText::LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement)
{
	PlotWindowOpenedTextType::LoadXmlElement(pElement);

	return S_OK;
}

const wchar_t * PlotWindowOpenedText::GetTypeName()
{
	return L"Text Window";
}

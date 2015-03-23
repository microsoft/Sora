#pragma once

#include "PlotWindowOpened.h"
#include "ChannelOpenedTextType.h"

class PlotWindowOpenedTextType : public PlotWindowOpened
{
public:
	PlotWindowOpenedTextType(std::shared_ptr<ProcessOpened>);
	~PlotWindowOpenedTextType();
	
protected:
	virtual void PlayPauseToLatest(bool);
	virtual CPlotWnd * CreatePlotWnd();
	virtual std::shared_ptr<SoraDbgPlot::Task::TaskSimple> TaskUpdate();
	virtual HRESULT AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent);
	virtual HRESULT LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement);
	virtual std::shared_ptr<BaseProperty> CreatePropertyPage();
	virtual void OnChannelAdded(std::shared_ptr<ChannelOpened>, const CRect &);
	
private:
	void SetColor(COLORREF);

private:

	struct UpdateOperation
	{
		std::shared_ptr<ChannelOpenedTextType> _channel;
		std::shared_ptr<bool> _updated;
	};

	std::shared_ptr<UpdateOperation> _operationUpdate;

};

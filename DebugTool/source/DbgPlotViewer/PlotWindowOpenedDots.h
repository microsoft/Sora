#pragma once
#include "Event.h"
#include "TaskQueue.h"
#include "PlotWindowOpened.h"
#include "ChannelOpenedDots.h"
#include "ChannelOpenedText.h"
#include "PlotWindowOpenedBitmapType.h"

class PlotWindowOpenedDots : public PlotWindowOpenedBitmapType
{
public:
	PlotWindowOpenedDots(std::shared_ptr<ProcessOpened>);
	~PlotWindowOpenedDots();

	virtual bool Accept(std::shared_ptr<ChannelOpened>, CPoint pointIn, CPoint & pointOut);

	void SetMaxValue(double maxValue);
	void SetLuminescence(bool bLuminescence);

	void UpdatePropertyMax(double max);
	void UpdatePropertyDataRange(size_t dataCount);

	virtual std::shared_ptr<SoraDbgPlot::Task::TaskSimple> TaskUpdate();

	virtual std::shared_ptr<BaseProperty> CreatePropertyPage();

protected:
	virtual void ModifyDataRange(size_t & range);
	virtual HRESULT AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent);
	virtual HRESULT LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement);
	virtual void OnMouseWheel(bool bIsUp);

protected:
	void DrawGrid(Graphics * g, const CRect & clientRect, double max, size_t range);

	double _maxValue;
	bool _luminescence;

	struct UpdateOperation
	{
		UpdateOperation()
		{
			param = std::make_shared<DrawDotsParam>();
			dataSize = std::make_shared<size_t>(0);
			bmp = 0;
		}
		~UpdateOperation()
		{
			if (bmp)
				delete bmp;
		}

		Bitmap * bmp;
		std::vector<std::shared_ptr<ChannelOpened> > _vecChUpdate;
		std::vector<std::shared_ptr<ChannelOpenedDots> > _vecChDots;
		std::vector<std::shared_ptr<ChannelOpenedText> > _vecChText;
		std::shared_ptr<DrawDotsParam> param;
		std::shared_ptr<size_t> dataSize;
		bool _applyAutoScale;
	};

	std::shared_ptr<UpdateOperation> _operationUpdate;
};

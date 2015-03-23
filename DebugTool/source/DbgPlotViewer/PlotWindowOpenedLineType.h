#pragma once

#include "Event.h"
#include "TaskQueue.h"
#include "PlotWindowOpenedBitmapType.h"
#include "ChannelOpenedLineType.h"
#include "ChannelOpenedText.h"
#include "GridAlg.h"

class PlotWindowOpenedLineType : public PlotWindowOpenedBitmapType
{
public:
	PlotWindowOpenedLineType(std::shared_ptr<ProcessOpened>);
	virtual std::shared_ptr<BaseProperty> CreatePropertyPage();

public:
	struct MaxValueEvent
	{
		double Max;
		double Min;
	};

	SoraDbgPlot::Event::Event<MaxValueEvent> EventMaxValueChanged;

	void SetMaxValue(double maxValue);
	void SetMinValue(double maxValue);

	void UpdatePropertyMaxMin(double max, double min);
	void UpdatePropertyAutoScale(bool bAutoScale);
	void UpdatePropertyDataRange(size_t dataCount);

	virtual std::shared_ptr<SoraDbgPlot::Task::TaskSimple> TaskUpdate();

private:
	struct UpdateOperation
	{
		UpdateOperation()
		{
			param = std::make_shared<DrawLineParam>();
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
		std::vector<std::shared_ptr<ChannelOpenedLineType> > _vecChLineType;
		std::vector<std::shared_ptr<ChannelOpenedText> > _vecChText;
		std::shared_ptr<DrawLineParam> param;
		std::shared_ptr<size_t> dataSize;
		bool _applyAutoScale;
	};
	
	std::shared_ptr<UpdateOperation> _operationUpdate;

protected:
	double _maxValue;
	double _minValue;
	double _maxValueAfterAutoScale;
	double _minValueAfterAutoScale;

protected:
	void DrawGrid(Graphics * g, const CRect &clientRect, double dispMax, double dispMin);
	virtual HRESULT AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent);
	virtual HRESULT LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement);
	virtual void OnCoordinateChanged();
	virtual bool IsRangeSettingEnabled();

private:

	GridSolutionSolver gridSolutionSolver;

	void DrawGridLine(Graphics * g, double resolution, double yData, const CRect & clientRect, double dispMax, double dispMin);
	Gdiplus::REAL GetClientY(double y, const CRect & clientRect, double dispMax, double dispMin);
};

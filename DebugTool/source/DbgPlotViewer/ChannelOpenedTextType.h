#pragma once
#include "ChannelOpened.h"

class ChannelOpenedTextType : public ChannelOpened
{
public:
	ChannelOpenedTextType();
	~ChannelOpenedTextType();
	
	SoraDbgPlot::Event::Event<COLORREF> EventColor;

	void ForColor(const std::function<void(COLORREF)> & f);

	virtual void Clear();
	virtual void OnColorUpdated();

	virtual std::shared_ptr<SoraDbgPlot::Task::TaskSimple> TaskGetSize(std::shared_ptr<size_t>) = 0;
	virtual size_t DataSize() = 0;
	virtual char * GetData(size_t index, bool bFromOldest) = 0;
};


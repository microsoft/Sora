#pragma once

#include <memory>
#include "ProcessOpened.h"
#include "ChannelOpened.h"

class ChannelAddable : public PropObject
{
public:
	virtual void Highlight(bool bHighlight) = 0;
	virtual bool Accept(std::shared_ptr<ChannelOpened>, CPoint pointIn, CPoint & pointOut) = 0;
	virtual void RequestAddChannel(std::shared_ptr<ProcessOpened>, std::shared_ptr<ChannelOpened>, CPoint pointIn) = 0;
	void Clear();
};

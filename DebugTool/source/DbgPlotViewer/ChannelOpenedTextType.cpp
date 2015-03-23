#include "stdafx.h"
#include "ChannelOpenedTextType.h"

using namespace std;

ChannelOpenedTextType::ChannelOpenedTextType()
{

}

ChannelOpenedTextType::~ChannelOpenedTextType()
{

}

void ChannelOpenedTextType::OnColorUpdated()
{
	EventColor.Raise(this, _color);
}

void ChannelOpenedTextType::Clear()
{
	EventColor.Reset();
	ChannelOpened::Clear();
}

void ChannelOpenedTextType::ForColor(const std::function<void(COLORREF)> & f)
{
	auto SThis = dynamic_pointer_cast<ChannelOpenedTextType, AsyncObject>(shared_from_this());
	this->DoLater([SThis, f](){
		f(SThis->_color);
	});
}

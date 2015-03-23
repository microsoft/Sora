#include <algorithm>
#include <memory>
#include "TaskQueue.h"
#include "SharedChannel.h"
#include "AppSettings.h"

using namespace std;
using namespace SoraDbgPlot::SharedObj;
using namespace SoraDbgPlot::Task;

int SharedChannel::Id() const
{
	return _smChannel->Id();	
}

int SharedChannel::Pid() const
{
	return _smChannel->Pid();
}

int SharedChannel::SpectrumDataSize() const
{
	return _smChannel->SpectrumSize();
}

const wchar_t * SharedChannel::Name() const
{
	return _smChannel->Name();
}

ChannelType SharedChannel::Type() const
{
	return _smChannel->Type();
}

SharedChannel::~SharedChannel()
{
	_taskQueue->DoTask([this](){
		for_each(this->_dataList->begin(), this->_dataList->end(), [](NewDataParam p){
			delete [] p.ptr;
		});
	});
}

bool SharedChannel::Write(const char * ptr, size_t length)
{
	char * buf = new char[length];
	memcpy(buf, ptr, length);

	NewDataParam p;
	p.ptr = buf;
	p.length = length;

	auto SThis = shared_from_this();
	this->_taskQueue->QueueTask([SThis, p](){
		SThis->_dataList->push_back(p);
		SThis->_dataSize += p.length;
		do
		{
			if (SThis->_dataList->size() ==0)
				break;
			
			int frontLen = SThis->_dataList->front().length;
			if (SThis->_dataSize - frontLen > SettingGetReplayBufferSize())
			{
				auto param = SThis->_dataList->front();
				delete [] param.ptr;
				SThis->_dataList->pop_front();
				SThis->_dataSize -= frontLen;
			}
			else
				break;
		} while(1);
	}, [p](){
		if (p.ptr)
			delete [] p.ptr;
	});

	return true;
}

shared_ptr<list<NewDataParam> > SharedChannel::CheckOut()
{
	shared_ptr<list<NewDataParam> > dataList;
	_taskQueue->DoTask([this, &dataList](){
		dataList = _dataList;
		_dataList = make_shared<list<NewDataParam> >();
		_dataSize = 0;		
	});
	return dataList;
}

SharedChannel::SharedChannel(std::shared_ptr<SmChannel> sm)
{
	_smChannel = sm;
	_dataSize = 0;
	_dataList = make_shared<list<NewDataParam> >();
	_taskQueue = make_shared<TaskQueue>();
}


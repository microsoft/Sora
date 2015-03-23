#pragma once

#include "ChannelManager.h"
#include "SmChannel.h"
#include "SmProcess.h"
#include "CSLock.h"

class SharedChannelW
{
public:
	SharedChannelW(const char * channelName, ChannelType type, unsigned int spectrumSize);
	~SharedChannelW();
	int Write(const char * ptr, size_t length);
	unsigned int SpectrumSize();
	bool IsType(ChannelType type);
	char * GetTextBuffer(size_t size);
	int BufferSizeAvailable();
	void Lock();
	void Unlock();

private:
	SmChannel * _smChannel;
	BaseChannel * _writable;
	unsigned int _spectrumSize;
	ChannelType _type;
	char * _textBuffer;
	size_t _textBufferSize;
	SoraDbgPlot::Lock::CSLock _lock;
};

class SharedSourceInfo
{
public:
	static BOOL Init(ShareMem * sm, void* context);
	int bufLen;
	volatile int rIdx;
	volatile int wIdx;
};

#include "SharedChannelW.h"

/*

	SharedChannelW

*/


SharedChannelW::SharedChannelW(const char * channelName, ChannelType type, unsigned int spectrumSize)
{
	wchar_t * wname = new wchar_t[MAX_PATH];
	::mbstowcs(wname, channelName, MAX_PATH);
	_smChannel = new SmChannel(wname, type, spectrumSize);
	delete [] wname;
	_writable = new BaseChannel(_smChannel->Id());
	_spectrumSize = spectrumSize;
	_type = type;
	_textBuffer = 0;
	_textBufferSize = 0;
}

SharedChannelW::~SharedChannelW()
{
	if (_writable)
		delete _writable;
	if (_smChannel)
		delete _smChannel;
	if (_textBuffer)
		delete _textBuffer;
}

int SharedChannelW::Write(const char * ptr, size_t length)
{
	int ret = _writable->Write(ptr, length);
		
	return ret;
}

unsigned int SharedChannelW::SpectrumSize()
{
	return _spectrumSize;
}

bool SharedChannelW::IsType(ChannelType type)
{
	return _type == type;
}

char * SharedChannelW::GetTextBuffer(size_t size)
{
	if (_textBufferSize < size)
	{
		_textBufferSize = size;
		if (_textBuffer)
			delete [] _textBuffer;

		_textBuffer = new char[size];
	}

	return _textBuffer;
}

int SharedChannelW::BufferSizeAvailable()
{
	int bufferSizeAvailable = _writable->BufferSizeAvailable();
	return bufferSizeAvailable;
}

void SharedChannelW::Lock()
{
	_lock.Lock();
}

void SharedChannelW::Unlock()
{
	_lock.Unlock();
}

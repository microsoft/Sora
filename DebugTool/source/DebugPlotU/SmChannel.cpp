#include <assert.h>
#include "SmChannel.h"
#include "SharedNameManagement.h"
#include "SharedSerialNumGenerator.h"


static wchar_t * myWcsDup(const wchar_t * src)
{
	size_t bufSize = wcslen(src);
	wchar_t * newBuf = new wchar_t[bufSize+1];
	memcpy(newBuf, src, bufSize*sizeof(wchar_t));
	newBuf[bufSize] = 0;
	return newBuf;
}

SmChannel::SmChannel(const wchar_t * channelName, ChannelType type, unsigned int spectrumSize)
{
	_channelName = myWcsDup(channelName);
	_pid = ::GetCurrentProcessId();
	_spectrumSize = spectrumSize;
	_type = type;

	SoraDbgPlot::Common::SharedSerialNumGenerator generator(
		SharedNameManager::GetSerialNumGeneratorName()
		);

	_id = generator.GetSerialNum();

	wchar_t * nameBuf;
	size_t sizeNumBuf = 0;
	SharedNameManager::GenChannelName(_id, 0, sizeNumBuf);

	nameBuf = new wchar_t[sizeNumBuf];
	SharedNameManager::GenChannelName(_id, nameBuf, sizeNumBuf);

	_sm = AllocateSharedMem(
		nameBuf, 
		sizeof(SmChannelStruct), 
		SmInit, this);

	delete [] nameBuf;

	_smStruct = (SmChannelStruct *)_sm->GetAddress();
}

SmChannel::SmChannel(int id)
{
	wchar_t * nameBuf;
	size_t sizeNumBuf = 0;
	SharedNameManager::GenChannelName(id, 0, sizeNumBuf);

	nameBuf = new wchar_t[sizeNumBuf];
	SharedNameManager::GenChannelName(id, nameBuf, sizeNumBuf);

	_sm = AllocateSharedMem(
		nameBuf, 
		sizeof(SmChannelStruct), 
		SmInitR, this);

	delete [] nameBuf;

	SmChannelStruct * smChannelStruct = (SmChannelStruct *)_sm->GetAddress();

	_channelName = myWcsDup(smChannelStruct->name);

	_type = smChannelStruct->type;
	_id = smChannelStruct->id;
	_pid = smChannelStruct->pid;
	_spectrumSize = smChannelStruct->spectrumSize;
}

SmChannel::~SmChannel()
{
	delete [] _channelName;
	ShareMem::FreeShareMem(_sm);
}

int SmChannel::Pid()
{
	return _pid;
}

int SmChannel::Id()
{
	return _id;
}

unsigned int SmChannel::SpectrumSize()
{
	return _spectrumSize;
}

const wchar_t * SmChannel::Name()
{
	return _channelName;
}

bool SmChannel::IsValid()
{
	return _id != -1;
}

BOOL SmChannel::SmInit(ShareMem* sm, void* context)
{
	SmChannel * smChannel = (SmChannel *)context;
	SmChannelStruct * smChannelStruct = (SmChannelStruct *)sm->GetAddress();

	wcscpy(smChannelStruct->name, smChannel->_channelName);
	smChannelStruct->type = smChannel->_type;
	smChannelStruct->id = smChannel->_id;
	smChannelStruct->pid = smChannel->_pid;
	smChannelStruct->spectrumSize = smChannel->_spectrumSize;

	return true;
}

BOOL SmChannel::SmInitR(ShareMem* sm, void* context)
{
	SmChannel * smChannel = (SmChannel *)context;
	SmChannelStruct * smChannelStruct = (SmChannelStruct *)sm->GetAddress();

	smChannelStruct->type = CH_TYPE_ILLEGAL;
	smChannelStruct->name[0] = 0;
	smChannelStruct->pid = 0;
	smChannelStruct->id = -1;
	smChannelStruct->spectrumSize = 0;

	return true;
}

ChannelType SmChannel::Type()
{
	return _type;
}

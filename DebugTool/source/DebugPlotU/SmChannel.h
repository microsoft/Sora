#pragma once

#include "ShareMemHelper.h"

enum ChannelType
{
	CH_TYPE_LINE,
	CH_TYPE_SPECTRUM,
	CH_TYPE_DOTS,
	CH_TYPE_TEXT,
	CH_TYPE_LOG,
	CH_TYPE_ILLEGAL,
};

struct SmChannelStruct
{
	wchar_t name[MAX_PATH];
	int pid;
	ChannelType type;
	int id;
	unsigned int spectrumSize;
};

class SmChannel
{
public:
	SmChannel(const wchar_t * name, ChannelType type, unsigned int spectrumSize);
	SmChannel(int id);
	~SmChannel();

	ChannelType Type();
	int Pid();
	int Id();
	unsigned int SpectrumSize();
	const wchar_t * Name();
	bool IsValid();

private:
	ShareMem * _sm;
	SmChannelStruct * _smStruct;

	static BOOL SmInit(ShareMem* sm, void* context);
	static BOOL SmInitR(ShareMem* sm, void* context);

private:
	wchar_t * _channelName;
	ChannelType _type;
	int _pid;
	int _id;
	unsigned int _spectrumSize;
};

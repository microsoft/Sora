#pragma once


int __stdcall SettingGetSourceBufferSize();
int __stdcall SettingGetSourceBufferComplex16Count();
const wchar_t * __stdcall SettingGetGlobalBufferName();
int __stdcall SettingGetGlobalBufferBlockSize();
int __stdcall SettingGetGlobalBufferBlockNum();
int __stdcall SettingGetGlobalBufferThreadHoldLow();
int __stdcall SettingGetGlobalBufferThreadHoldHigh();
int __stdcall SettingGetReplayBufferSize();
int __stdcall SettingGetMaxSpeed();

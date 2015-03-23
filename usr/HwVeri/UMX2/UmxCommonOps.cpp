#include "sora.h"
#include "Radio.h"
#include "UmxCommonOps.h"

HRESULT SoraUmxInit(Radio * radio)
{
	radio->SetInitialized(true);
	return S_OK;

	Logger * logger = Logger::GetLogger(L"umx");

	BOOLEAN succ = SoraUInitUserExtension("\\\\.\\HWTest");

	logger->Log(LOG_FUNC_CALL, L"SoraUInitUserExtension called\r\n");

	if (!succ)
	{
		logger->Log(LOG_ERROR, L"failed\n");
		logger->Log(LOG_ERROR, 
			L"Make sure that RCB driver and HwTest driver are both installed "
			L"and enabled.\n"
			L"You may try disabling and enabling HwTest driver to solve the problem.\n"
			);

		radio->SetInitialized(false);
		return -1;
	}

	radio->SetInitialized(true);

	logger->Log(LOG_SUCCESS, L"success\n");

	return S_OK;		
}

HRESULT SoraUmxDeinit(Radio * radio)
{
	radio->SetInitialized(false);
	return S_OK;

	Logger * logger = Logger::GetLogger(L"umx");
	if (radio->IsInitialized())
	{
		//StopSend(true);
		//StopReceive(true);

		SoraUCleanUserExtension();
		logger->Log(LOG_FUNC_CALL, L"SoraUCleanUserExtension called\r\n");
		radio->SetInitialized(false);
	}
	return S_OK;
}

int SetPriority()
{
	Logger * logger = Logger::GetLogger(L"umx");

	// Set thread affinity and priority
    DWORD_PTR dwProcessAffinityMask, dwSystemAffinityMask;

    if (!GetProcessAffinityMask(GetCurrentProcess(), &dwProcessAffinityMask, &dwSystemAffinityMask))
    {
		logger->Log(LOG_FUNC_CALL, L"GetProcessAffinityMask() failed.\r\n");
        return -1;
    }

    if (dwProcessAffinityMask & 0x03)
    {
        if (!SetThreadAffinityMask(GetCurrentThread(), 0x03))
        {
			logger->Log(LOG_FUNC_CALL, L"SetThreadAffinityMask() failed.\r\n");
            return -1;
        }
    }

    if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
    {
		logger->Log(LOG_ERROR, L"SetPriorityClass() failed.\r\n");
        return -1;
    }

	return 0;
}

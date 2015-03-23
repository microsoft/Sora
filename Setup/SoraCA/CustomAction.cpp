
#include "stdafx.h"
#include <shlwapi.h>

UINT __stdcall CheckDepends(MSIHANDLE hInstall);
UINT __stdcall VerifyInstallDir(MSIHANDLE hInstall);
BOOL __stdcall FindSoraRoot(MSIHANDLE hInstall);
BOOL __stdcall CheckWinDDK(LPCTSTR szPath, MSIHANDLE hInstall);
BOOL __stdcall FindWinDDK(MSIHANDLE hInstall);

UINT __stdcall CheckDepends(MSIHANDLE hInstall)
{
	HRESULT hr = S_OK;
	UINT er = ERROR_SUCCESS;

	hr = WcaInitialize(hInstall, "CA_CheckDepends");
	ExitOnFailure(hr, "Failed to initialize");

	WcaLog(LOGMSG_STANDARD, "Initialized.");

    FindSoraRoot(hInstall);
    FindWinDDK(hInstall);

LExit:
	er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
	return WcaFinalize(er);
}

BOOL __stdcall FindSoraRoot(MSIHANDLE hInstall)
{
    TCHAR szBuf[1000];
    DWORD dwRes = GetEnvironmentVariable(TEXT("SORA_ROOT"), szBuf, 1000);
    if (dwRes == 0)
    {
        DWORD dwErr = GetLastError();
        if (dwErr == ERROR_ENVVAR_NOT_FOUND)
        {
            WcaLog(LOGMSG_STANDARD, "Environment variable SORA_ROOT not found\n");
        }
        else
        {
            WcaLog(LOGMSG_STANDARD, "Error when looking for SORA_ROOT\n");
        }
        return FALSE;
    }
    else if (dwRes > 1000)
    {
        WcaLog(LOGMSG_STANDARD, "Buffer is too small\n");
        return FALSE;
    }

    WcaLog(LOGMSG_STANDARD, "SoraRoot=[%s]\n", szBuf);
    MsiSetProperty(hInstall, TEXT("SORA_ROOT"), szBuf);
    return TRUE;
}

BOOL __stdcall CheckWinDDK(LPCTSTR szPath, MSIHANDLE hInstall)
{
    if (!PathFileExists(szPath)) return FALSE;
    WIN32_FIND_DATA findData = {0};
    TCHAR szPattern[MAX_PATH];
    StringCchPrintf(szPattern, MAX_PATH, TEXT("%s\\*.*"), szPath);
    HANDLE hFind = FindFirstFile(szPattern, &findData);
    if (hFind == INVALID_HANDLE_VALUE) return FALSE;
    BOOL fContinue = TRUE;
    while (fContinue)
    {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
            && (_tcscmp(findData.cFileName, TEXT(".")) != 0)
            && (_tcscmp(findData.cFileName, TEXT("..")) != 0))
        {
            StringCchPrintf(szPattern, MAX_PATH, TEXT("%s\\%s\\bin\\setenv.bat"), szPath, findData.cFileName);
            if (PathFileExists(szPattern))
            {
                FindClose(hFind);
                StringCchPrintf(szPattern, MAX_PATH, TEXT("%s\\%s"), szPath, findData.cFileName);
                WcaLog(LOGMSG_STANDARD, "Found DDK in [%s]\n", szPattern);
                MsiSetProperty(hInstall, TEXT("WINDDK_ROOT"), szPattern);
                return TRUE;
            }
        }
        fContinue = FindNextFile(hFind, &findData);
    }
    FindClose(hFind);
    WcaLog(LOGMSG_STANDARD, "Not found in %s\n", szPath);
    return FALSE;
}

BOOL __stdcall FindWinDDK(MSIHANDLE hInstall)
{
    LPCTSTR s_DriveRoot[] =
    {
        TEXT("A:\\"), TEXT("B:\\"), TEXT("C:\\"), TEXT("D:\\"), TEXT("E:\\"),
        TEXT("F:\\"), TEXT("G:\\"), TEXT("H:\\"), TEXT("I:\\"), TEXT("J:\\"),
        TEXT("K:\\"), TEXT("L:\\"), TEXT("M:\\"), TEXT("N:\\"), TEXT("O:\\"),
        TEXT("P:\\"), TEXT("Q:\\"), TEXT("R:\\"), TEXT("S:\\"), TEXT("T:\\"),
        TEXT("U:\\"), TEXT("V:\\"), TEXT("W:\\"), TEXT("X:\\"), TEXT("Y:\\"),
        TEXT("Z:\\")
    };

    DWORD dwDriveMask = GetLogicalDrives();
    unsigned int bitmask = 1;
    for (int i = 0; i < 26; i++)
    {
        if (dwDriveMask & bitmask)
        {
            WcaLog(LOGMSG_STANDARD, "[%c:\\]\t", 'A' + i);
            UINT uiDriveType = GetDriveType(s_DriveRoot[i]);
            if (uiDriveType == DRIVE_FIXED)
            {
                WcaLog(LOGMSG_STANDARD, "Fixed disk\n");

                TCHAR szPath[MAX_PATH];
                StringCchPrintf(szPath, MAX_PATH, TEXT("%sWinDDK"), s_DriveRoot[i]);
                if (CheckWinDDK(szPath, hInstall)) return TRUE;

                StringCchPrintf(szPath, MAX_PATH, TEXT("%sProgram Files\\WinDDK"), s_DriveRoot[i]);
                if (CheckWinDDK(szPath, hInstall)) return TRUE;
            }
        }
        bitmask <<= 1;
    }

    return FALSE;
}


UINT __stdcall VerifyInstallDir(MSIHANDLE hInstall)
{
	HRESULT hr = S_OK;
	UINT er = ERROR_SUCCESS;

	hr = WcaInitialize(hInstall, "CA_VerifyInstallDir");
	ExitOnFailure(hr, "Failed to initialize");

    TCHAR szInstallDir[MAX_PATH];
    DWORD dwLen = MAX_PATH;
    MsiGetProperty (hInstall, TEXT("APPLICATIONFOLDER"), szInstallDir, &dwLen);
    if (_tcschr(szInstallDir, TEXT(' ')) == NULL)
    {
        MsiSetProperty (hInstall, TEXT("SoraRootOK"), TEXT("1"));
    }
    else
    {
        MsiSetProperty (hInstall, TEXT("SoraRootOK"), TEXT("0"));
    }

LExit:
	er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
	return WcaFinalize(er);
}

// DllMain - Initialize and cleanup WiX custom action utils.
extern "C" BOOL WINAPI DllMain(
	__in HINSTANCE hInst,
	__in ULONG ulReason,
	__in LPVOID
	)
{
	switch(ulReason)
	{
	case DLL_PROCESS_ATTACH:
		WcaGlobalInitialize(hInst);
		break;

	case DLL_PROCESS_DETACH:
		WcaGlobalFinalize();
		break;
	}

	return TRUE;
}

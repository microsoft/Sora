// CheckDepends.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <shlwapi.h>
#include <conio.h>
#include <strsafe.h>

typedef DWORD MSIHANDLE;

BOOL __stdcall FindSoraRoot(MSIHANDLE hInstall)
{
    TCHAR szBuf[1000];
    DWORD dwRes = GetEnvironmentVariable(TEXT("SORA_ROOT"), szBuf, 1000);
    if (dwRes == 0)
    {
        DWORD dwErr = GetLastError();
        if (dwErr == ERROR_ENVVAR_NOT_FOUND)
        {
            printf("Environment variable SORA_ROOT not found\n");
        }
        else
        {
            printf("Error when looking for SORA_ROOT\n");
        }
        return FALSE;
    }
    else if (dwRes > 1000)
    {
        printf("Buffer is too small\n");
        return FALSE;
    }

    _tprintf(TEXT("SoraRoot=[%s]\n"), szBuf);
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
                _tprintf(TEXT("Found %s\n"), szPattern);
                return TRUE;
            }
        }
        fContinue = FindNextFile(hFind, &findData);
    }
    FindClose(hFind);
    _tprintf(TEXT("Not found in %s\n"), szPath);
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
    for (int i = 0; i < 32; i++)
    {
        if (dwDriveMask & bitmask)
        {
            printf("[%c:\\]\t", 'A' + i);
            UINT uiDriveType = GetDriveType(s_DriveRoot[i]);
            if (uiDriveType == DRIVE_FIXED)
            {
                printf("Fixed disk\n");

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


int _tmain(int argc, _TCHAR* argv[])
{
    FindSoraRoot(0);
    FindWinDDK(0);

    getch();
	return 0;
}


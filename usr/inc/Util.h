#ifndef _SORA_USR_UTIL_H
#define _SORA_USR_UTIL_H

#include <windows.h>
#include <stdarg.h>

inline void OutputDebugMessage(const char *format, ...)
{
    char szMsg[1024];
    va_list args;
    va_start(args, format);
    vsprintf_s(szMsg, 1024, format, args);
    OutputDebugStringA(szMsg);
}

inline DWORD RoundUpBase(DWORD dw, DWORD dwBase)
{
    if (dw < dwBase) return dwBase;
    int nRemain = dw % dwBase;
    if (nRemain == 0) return dw;
    return (dw - nRemain + dwBase);
}

static bool IsWhiteString(char *psz)
{
    while (*psz != 0)
    {
        if (!isspace((unsigned char)(psz[0]))) return false;
        psz++;
    }

    return true;
}

static void PreprocessDataForOldHardware(BYTE *head, int len)
{
    if (head != NULL)
    {
        short *p = (short *)head;
        for (int i = 0; i < len; i+= 128)
        {
            p += 8;

            for (int j = 0; j < 56; j++)
            {
                (*p) <<= 2;
                p++;
            }
        }
    }
}

static BOOL CenterWindow(HWND hWnd, HWND hWndCenter = NULL) throw()
{
	if (!::IsWindow(hWnd)) return FALSE;

	// determine owner window to center against
	DWORD dwStyle = (DWORD)::GetWindowLong(hWnd, GWL_STYLE);
	if(hWndCenter == NULL)
	{
		if(dwStyle & WS_CHILD)
			hWndCenter = ::GetParent(hWnd);
		else
			hWndCenter = ::GetWindow(hWnd, GW_OWNER);
	}

	// get coordinates of the window relative to its parent
	RECT rcDlg;
	::GetWindowRect(hWnd, &rcDlg);
	RECT rcArea;
	RECT rcCenter;
	HWND hWndParent;
	if(!(dwStyle & WS_CHILD))
	{
		// don't center against invisible or minimized windows
		if(hWndCenter != NULL)
		{
			DWORD dwStyleCenter = ::GetWindowLong(hWndCenter, GWL_STYLE);
			if(!(dwStyleCenter & WS_VISIBLE) || (dwStyleCenter & WS_MINIMIZE))
				hWndCenter = NULL;
		}

		// center within screen coordinates
#if WINVER < 0x0500
		::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);
#else
		HMONITOR hMonitor = NULL;
		if(hWndCenter != NULL)
		{
			hMonitor = ::MonitorFromWindow(hWndCenter, MONITOR_DEFAULTTONEAREST);
		}
		else
		{
			hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		}
		if (hMonitor == NULL) return FALSE;
		
		MONITORINFO minfo;
		minfo.cbSize = sizeof(MONITORINFO);
		BOOL bResult = ::GetMonitorInfo(hMonitor, &minfo);
		if (!bResult) return FALSE;
		
		rcArea = minfo.rcWork;
#endif
		if(hWndCenter == NULL)
			rcCenter = rcArea;
		else
			::GetWindowRect(hWndCenter, &rcCenter);
	}
	else
	{
		// center within parent client coordinates
		hWndParent = ::GetParent(hWnd);
		if (!::IsWindow(hWndParent)) return FALSE;

		::GetClientRect(hWndParent, &rcArea);
		::GetClientRect(hWndCenter, &rcCenter);
		::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
	}

	int DlgWidth = rcDlg.right - rcDlg.left;
	int DlgHeight = rcDlg.bottom - rcDlg.top;

	// find dialog's upper left based on rcCenter
	int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
	int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

	// if the dialog is outside the screen, move it inside
	if(xLeft + DlgWidth > rcArea.right)
		xLeft = rcArea.right - DlgWidth;
	if(xLeft < rcArea.left)
		xLeft = rcArea.left;

	if(yTop + DlgHeight > rcArea.bottom)
		yTop = rcArea.bottom - DlgHeight;
	if(yTop < rcArea.top)
		yTop = rcArea.top;

	// map screen coordinates to child coordinates
	return ::SetWindowPos(hWnd, NULL, xLeft, yTop, -1, -1,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

class CCompareGUID
{
public:
    bool operator()(REFGUID left, REFGUID right) const
    {
        return (left.Data1 < right.Data1);
    }
};

#define SAFE_RELEASE(p) do { if ((p) != NULL) { (p)->Release(); (p) = NULL; } } while (0)
#define SAFE_DELETE(p) do { if ((p) != NULL) { delete (p); (p) = NULL; } } while (0)
#define SAFE_DELETE_ARRAY(p) do { if ((p) != NULL) { delete[] (p); (p) = NULL; } } while (0)

#define ERROR_EXIT(condition) do { if (condition) goto ErrExit; } while (0)
#define ERROR_EXIT_SETHR(condition) do { if (condition) { hr = E_FAIL; goto ErrExit; } } while (0)
#define RETURN_IF(condition)  do { if (condition) return; } while (0)
#define RETURN_VALUE_IF(condition, val)  do { if (condition) return (val); } while (0)

#define GET_WIDTH(rc) ((rc).right - (rc).left)
#define GET_HEIGHT(rc) ((rc).bottom - (rc).top)

#endif

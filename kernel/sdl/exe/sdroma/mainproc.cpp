#include "common.h"

LRESULT CALLBACK 
mainProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
        case WM_CREATE:
            SetTimer(hwnd, 1, 15, NULL);
            paintInit(hwnd);
            break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
        case WM_TIMER:
            if (isPlaying())
                pushData(getSpeed());
            else
                break;
        case WM_PAINT:
            paintAll(hwnd);
            break;
        case WM_KEYDOWN:
            keyControl(wparam);
            paintAll(hwnd);
            break;
        case WM_LBUTTONUP:
        {
            POINTS pt = MAKEPOINTS(lparam);
            int t;
            if ((t = getPointerTime(pt.x + 1, pt.y)) >= 0)
                resetData(t);
            paintAll(hwnd);
            break;
        }
        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    return 0;
}


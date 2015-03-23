#include "common.h"
#include <stdio.h>

HWND gHwnd;

static void setWindowsClass(WNDCLASS & wc);
static void winListen();

void RegInfoWindows ();
    
void 
startDemoWindow()
{
    WNDCLASS wc;

    setWindowsClass(wc);
	if (!RegisterClass(&wc))
	{
		dprintf("ERROR: Register window class for core.\n");
		return;
	}

  //  RegInfoWindows ();
    
    winListen();
}

static void 
setWindowsClass(WNDCLASS & wc)
{
	wc.style = 0;
	wc.lpfnWndProc = mainProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = getMainInstance();	
    wc.hIcon = LoadIcon(getMainInstance(), MAKEINTRESOURCE(1));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = getClassName();
}


static void
winListen()
{
	HWND hwndMain = CreateWindow(
            getClassName(),
            getWinTitle(),
            WS_OVERLAPPEDWINDOW & (~(WS_MAXIMIZEBOX | WS_SIZEBOX)), 
            0, 0,
            getWinWidth(), 
            getWinHeight(),
            NULL,
            NULL,
            getMainInstance(),
            NULL);

	ShowWindow(hwndMain, SW_SHOWNORMAL);
	UpdateWindow(hwndMain);

    gHwnd = hwndMain;
    
    // paintInit(hwndMain);

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    paintClean(hwndMain);
}


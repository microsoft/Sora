#include "common.h"

static bool flagIsPlaying = false;

bool
isPlaying()
{
    return flagIsPlaying;
}

void
playOrPause()
{
    flagIsPlaying = !flagIsPlaying;
}

void
play()
{
    flagIsPlaying = true;
}

void
pause()
{
    flagIsPlaying = false;
}

static const int SPEEDS[]
    = { 1, 2, 5, 10, 20, 50, 100, 200, 300, 400, 600, 800, 1000, 1600, 3200 };
static const int SPEEDMAX = sizeof(SPEEDS) / sizeof(int); 
static int currentSpeed = 7;

void
speedUp()
{
    currentSpeed++;
    if (currentSpeed >= SPEEDMAX)
        currentSpeed = SPEEDMAX - 1;

	SetWindowText ( gHwnd, getWinTitle () );
}

void
speedDown()
{
    currentSpeed--;
    if (currentSpeed < 0)
        currentSpeed = 0;

	SetWindowText ( gHwnd, getWinTitle () );
}

int
getSpeed()
{
    return SPEEDS[currentSpeed];
}

int
getSpeedLevel ()
{
	return currentSpeed;
}
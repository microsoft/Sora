#pragma once

// Return value:
//        == 0 : no key
//         > 0 : a key is pressed
FINL
int SoraGetConsoleKey () {
	int input = 0;
	if ( _kbhit () ) {
		input = _getch();
	}
	return input;
}
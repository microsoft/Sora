#pragma once

#define diff_print(last_hr, hr, str) {	\
										\
	if (last_hr != hr) {				\
		split_printf("%s", str);				\
		last_hr  = hr;					\
	}									\
}

inline void split_printf ( char* format, ... )
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo ( GetStdHandle (STD_OUTPUT_HANDLE), &info );
	
	COORD cord;
	cord.X = info.srWindow.Left; cord.Y = info.srWindow.Bottom-1;
	SetConsoleCursorPosition ( GetStdHandle (STD_OUTPUT_HANDLE), cord );
	
	va_list vl;
	va_start ( vl, format );
	vprintf ( format, vl );
	va_end (vl);

//	print_status ();
}


#define debug_printf(format, ...) {		\
										\
	if (debug)							\
		split_printf(format, ## __VA_ARGS__);	\
}


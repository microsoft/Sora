#include "FileHelper.h"

bool SoraDbgPlot::File::CreateDirectoryRecursive(const wchar_t * __folder)
{
	bool bSucc;

	wchar_t * folderDup = _wcsdup(__folder);
	int strlength = wcslen(folderDup);
	
	// remove the last '\'
	if ( *(folderDup + strlength - 1) == '\\' )
		folderDup[strlength - 1] = 0;
	
	wchar_t * ptr = (wchar_t *)(folderDup + strlength - 1);

	while(ptr > folderDup && *ptr != '\\')
		--ptr;
	
	if (ptr == folderDup)	// root
		bSucc = true;
	else
	{
		*ptr = 0;
		bSucc = CreateDirectoryRecursive(folderDup);

		if (bSucc)
		{
			*ptr = '\\';
			BOOL bRes = CreateDirectoryW(folderDup, NULL);
			if (!bRes && GetLastError() != ERROR_ALREADY_EXISTS)
				bSucc = false;
			bSucc = true;
		}
		else
			bSucc = false;
	}

	delete [] folderDup;
	
	return bSucc;
}

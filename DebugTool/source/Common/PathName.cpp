#include "PathName.h"

const wchar_t * PathName::__reserved_char = L"\\/:*?\"<>| ";

PathName::PathName()
{
}

void PathName::AppendWithEscape(const wchar_t * name, wchar_t creplace)
{
	const wchar_t * ptr = name;
	wchar_t c;
	while( (c = *ptr) != 0 )
	{
		if (IsReserved(c))
			c = creplace;
		_path.push_back(c);
		++ptr;
	}		
}

void PathName::Append(const wchar_t * name)
{
	const wchar_t * ptr = name;
	wchar_t c;
	while( (c = *ptr) != 0 )
	{
		_path.push_back(c);
		++ptr;
	}
}

void PathName::Reset()
{
	_path.clear();
}

PathName::operator const wchar_t *()
{
	if ( (_path.size() == 0) ||
		_path[_path.size()-1] != 0
		)
	{
		_path.push_back(0);
	}

	return &_path[0];
}

bool PathName::IsReserved(wchar_t c)
{
	for (auto ptr = __reserved_char; *ptr != 0; ++ptr)
	{
		if (*ptr == c)
			return true;
	}
	return false;
}

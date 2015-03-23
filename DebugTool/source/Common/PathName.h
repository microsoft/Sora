#pragma once

#include <Windows.h>
#include <vector>

class PathName
{
public:
	PathName();

	void AppendWithEscape(const wchar_t * name, wchar_t creplace);

	void Append(const wchar_t * name);

	void Reset();

	operator const wchar_t *();

private:
	bool IsReserved(wchar_t c);

private:
	static const wchar_t * __reserved_char;
	std::vector<wchar_t> _path;
};

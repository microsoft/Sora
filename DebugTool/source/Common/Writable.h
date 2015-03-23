#pragma once

namespace SoraDbgPlot
{
	class Writable
	{
	public:
		virtual void Write(const void * ptr, size_t size, size_t & sizeWritten) = 0;
	};
}

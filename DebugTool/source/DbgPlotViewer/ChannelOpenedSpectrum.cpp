#include "stdafx.h"
#include <assert.h>
#include <memory>
#include "ChannelOpenedSpectrum.h"
#include "TaskQueue.h"

using namespace std;
using namespace SoraDbgPlot::Task;


ChannelOpenedSpectrum::ChannelOpenedSpectrum()
{
	DrawUsingSample(false);
}

shared_ptr<BaseProperty> ChannelOpenedSpectrum::CreatePropertyPage()
{
	return ChannelOpened::CreatePropertyPage();
}

size_t ChannelOpenedSpectrum::RangeToSize(size_t range)
{
	return this->_spectrumSize;
}


size_t ChannelOpenedSpectrum::IndexToSize(size_t index)
{
	return index * this->_spectrumSize;
}

shared_ptr<TaskSimple> ChannelOpenedSpectrum::TaskUpdateDataSize(std::shared_ptr<size_t> size)
{
	auto SThis = dynamic_pointer_cast<ChannelOpenedSpectrum, AsyncObject>(shared_from_this());
	return make_shared<TaskSimple>(TaskQueue(), [SThis, size](){
		size_t oldSize = *size;
		size_t mySize = SThis->_ringBuffer->Size() / SThis->_spectrumSize;
		size_t newSize = min(oldSize, mySize);
		*size = newSize;
	});
}

const wchar_t * ChannelOpenedSpectrum::GetTypeName()
{
	return L"Spectrum Channel";
}

bool ChannelOpenedSpectrum::Export(const CString & filename, bool bAll)
{
	auto SThis = dynamic_pointer_cast<ChannelOpenedSpectrum, AsyncObject>(shared_from_this());
	this->DoLater([SThis, filename, bAll](){
		FILE * fp;
		errno_t ret = _wfopen_s(&fp, filename, L"wb");

		if (ret == 0)
		{
			size_t spectrumSize = SThis->SpectrumDataSize();

			char * digitBuf = new char[128];
			//int * numBuf = new int[spectrumSize];
			size_t numOutput = 0;

			if (bAll)
			{
				SThis->_ringBuffer->Export([fp, digitBuf, &numOutput, spectrumSize](const int * ptr, size_t length){
					while(length > 0)
					{
						fprintf(fp, "%d ", *ptr);
						numOutput++;
						if (numOutput == spectrumSize)
						{
							fprintf(fp, "\r\n");
							numOutput = 0;
						}
						ptr++;
						length --;
					}
				});
			}
			else
			{
				size_t start = SThis->_latestIndex;
				size_t length = SThis->_range;
				length = min(length, SThis->_ringBuffer->Size());

				SThis->_ringBuffer->ExportRange(start, length, [fp, digitBuf, &numOutput, spectrumSize](const int * ptr, size_t length){
					while(length > 0)
					{
						fprintf(fp, "%d ", *ptr);
						numOutput++;
						if (numOutput == spectrumSize)
						{
							fprintf(fp, "\r\n");
							numOutput = 0;
						}
						ptr++;
						length --;
					}
				});
			}

			delete [] digitBuf;

			fclose(fp);
		}
	});

	return true;
}

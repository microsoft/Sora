#include "stdafx.h"
#include <assert.h>
#include <memory>
#include "ChannelOpenedLine.h"
#include "TaskQueue.h"
#include "ChannelProperty.h"

using namespace std;
using namespace SoraDbgPlot::Task;

shared_ptr<BaseProperty> ChannelOpenedLine::CreatePropertyPage()
{
	return ChannelOpened::CreatePropertyPage();
}

const wchar_t * ChannelOpenedLine::GetTypeName()
{
	return L"Line Channel";
}

bool ChannelOpenedLine::Export(const CString & filename, bool bAll)
{
	auto SThis = dynamic_pointer_cast<ChannelOpenedLine, AsyncObject>(shared_from_this());

	this->DoLater([SThis, filename, bAll](){
		FILE * fp;
		errno_t ret = _wfopen_s(&fp, filename, L"wb");

		if (ret == 0)
		{
			char * digitBuf = new char[128];

			if (bAll)
			{
				SThis->_ringBuffer->Export([fp, digitBuf](const int * ptr, size_t length){
					while(length > 0)
					{
						fprintf(fp, "%d\r\n", *ptr);
						ptr ++;
						length --;
					}
				});
			}
			else
			{
				size_t start = SThis->_latestIndex;
				size_t length = SThis->_range;
				length = min(length, SThis->_ringBuffer->Size());

				SThis->_ringBuffer->ExportRange(start, length, [fp, digitBuf](const int * ptr, size_t length){
					while(length > 0)
					{
						fprintf(fp, "%d\r\n", *ptr);
						ptr ++;
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

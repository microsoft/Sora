#include "stdafx.h"
#include "ChannelOpenedText.h"
#include "SubPlotWnd.h"
#include "HelperFunc.h"
#include "AppSettings.h"

using namespace std;
using namespace SoraDbgPlot::Task;

ChannelOpenedText::ChannelOpenedText()
{
	_plotWnd = 0;
	_latestTimeIdx = 0;
	_ringBuffer = new RingBufferWithTimeStamp<char>(::SettingGetReplayBufferSize() / sizeof(char));
	_filter = new SoraDbgPlot::FrameWithSizeInfoWriter<char>();
	_rect = CRect(0, 0, 10, 10);

	_filter->EventFlushData.Subscribe([this](const void * sender, const SoraDbgPlot::FrameWithSizeInfoWriter<char>::FlushDataEvent & e){

		size_t dataLen = e.length;
		if (dataLen >= 0)
		{
			int sizeNeeded = dataLen*2 + 1;

			char * dataBuf = _newLineFilterBuffer.UseBuf(sizeNeeded);

			char * ptrSrc = e.ptr;
			char * ptrDest = dataBuf;
			char preChar;

			while(ptrSrc < e.ptr + e.length)
			{
				char c = *ptrSrc;
				if (c != '\n')
					*ptrDest++ = c;
				else
				{
					if ( (ptrSrc == e.ptr) || preChar != '\r')
					{
						*ptrDest++ = '\r';
						*ptrDest++ = '\n';
					}
				}

				ptrSrc++;
				preChar = c;
			}

			*ptrDest = 0;
			int length = ptrDest - dataBuf;

			this->_ringBuffer->Write(dataBuf, length, e.timestamp);

			_newLineFilterBuffer.ReturnBuf();
		}
	});

	_newLineFilterBuffer.ConfigKeptSize(16*1024);
}

ChannelOpenedText::~ChannelOpenedText()
{
	delete _filter;
	delete _ringBuffer;
}

void ChannelOpenedText::WriteData(const char * data, size_t length)
{
	size_t dummy;
	this->_filter->Write(data, length, dummy);
	delete [] data;
}

SubPlotWnd * ChannelOpenedText::CreateSubPlotWnd()
{
	auto subWnd = new SubPlotWnd;

	auto SThis = dynamic_pointer_cast<ChannelOpenedText, AsyncObject>(shared_from_this());

	subWnd->EventMoveWindow.Subscribe([SThis](const void * sender, const CRect & rect){
		SThis->_rect = rect;
	});

	subWnd->EventClosed.Subscribe([SThis](const void * sender, const bool & e){

		auto SThis2 = SThis;
		SubPlotWnd * subPlotWnd = (SubPlotWnd *)sender;

		SThis2->DoLater([SThis2, subPlotWnd](){
			if (SThis2->_plotWnd == subPlotWnd)
				SThis2->_plotWnd = 0;

			auto subPlotWnd2 = subPlotWnd;
			SThis2->DoLater([subPlotWnd2](){
				delete subPlotWnd2;
			});
		});
	});
	
	this->DoNow([SThis, subWnd](){
		subWnd->SetColor(SThis->_color);
		SThis->_plotWnd = subWnd;
	});

	return subWnd;
}

void ChannelOpenedText::SetRect(const CRect & rect)
{
	auto shared_me = dynamic_pointer_cast<ChannelOpenedText, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([shared_me, rect](){
		shared_me->_rect = rect;
	});
}

std::shared_ptr<SoraDbgPlot::Task::TaskSimple> ChannelOpenedText::TaskGetSize(std::shared_ptr<size_t> size)
{
	auto SThis = dynamic_pointer_cast<ChannelOpenedText, AsyncObject>(shared_from_this());
	auto task = make_shared<TaskSimple>(TaskQueue(), [SThis, size](){
		*size = SThis->_ringBuffer->RecordCount();
	});
	
	return task;
}

size_t ChannelOpenedText::DataSize()
{
	return this->_ringBuffer->RecordCount();
}

char * ChannelOpenedText::GetData(size_t index, bool bFromOldest)
{
	char * addr = 0;
	auto SThis = dynamic_pointer_cast<ChannelOpenedText, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([SThis, &addr, index, bFromOldest](){
	
		size_t dataSize = SThis->DataSize();

		if (dataSize <= index || index < 0)
		{
			addr = new char[1];
			addr[0] = 0;
			return;
		}

		int index2 = bFromOldest ? dataSize - index - 1 : index;

		size_t sizeOfData;
		bool succ = SThis->_ringBuffer->GetDataSizeByTimeStampIdx(index2, sizeOfData);

		if (succ)
		{
			char * data = new char[sizeOfData + 1];
			size_t readSize;
			succ = SThis->_ringBuffer->ReadDataByTimeStampIdx(index2, data, sizeOfData, readSize);
			if (succ)
			{
				data[readSize] = 0;
				addr = data;
				return;
			}
			else
			{
				assert(FALSE);
				delete [] data;
				return;
			}
		}

	});

	return addr;
	
}

void ChannelOpenedText::CloseSubPlotWnd()
{
	auto SThis = dynamic_pointer_cast<ChannelOpenedText, AsyncObject>(shared_from_this());
	this->DoLater([SThis](){
		if (SThis->_plotWnd)
		{
			HWND hWnd = SThis->_plotWnd->m_hWnd;
			if (hWnd)
				::PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
	});
}

void ChannelOpenedText::SeekTimeStamp(unsigned long long timestamp)
{
	auto SThis = dynamic_pointer_cast<ChannelOpenedText, AsyncObject>(shared_from_this());
	this->DoLater([SThis, timestamp](){
		if (SThis->DataSize() > 0)
		{
			unsigned long long out;
			size_t outIdx;
			bool found = SThis->_ringBuffer->GetNearestOldTimeStamp(timestamp, out, outIdx);
			if (found)
				SThis->_latestTimeIdx = outIdx;
		}
	});
}

void ChannelOpenedText::UpdateSubPlotWnd()
{
	auto SThis = dynamic_pointer_cast<ChannelOpenedText, AsyncObject>(shared_from_this());
	this->DoLater([SThis](){
		if (SThis->_plotWnd)
		{
			if (SThis->DataSize() > 0)
			{
				char * data = SThis->GetData(SThis->_latestTimeIdx, false);
				CString str(data);
				SThis->_plotWnd->PlotText(str);
				delete [] data;
			}
		}
	});
}

void ChannelOpenedText::GetRect(CRect & rect)
{
	this->DoNow([this, &rect]() mutable {
		rect = this->_rect;
	});
}

void ChannelOpenedText::OnColorUpdated()
{
	ChannelOpenedTextType::OnColorUpdated();

	auto SThis = dynamic_pointer_cast<ChannelOpenedText, AsyncObject>(shared_from_this());
	this->DoNow([SThis](){
		if (SThis->_plotWnd)
		{
			SThis->_plotWnd->SetColor(SThis->_color);
		}
	});
}

HRESULT ChannelOpenedText::AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent)
{
	ChannelOpened::AppendXmlProperty(pDom, pParent);

	auto SThis = dynamic_pointer_cast<ChannelOpenedText, AsyncObject>(shared_from_this());
	this->DoNow([SThis, pDom, pParent](){

		IXMLDOMElement *pElement = NULL;

		CreateAndAddElementNode(pDom, L"subRectTLX", L"\n\t\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_rect.TopLeft().x);

		CreateAndAddElementNode(pDom, L"subRectTLY", L"\n\t\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_rect.TopLeft().y);

		CreateAndAddElementNode(pDom, L"subRectBRX", L"\n\t\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_rect.BottomRight().x);

		CreateAndAddElementNode(pDom, L"subRectBRY", L"\n\t\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_rect.BottomRight().y);
	});

	return S_OK;
}

HRESULT ChannelOpenedText::LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement)
{
	ChannelOpened::LoadXmlElement(pElement);

	CString cs;
	MSXML2::IXMLDOMNodePtr pNode;
	pNode = pElement->selectSingleNode(L"subRectTLX");
	if (pNode == 0)
		goto RET;
	int tlx = atoi((LPCSTR)(pNode->text));

	pNode = pElement->selectSingleNode(L"subRectTLY");
	if (pNode == 0)
		goto RET;
	int tly =atoi((LPCSTR)(pNode->text));

	pNode = pElement->selectSingleNode(L"subRectBRX");
	if (pNode == 0)
		goto RET;
	int brx =atoi((LPCSTR)(pNode->text));

	pNode = pElement->selectSingleNode(L"subRectBRY");
	if (pNode == 0)
		goto RET;
	int bry = atoi((LPCSTR)(pNode->text));
	
	this->_rect.SetRect(tlx,tly,brx,bry);

RET:
	return S_OK;
}

const wchar_t * ChannelOpenedText::GetTypeName()
{
	return L"Text Channel";
}

bool ChannelOpenedText::Export(const CString & filename, bool bAll)
{
	auto SThis = dynamic_pointer_cast<ChannelOpenedText, AsyncObject>(shared_from_this());

	this->DoLater([SThis, filename, bAll](){
		FILE * fp;
		errno_t ret = _wfopen_s(&fp, filename, L"wb");

		if (ret == 0)
		{
			if (1)
			{
				SThis->_ringBuffer->Export([fp](const char * ptr, size_t length){
					fwrite(ptr, 1, length, fp);
				});
			}

			fclose(fp);
		}	
	});

	return true;
}

void ChannelOpenedText::ClearData()
{
	this->_ringBuffer->Reset();
}

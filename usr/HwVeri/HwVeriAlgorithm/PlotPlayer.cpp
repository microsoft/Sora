#include <Windows.h>
#include "PlotPlayer.h"

PlotData::PlotData(int size)
{
	bufsize = bufsize;
	buffer = new int[bufsize];
	writePointer = buffer;
	readPointer = buffer;
	readableCnt = 0;
	writableCnt = bufsize;
}

PlotData::~PlotData()
{
	delete buffer;
}

int PlotData::AddData(int * input, int size)
{
	if (size > bufsize)
		size = bufsize;

	int lenToBufEnd = bufsize - (writePointer - buffer);
	if (lenToBufEnd >= size)
	{
		memcpy(writePointer, input, sizeof(int)*size);
		writePointer += size;
	}
	else
	{
		memcpy(writePointer, input, sizeof(int)*lenToBufEnd);
		writePointer = buffer;
		memcpy(writePointer, input+lenToBufEnd, size-lenToBufEnd);
		writePointer += size-lenToBufEnd;
	}

	if (writePointer == buffer + bufsize)
	{
		writePointer = buffer;
	}

	readableCnt += size;
	if (readableCnt > bufsize)
	{
		readableCnt = bufsize;
	}

	return size;
}

int PlotData::GetNewestData(int * output, int size)
{
	if (size > readableCnt)
		size = readableCnt;

	int lenFromBegin = writePointer - buffer;
	if (lenFromBegin >= size)
	{
		memcpy(output, writePointer - size, size);
	}
	else
	{
		int copysize2 = lenFromBegin;
		int copysize1 = size - copysize2;

		int * src1 = buffer + bufsize - copysize1;
		memcpy(output, src1, sizeof(int)*copysize1);
		memcpy(output + copysize1, buffer, copysize2);
	}

	readableCnt -= size;

	return size;
}

Ploter::Ploter(PlotData * aData, int aBufsize)
{
	data = aData;
	bufsize = aBufsize;
	windowSize = aWindowSize;
	buffer = new int[bufsize];
	bufferDouble = new double[bufsize];
	curPointer = buffer;
	curPointerDouble = bufferDouble;
	validDataLen = 0;
}

void Ploter::AddGraph(Graph * graph)
{
	graphs.push_back(graph);
}

void Ploter::Plot()
{
	if (validDataLen < windowSize)	// not enough data to draw
	{
		if (validDataLen > 0)		// move the remaining data to the begining of the buffer
		{
			memcpy(bufferDouble, curPointerDouble, validDataLen*sizeof(double));
		}
		int len = data->GetNewestData(buffer/* + validDataLen*/, bufsize-validDataLen);
		for (int i = 0; i < len; i++)
		{
			*(bufferDouble + validDataLen + i) = buffer[i];
		}

		validDataLen += len;
		curPointer = buffer;
	}

	if (validDataLen >= windowSize)
	{
		for (int i = 0; i < graphs.size(); i++)
		{
			graphs[i]->Draw(curPointerDouble, windowSize);
		}

		curPointer++;
		validDataLen--;
	}
	else
	{
		//TODO: insert blank data
	}
}

Series::Series(int len, int dataSize)
{
	graph = 0;

	this->len = len;
	this->dataSize = dataSize;

	buf = new int[len*dataSize];

	memset(buf, 0, len*sizeof(int));
	curPtr = buf;
}

Series::~Series()
{
	delete [] buf;
}

void Series::SetGraph(Graph * graph)
{
	this->graph = graph;
}

int Series::GetDataSize()
{
	return dataSize;
}

int Series::GetDataCount()
{
	
}

int Series::PushData(int * data, int cnt)
{
	if (cnt != dataSize)
		return 0;
	else
	{
		for (int i = 0; i < dataSize; i++)
		{
			*curPtr = data[i];
			curPtr++;
		}
		if (curPtr == buf + len)
			curPtr = buf;
	}
}

int Graph::AddSeries()
{
	Series * series = new Series();
	this->series.push_back(series);
	series->SetGraph(this);
}




OverviewGraph::OverviewGraph(int windowsize)
{
	plotBuf = 0;

	wndOverview = new CAnimWnd();
	wndOverview->Create("Overview", 320, 320, 300, 300);
	wndOverview->EraseCanvas();
	wndOverview->SetScale(-1000, 1000);
	wndOverview->ShowWindow();	
}

OverviewGraph::~OverviewGraph()
{
}

void OverviewGraph::Draw()
{
	wndOverview->EraseCanvas();
	for (int i = 0; i < series.size(); i++)
	{
		int * buf = series[i]->GetPlotBuf();
		for (int 
	}
	wndOverview->UpdateCanvas();
}


//void OverviewGraph::PushData(int * data)
//{
//	*curPtr = *data;
//	curPtr++;
//	if (curPtr == buf + bufLen)
//		curPtr = buf;
//
//	if (curPtr == buf)
//	{
//		wndOverview->PlotLine(buf, bufLen, RGB(0, 200, 0));
//	}
//	else
//	{
//		int len1 = curPtr - buf;
//		int len2 = bufLen - len1;
//		memcpy(plotBuf, curPtr, len2);
//		memcpy(plotBuf+len2, buf, len1);
//		wndOverview->PlotLine(plotBuf, bufLen, RGB(0, 200, 0));
//	}
//}
//
//int OverviewGraph::GetDataSize()
//{
//	return 1;
//}

ConstelGraph::ConstelGraph(int windowsize)
{
	bufLen = windowsize;

	buf = new COMPLEX[bufLen];
	memset(buf, 0, bufLen*sizeof(COMPLEX));
	curPtr = buf;

	wndConstel = new CAnimWnd();
	wndConstel->Create("Constel", 640, 320, 300, 300);
	wndConstel->EraseCanvas();
	wndConstel->SetScale(-1000, 1000);
	wndConstel->ShowWindow();
}

ConstelGraph::~ConstelGraph()
{
	if (buf)
		delete [] buf;

	delete wndConstel;
}

void ConstelGraph::PushData(int * data)
{
	(*curPtr).re = *data;
	(*curPtr).im = *(data+1);

	curPtr++;
	if (curPtr == buf + bufLen)
		curPtr = buf;

	if (curPtr == buf)
	{
		wndConstel->PlotDots(buf, bufLen, RGB(0, 200, 0));
	}
	else
	{
		int len1 = curPtr - buf;
		int len2 = bufLen - len1;
		memcpy(plotBuf, curPtr, len2);
		memcpy(plotBuf+len2, buf, len1);
		wndConstel->PlotDots(plotBuf, bufLen, RGB(0, 200, 0));
	}	
}

int ConstelGraph::GetDataSize()
{
	return 2;
}

void PlotPlayer::AddPloter(Ploter * ploter)
{
	ploters.push_back(ploter);
}

void PlotPlayer::Play()
{
	DWORD threadID;
	threadPlayer = CreateThread(
		NULL,              // default security
		0,                 // default stack size
		PlotThread,        // name of the thread function
		this,              // no thread parameters
		0,                 // default startup flags
		&threadID); 
}

DWORD WINAPI PlotPlayer::PlotThread(PVOID arglist)
{
	PlotPlayer * player = (PlotPlayer *)arglist;

	while(1)
	{
		if (CheckExit())
			break;
		CheckPause();
		::Sleep(100);
		for (int i = 0; i < player->ploters.size(); i++)
		{
			player->ploters[i]->Plot();
		}
	}
	return 0;
}

void Stop()
{
	thread.stop();
}

#define SIZE_16K (16*1024)

void UnitTest()
{
	const int SIZE_1K = 1024;

	PlotPlayer * player = new PlotPlayer();

	PlotData * overviewData = new PlotData(SIZE_1K);
	PlotData * constelData = new PlotData(SIZE_1K);

	OverviewGraph * overviewGragh = new OverviewGraph();
	ConstelGraph * constelGraph = new ConstelGraph();
	
	Ploter * overviewPloter = new Ploter(overviewData, SIZE_16K, 100);
	Ploter * constelPloter = new Ploter(constelPloter, SIZE_16K, 100);

	player->AddPloter(overviewPloter);
	player->AddPloter(constelPloter);
	player->Play();

	int * overviewBuf = new int[SIZE_1K];
	int * constelBuf = new int[SIZE_1K];

	while(1)
	{
		overviewData->AddData(overviewBuf, SIZE_1K);
		overviewData->AddData(overviewBuf, SIZE_1K);
		::Sleep(1);
	}

	player->Stop();
}


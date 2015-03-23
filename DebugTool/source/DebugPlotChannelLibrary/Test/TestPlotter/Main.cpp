#include <Windows.h>
#include <stdio.h>
#include "sora.h"
#include "DpChannel.h"
#include "DpChannelExt.h"
//#include "vld.h"
#include "PlotData.h"
#include "PlotLine.h"
#include "PlotSpectrum.h"
#include "PlotDots.h"
#include "PlotText.h"

static const char * CH_CMD_INFO = 
	"a: DpPlotData\n" 
	"l: DpPlotLine\n"
	"s: DpPlotSpectrum\n"
	"d: DpPlotDots\n"
	"t: DpPlotText\n";

void PlotData2()
{
	while(1)
	{
		::DpPlotData("some data", 100, "1234567890", 10);
	}
}

int __cdecl main(int argc, char **argv)
{
	printf("TestPlotter started\n");

	if (argc < 2)
	{
		printf("error, please specify the channel type\n");
		printf(CH_CMD_INFO);
		return 0;
	}

	char type = *argv[1];

	switch(type)
	{
	case 'a':
		PlotData();
		break;
	case 'l':
		PlotLine();
		break;
	case 's':
		PlotSpectrum();
		break;
	case 'd':
		PlotDots();
		break;
	case 't':
		PlotText();
		break;
	case 'o':
		PlotData2();
		break;
	default:
		printf("error, unknown type\n)");
		printf(CH_CMD_INFO);
	}
}

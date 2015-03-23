// TestPlot.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Plot.h"
#include "simplewnd.h"
#include "SoraThread.h"

void Test__Plot()
{
	const int AMP = 12000;

	CAnimWnd * window1;
	window1 = new CAnimWnd ( );
	window1->Create ( "washingweb", 640, 220, 320, 320 );
	window1->EraseCanvas ();
	window1->SetScale ( -AMP, AMP );
	window1->ShowWindow ();

	printf("%x\n", window1);

	Series<double> overviewSeries(200);
	DrawLine * drawline = new DrawLine(window1, 0);
	drawline->SetColor(RGB(0,255,0));
	overviewSeries.SetDrawMethod(drawline);

	PlotImpl<double> * plot1 = new PlotImpl<double>(&overviewSeries, 600);
	plot1->SetTimeIntervalMS(20);
	
	CAnimWnd * window2;
	window2 = new CAnimWnd ( );
	window2->Create ( "washingweb", 300, 220, 320, 320 );
	window2->EraseCanvas ();
	window2->SetScale ( -AMP, AMP );
	window2->ShowWindow ();

	printf("%x\n", window2);

	Series<COMPLEX> constelSeries(20);
	DrawDots * drawdots = new DrawDots(window2, 0);
	drawdots->SetColor(RGB(255,0,0));
	constelSeries.SetDrawMethod(drawdots);

	PlotImpl<COMPLEX> * plot2 = new PlotImpl<COMPLEX>(&constelSeries, 600);
	plot2->SetTimeIntervalMS(20);

	PlotPlayer * player = new PlotPlayer();
	player->AddPlot(plot1);
	player->AddPlot(plot2);
	player->SetTimeIntervalMS(1);

	player->Start(true);

	int time = 0;

	const double PI = 3.14159;
	const double PERIOD = 50;
	const int LEN = 2000;

	double buf[LEN];
	double bufOverview[LEN/2];

	for (int i = 0; i < LEN/2; i++)
	{
		double re = AMP * cos((double)time*2*PI/PERIOD);
		double im = AMP * sin((double)time*2*PI/PERIOD);
		buf[2*i] = re;
		buf[2*i+1] = im;
		bufOverview[i] = re;

		time++;
	}

	while(1)
	{
		//::Sleep(1000);
		double re = AMP * cos((double)time*2*PI/PERIOD);
		double im = AMP * sin((double)time*2*PI/PERIOD);
		COMPLEX complex;
		complex.re = re;
		complex.im = im;
		
		plot1->PushData(bufOverview, LEN/2);
		plot2->PushData(buf, LEN);

		::Sleep(1);
		time++;		
	}
}


void Test__CAnimWnd_Series_DrawMethos()
{
	//return test1(argc, argv);
	const int AMP = 12000;

	CAnimWnd * window1;
	window1 = new CAnimWnd ( );
	window1->Create ( "washingweb", 300, 220, 320, 320 );
	window1->EraseCanvas ();
	window1->SetScale ( -AMP, AMP );
	window1->ShowWindow ();

	CAnimWnd * window2;
	window2 = new CAnimWnd ( );
	window2->Create ( "washingweb", 640, 220, 320, 320 );
	window2->EraseCanvas ();
	window2->SetScale ( -AMP, AMP );
	window2->ShowWindow ();

	Series<double> overviewSeries(200);
	DrawLine * drawline = new DrawLine(window1, 0);
	drawline->SetColor(RGB(0,255,0));
	overviewSeries.SetDrawMethod(drawline);

	Series<COMPLEX> complexSeries(10);
	DrawDots * drawdots = new DrawDots(window2, 0);
	drawdots->SetColor(RGB(255,0,0));
	complexSeries.SetDrawMethod(drawdots);


	const double PI = 3.14159;
	const double PERIOD = 50;
	const int LEN = 200;

	int time = 0;

	while(1)
	{
		double re = AMP * cos((double)time*2*PI/PERIOD);
		double im = AMP * sin((double)time*2*PI/PERIOD);
		COMPLEX complex;
		complex.re = re;
		complex.im = im;
		overviewSeries.PushData(re);
		complexSeries.PushData(complex);
		::Sleep(50);
		time++;
	}
}


void HwVeri()
{
	const int AMP = 1200;

	CAnimWnd * wndOverview = new CAnimWnd ( );
	wndOverview->Create ( "overview", 0, 0, 320, 320 );
	wndOverview->EraseCanvas ();
	wndOverview->SetScale ( -AMP, AMP );
	wndOverview->ShowWindow();

	Series<double> * seriesOverview = new Series<double>(200);
	DrawLine * drawline = new DrawLine(wndOverview, 0);
	drawline->SetColor(RGB(0,255,0));
	seriesOverview->SetDrawMethod(drawline);

	PlotImpl<double> * plotOverview = new PlotImpl<double>(seriesOverview, 200);


	CAnimWnd * wndConstel = new CAnimWnd ( );
	wndConstel->Create ( "constel", 0, 0, 320, 320 );
	wndConstel->EraseCanvas ();
	wndConstel->SetScale ( -AMP, AMP );
	wndConstel->ShowWindow();

	Series<COMPLEX> * seriesConstel = new Series<COMPLEX>(200);
	DrawDots * drawdots = new DrawDots(wndConstel, 0);
	drawdots->SetColor(RGB(255,0,0));
	seriesConstel->SetDrawMethod(drawdots);

	PlotImpl<COMPLEX> * plotConstel = new PlotImpl<COMPLEX>(seriesConstel, 100);

	PlotPlayer * plotPlayer = new PlotPlayer();
	plotPlayer->AddPlot(plotOverview);
	plotPlayer->AddPlot(plotConstel);
	plotPlayer->SetTimeIntervalMS(1);

	plotPlayer->Start(false);

	const double PI = 3.14159;
	const double PERIOD = 50;
	const int LEN = 200;

	int time = 0;

	while(1)
	{
		double re = AMP * cos((double)time*2*PI/PERIOD);
		double im = AMP * sin((double)time*2*PI/PERIOD);
		COMPLEX complex;
		complex.re = re;
		complex.im = im;
		seriesOverview->PushData(re);
		seriesConstel->PushData(complex);
		::Sleep(50);
		time++;
	}
}

int __cdecl _tmain(int argc, _TCHAR* argv[])
{
	//Test__CAnimWnd_Series_DrawMethos();
	Test__Plot();
	//HwVeri();

	return 0;
}


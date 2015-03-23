#include <windows.h>
#include <WinUser.h>
#include <CommCtrl.h>
#include <Windowsx.h>
#include <wchar.h>
#include <Richedit.h>
#include "Logger.h"
#include "LoggerImpl.h"
#include "HwApp.h"
#include "Message.h"

#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define SLIDER_GAIN_RANGE 65535

static Logger * g_logger;
static Application * g_app;
static HINSTANCE g_hInstance;


LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;
BOOL CALLBACK DlgProc (HWND, UINT, WPARAM, LPARAM) ;
void InitDialog();

HWND g_hDlg;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
{
	g_hInstance = hInstance;

	HMODULE hRE = LoadLibrary(L"riched20.dll");

	g_app = new Application(); 


	static TCHAR szAppName[] = /*TEXT ("HexCalc")*/TEXT("hwveridlg") ;
	//HWND         hwnd ;
	MSG          msg ;
	WNDCLASS     wndclass ;

	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = WndProc ;
	wndclass.cbClsExtra    = 0 ;
	wndclass.cbWndExtra    = DLGWINDOWEXTRA ;    // Note!
	wndclass.hInstance     = hInstance ;
	wndclass.hIcon         = LoadIcon (hInstance, L"HWVERIDLG_ICO") ;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	wndclass.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1) ;
	wndclass.lpszMenuName  = NULL ;
	wndclass.lpszClassName = szAppName ;

	if (!RegisterClass (&wndclass))
	{
		MessageBox (NULL, TEXT ("This program requires Windows NT!"),
			szAppName, MB_ICONERROR) ;
		return 0 ;
	}

	g_hDlg = CreateDialog (hInstance, szAppName, 0, DlgProc) ;

	g_app->Init(g_hDlg);
	
	InitDialog();

	ShowWindow (g_hDlg, iCmdShow) ;

	while (GetMessage (&msg, NULL, 0, 0))
	{
		TranslateMessage (&msg) ;
		DispatchMessage (&msg) ;
	}

	FreeLibrary(hRE);
	return msg.wParam ;
}


LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     static BOOL  bNewNumber = TRUE ;
     static int   iOperation = '=' ;
     static UINT  iNumber, iFirstNum ;
     //HWND         hButton ;
     
     switch (message)
     {
	 case WM_COMMAND:
		 return 0;
     case WM_DESTROY:
          PostQuitMessage (0) ;
          return 0 ;
     }
     return DefWindowProc (hwnd, message, wParam, lParam) ;
}


void ClearLog()
{
	HWND outputWindow = GetDlgItem(g_hDlg, IDC_RICHEDIT21_LOG);
	SendMessage(outputWindow, WM_SETTEXT, 0, (LPARAM)L"");
}

void DlgLog(int type, wchar_t * string)
{
	long nBegin, nEnd;

	HWND hRichEdit = GetDlgItem(g_hDlg, IDC_RICHEDIT21_LOG);

	CHARRANGE selection;

	// Set Selection
	selection.cpMin = -1;
	selection.cpMax = -1;
	SendMessage(hRichEdit, EM_EXSETSEL, 0, (LPARAM)&selection);

	// Get selection
	SendMessage(hRichEdit, EM_EXSETSEL, 0, (LPARAM)&selection);
	nBegin = selection.cpMin;
	nEnd = selection.cpMax;

	// replace selection
	SendMessage(hRichEdit, EM_REPLACESEL, FALSE, (LPARAM)string);

	// select the text just appended
	selection.cpMin = nBegin;
	selection.cpMax = -1;
	SendMessage(hRichEdit, EM_SETSEL, 0, (LPARAM)&selection);
	SendMessage(hRichEdit, WM_VSCROLL, SB_BOTTOM, 0);

	delete [] string;

	//CHARFORMAT2 cf;
	//outputWindow->GetDefaultCharFormat(cf);

	//cf.dwEffects &= ~CFE_AUTOCOLOR;
	//cf.dwMask |= CFM_COLOR;

	//int type = 0;
	//switch(type)
	//{
	//default:
	//	cf.crTextColor = RGB(0, 0, 0);
	//}

	//outputWindow->SetSelectionCharFormat(cf);
}

//void LoadBin()
//{
// HGLOBAL     res_handle = NULL;
//    HRSRC       res;
//    char *      res_data;
//    DWORD       res_size;
//
//    // NOTE: providing g_hInstance is important, NULL might not work
//    res = FindResource(g_hInstance, TEXT("BINTYPE"), MAKEINTRESOURCE(IDR_OFDM_DATA));
//    if (!res)
//        return;
//    res_handle = LoadResource(NULL, res);
//    if (!res_handle)
//        return;
//    res_data = (char*)LockResource(res_handle);
//    res_size = SizeofResource(NULL, res);
//	g_logger->Log(0, L"res_size %d\n");
//}

static std::vector<int> gVecRadioSelTable;

void InitRadioSel()
{
	Radio::Init();
	std::vector<int> availableRadioIds = Radio::AvailableRadios();
	HWND comboRxpa = GetDlgItem(g_hDlg, IDC_COMBO_RADIO_SEL);
	wchar_t buf[32];

	for (std::vector<int>::iterator it = availableRadioIds.begin(); it != availableRadioIds.end(); ++it)
	{
		int radioIdx = *it;
		swprintf(buf, L"%d", radioIdx);
		ComboBox_AddString(comboRxpa, buf);
		gVecRadioSelTable.push_back(radioIdx);
	}

	if (ComboBox_GetCount(comboRxpa) > 0)
	{
		ComboBox_SetCurSel(comboRxpa, 0);
		Radio::SelectRadio(availableRadioIds[0]);
	}
}

void InitComboRxpa()
{
	HWND comboRxpa = GetDlgItem(g_hDlg, IDC_COMBO_RXPA);
	//ComboBox_AddString(comboRxpa, L"0x0");
	ComboBox_AddString(comboRxpa, L"  0 dB");
	ComboBox_AddString(comboRxpa, L"16 dB");
	ComboBox_AddString(comboRxpa, L"32 dB");
	ComboBox_SetCurSel(comboRxpa, 1);
}

void InitComboSampleRate()
{
	HWND comboSampleRate = GetDlgItem(g_hDlg, IDC_COMBO_SAMPLE_RATE);
	ComboBox_AddString(comboSampleRate, L"40 MHz");
	ComboBox_AddString(comboSampleRate, L"44 MHz");
	ComboBox_SetCurSel(comboSampleRate, 0);
}

void InitSliderGain()
{
	HWND sliderGain = GetDlgItem(g_hDlg, IDC_SLIDER_GAIN);
	//SetScrollRange(sliderGain, SB_CTL, 20, 80, TRUE);
	SendMessage(sliderGain, TBM_SETRANGEMIN, TRUE, 0);
	SendMessage(sliderGain, TBM_SETRANGEMAX, TRUE, SLIDER_GAIN_RANGE);
}

void InitLogger()
{
	LogMethod * logMethod = new DlgLogger(g_hDlg);

	Logger::GetLogger(L"UI")->SetLogMethod(logMethod, false);
	Logger::GetLogger(L"TaskQueue")->SetLogMethod(logMethod, false);
	Logger::GetLogger(L"Utility")->SetLogMethod(logMethod, false);
	Logger::GetLogger(L"App")->SetLogMethod(logMethod, false);
	Logger::GetLogger(L"umx")->SetLogMethod(logMethod, false);
	Logger::GetLogger(L"alg")->SetLogMethod(logMethod, false);
	Logger::GetLogger(L"radio")->SetLogMethod(logMethod, false);

	g_logger = Logger::GetLogger(L"UI");
}

void InitMode()
{
	CheckRadioButton(g_hDlg, IDC_RADIO_LOOP, IDC_RADIO_SNR, IDC_RADIO_SINE);
	g_app->SelectModeSine();
}

void InitDirection()
{
	CheckRadioButton(g_hDlg, IDC_RADIO_SEND, IDC_RADIO_RECEIVE, IDC_RADIO_SEND);
	g_app->SelectDirectionSend();
}

void InitAutogain()
{
	::SendMessageA(GetDlgItem(g_hDlg, IDC_CHECK_AUTOGAIN), BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
}

void OnInitDialog()
{
	InitLogger();
	InitRadioSel();
	InitComboRxpa();
	InitComboSampleRate();
	InitSliderGain();
	InitAutogain();
	//LoadBin();
	//InitMode();
	//InitDirection();
}

void InitDialog()
{
	InitMode();
	InitDirection();
}

BOOL CALLBACK DlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int id;
	CAnimWnd * wnd;

	switch(message)
	{
	case WM_INITDIALOG:
		g_hDlg = hDlg;
		OnInitDialog();
		return TRUE;

	case WM_TIMER:
		g_app->OnTimer();
		return TRUE;

	case WM_LBUTTONDOWN:
		//g_logger->Log(0, L"l button down\r\n");
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_START:
			g_app->OnClickButtonStart();
			return TRUE;

		case IDC_BUTTON_SUGGESTION:
			g_app->OnSuggestion();
			return TRUE;

		case IDC_BUTTON_CALIBRATION:
			g_app->OnClickCalibration();
			return TRUE;

		case IDC_RADIO_LOOP:
			return TRUE;

		case IDC_RADIO_SINE:
		case IDC_RADIO_SNR:
			id = LOWORD(wParam);
			CheckRadioButton(hDlg, IDC_RADIO_LOOP, IDC_RADIO_SNR, id);
			
			if (id == IDC_RADIO_SINE)
				g_app->SelectModeSine();
			else
				g_app->SelectModeSnr();

			return TRUE;

		// select direction
		case IDC_RADIO_SEND:
		case IDC_RADIO_RECEIVE:
			id = LOWORD(wParam);
			CheckRadioButton(hDlg, IDC_RADIO_SEND, IDC_RADIO_RECEIVE, id);
			
			if (id == IDC_RADIO_SEND)
				g_app->SelectDirectionSend();
			else
				g_app->SelectDirectionReceive();

			return TRUE;

		case IDC_SLIDER_GAIN:
			return TRUE;

		case IDC_COMBO_RXPA:
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				int selection = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBO_RXPA));
				int rxpa;
				switch(selection)
				{
				case 0:
					rxpa = 0x1000;
					break;
				case 1:
					rxpa = 0x2000;
					break;
				case 2:
					rxpa = 0x3000;
					break;
				default:
					rxpa = 0x0;
				}

				g_app->SetRxpa(rxpa);
			}
			return TRUE;

		case IDC_COMBO_SAMPLE_RATE:
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				int selection = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBO_SAMPLE_RATE));
				switch (selection)
				{
				case 0:
					g_app->SetSampleRate(40);
					break;
				case 1:
					g_app->SetSampleRate(44);
					break;
				}
			}
			return TRUE;

		case IDC_COMBO_RADIO_SEL:
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				int selection = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBO_RADIO_SEL));
				int radioId = gVecRadioSelTable[selection];
				wchar_t buf[32];

				g_app->SaveOpState();
				Radio::SelectRadio(radioId);
				g_app->LoadOpState();
				g_app->ReconfigUI();
				g_app->UpdateUIPara();
				swprintf(buf, L"Radio %d selected\n", radioId);
				Logger::GetLogger(L"UI")->Log(LOG_DEFAULT, buf);
			}
			return TRUE;

		case IDC_BUTTON_CLR_LOG:
			//g_logger->Log(0, L"Button clear\r\n");
			ClearLog();
			return TRUE;

		case IDC_BUTTON_LOAD_PARA:
			g_app->LoadPara();
			return TRUE;

		case IDC_BUTTON_SAVE_PARA:
			g_app->SavePara();
			return TRUE;

		case IDC_BUTTON_DUMP:
			g_app->Dump();
			return TRUE;

		case IDC_BUTTON_SAVE_LOG:
			g_app->SaveLog();
			return TRUE;
		case IDC_CHECK_AUTOGAIN:
			do
			{
				LRESULT lr = ::SendMessage(GetDlgItem(g_hDlg, IDC_CHECK_AUTOGAIN), BM_GETCHECK, 0, 0);
				bool checked = false;
				if (lr == BST_CHECKED)
					checked = true;
				g_app->SetAutoGain(checked);
			} while(0);
			return TRUE;
		}

		return TRUE;

	case WM_HSCROLL:
		switch(LOWORD(wParam))
		{
		case TB_ENDTRACK:
			do {
				int sel = ::SendMessage(GetDlgItem(hDlg, IDC_SLIDER_GAIN), TBM_GETPOS, 0, 0);
				g_app->SetGain(sel, SLIDER_GAIN_RANGE);
			} while(0);
			break;
		case TB_THUMBTRACK:
			g_app->OnGainSliderChange(HIWORD(wParam), SLIDER_GAIN_RANGE);
			break;
		}
		return TRUE;
		
	case WM_CLOSE:
		g_app->Deinit();
		::PostMessage(hDlg, WM_CLEAN_BEFORE_QUIT, 0, 0);
		return TRUE;

	case WM_LOGGER:
		g_app->DlgLog((int)wParam, (wchar_t *)lParam);
		//DlgLog((int)wParam, (wchar_t *)lParam);
		return TRUE;

	case WM_TASK_DONE:
		g_app->ProcessTaskMessage((TASK_TYPE)wParam, (void *)lParam);
		return TRUE;

	case WM_CLEAN_BEFORE_QUIT:
		//::Sleep(1000);
		PostQuitMessage (0);
		return TRUE;

	case WM_UPDATE_CANVAS:
		wnd = (CAnimWnd *)wParam;
		wnd->UpdateCanvas();
		return TRUE;

	case WM_DESTORY_PLOT:
		wnd = (CAnimWnd *)wParam;
		::SendMessage(wnd->HWnd(), WM_DESTROY, 0, 0);
		delete wnd;
		return TRUE;

	case WM_SHOW_WINDOW:
		::ShowWindow((HWND)wParam, (int)lParam);
		return TRUE;

	case WM_NEW_DUMP_DATA:
		//delete [] (char *)lParam;
		g_app->AddDumpData((char *)lParam, (int)wParam);
		return TRUE;

	case WM_UPDATE_UI:
		g_app->UpdateUIPara();
		return TRUE;

	case WM_UPDATE_DATA:
		g_app->UpdateData((UPDATE_DATA_TYPE)wParam, (double *)lParam);
		return TRUE;

	case WM_RECONFIG_UI:
		g_app->ReconfigUI();
		return TRUE;
	
	//case WM_UPDATA_STATUS_BAR:
	//	g_app->RefreshStatusBar();
	//	return TRUE;
	//case WM_THREAD_START:
	//	g_app->OnThreadStart();
	//	return TRUE;

	//case WM_THREAD_EXIT:
	//	g_app->OnThreadExit();
	//	return TRUE;
	}

	return FALSE;
}

#pragma once

#include "Invokable.h"
#include "CustomButton.h"
#include "Event.h"

// ControlPanelWnd

class ControlPanelWnd : public Invokable
{
	DECLARE_DYNAMIC(ControlPanelWnd)

public:
	static int Height();
	static int ButtonWidth();

public:
	ControlPanelWnd(const CString &);
	virtual ~ControlPanelWnd();

	void SetCaptionWidth(int width);
	void SetCaption(const CString & caption);
	void SetReadWritePos(float readPos, float writePos);
	void SetPlayPauseButtonState(bool bPlay);
	void SetBitmap(Bitmap * bitmap);
	void EnableButton(bool bEnable);

	SoraDbgPlot::Event::Event<bool> EventClosed;
	SoraDbgPlot::Event::Event<bool> EventPlayPause;
	SoraDbgPlot::Event::Event<bool> EventSingleStep;
	SoraDbgPlot::Event::Event<CRect> EventTraceBarSizeChanged;
	SoraDbgPlot::Event::Event<bool> EventTraceBarWheel;
	SoraDbgPlot::Event::Event<double> EventTraceBarSeek;

private:
	void AdjustLayout();
	void DrawCaption(Graphics * g);
	void DrawTrackBar(Graphics * g);

private:
	bool _buttonEnabled;
	Bitmap * _bitmap;
	CString _caption;
	float _readPos;
	float _writePos;
	bool _buttonIsPlayState;

	enum {
		ID_PLAY_BUTTON = 1,
		ID_SINGLE_STEP_BUTTON
	};

	CFont _buttonFont;
	CustomButton _playButton;
	CustomButton _singleStepButton;
	int _captionWidth;

	int _captionLeft;
	int _captionRight;
	int _trackBarLeft;
	int _trackBarRight;
	int _playButtonLeft;
	int _playButtonRight;
	int _singleStepButtonLeft;
	int _singleStepButtonRight;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnPlayPauseButtonClicked();
	afx_msg void OnSingleStepButtonClicked();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PostNcDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};



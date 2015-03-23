#pragma once


// CustomButton

enum ButtonType
{
	TYPE_PLAY,
	TYPE_PAUSE,
	TYPE_PLUS,
	TYPE_MINUS,
	TYPE_AUTO_REPLAY,
	TYPE_AUTO_PAUSE,
};

class CustomButton : public CButton
{
	DECLARE_DYNAMIC(CustomButton)

public:
	CustomButton();
	virtual ~CustomButton();
	void SetType(enum ButtonType);

protected:
	DECLARE_MESSAGE_MAP()
public:
//	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
private:
	Bitmap* bmp;

	//Bitmap* bmpPlay = 0;
	//Bitmap* bmpPause = 0;
	//Bitmap* bmpPlus = 0;
	//Bitmap* bmpMinus = 0;
	//Bitmap* bmpAutoReplay = 0;
	//Bitmap* bmpAutoPause = 0;
};



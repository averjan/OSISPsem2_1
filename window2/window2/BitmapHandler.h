#pragma once

#include <windows.h>
#include <string>

class BitmapHandler
{
private:
	bool IsMovableLeft(int direct);
	bool IsMovableRight(int direct, RECT clientRect);
	bool IsMovableTop(int direct);
	bool IsMovableBottom(int direct, RECT clientRect);
	bool CalculatePossibleXOffset(int* offsetX);
	bool CalculatePossibleYOffset(int* offsetY);
public:
	HWND parentHwnd;
	HDC bmpContext;
	HDC canvasHdc;
	int x, y;
	int width, height;
	int speed;
	RECT usrRec;

	BitmapHandler(HWND parentWindow, LPCWSTR fileName);

	void Draw();

	/*
	* Moves object's bitmap using given offset
	* Return false if it's impossible
	*/
	bool MoveRect(int offsetX, int offsetY);

	bool AutoMoveRect();
};


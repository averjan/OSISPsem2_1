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
	bool IsLeftHit(int direct);
	bool IsRightHit(int direct, RECT clientRect);
	bool IsTopHit(int direct);
	bool IsBottomHit(int direct, RECT clientRect);
	bool CalculatePossibleXOffset(int* offsetX);
	bool CalculatePossibleYOffset(int* offsetY);

	void SetAutoMoveDirections(int* offsetX, int* offsetY);

public:
	HWND parentHwnd;
	HDC bmpContext;
	HDC canvasHdc;
	RECT usrRec;
	int x, y;
	int width, height;
	int speed;
	double motionAngle;
	const int step = 5;
	int speedX, speedY;

	BitmapHandler(HWND parentWindow, LPCWSTR fileName);
	~BitmapHandler();

	void Draw();

	/*
	* Moves object's bitmap using given offset
	* Return false if it's impossible
	*/
	bool MoveRect(int offsetX, int offsetY);

	void AutoMoveRect();

	bool PtInBmp(POINT pt);

	void GenerateMotionAngle();
};


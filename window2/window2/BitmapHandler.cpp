#include "BitmapHandler.h"

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

BitmapHandler::BitmapHandler(HWND parentWindow, LPCWSTR fileName)
{
	this->x = 0;
	this->y = 0;
    this->speed = 5;
    this->parentHwnd = parentWindow;

    BITMAP bitmapInfo;
    HBITMAP hbmp = (HBITMAP)LoadImage(NULL, fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    GetObject(hbmp, sizeof(BITMAP), &bitmapInfo);
    this->width = bitmapInfo.bmWidth;
    this->height = bitmapInfo.bmHeight;

    this->canvasHdc = GetDC(parentWindow);
    this->bmpContext = CreateCompatibleDC(this->canvasHdc);
    SelectObject(this->bmpContext, hbmp);
    SetRect(&(this->usrRec), x, y, x + width, y + height);
}

void BitmapHandler::Draw()
{
    TransparentBlt(canvasHdc, usrRec.left, usrRec.top, width, height, bmpContext,
        0, 0, width, height, GetSysColor(COLOR_WINDOW));
}

bool BitmapHandler::MoveRect(int offsetX, int offsetY)
{
    if (!CalculatePossibleXOffset(&offsetX))
    {
        return false;
    }

    if (!CalculatePossibleYOffset(&offsetY))
    {
        return false;
    }

    OffsetRect(&usrRec, offsetX, offsetY);
    return true;
}

bool BitmapHandler::CalculatePossibleXOffset(int* offsetX)
{
    int direct = sgn(*offsetX);
    RECT clientRect;
    GetClientRect(parentHwnd, &clientRect);
    if (IsMovableLeft(direct))
    {
        *offsetX = (usrRec.left > speed) ? speed : usrRec.left;
    }
    else if (IsMovableRight(direct, clientRect))
    {
        int rightOffset = clientRect.right - (usrRec.right + clientRect.left);
        *offsetX = (rightOffset > speed) ? speed : rightOffset;
    }
    else
    {
        return false;
    }

    return true;
}

bool BitmapHandler::CalculatePossibleYOffset(int* offsetY)
{
    int direct = sgn(*offsetY);
    RECT clientRect;
    GetClientRect(parentHwnd, &clientRect);
    if (IsMovableTop(direct))
    {
        *offsetY = (usrRec.top > speed) ? speed : usrRec.top;
    }
    else if (IsMovableBottom(direct, clientRect))
    {
        int bottomOffset = clientRect.bottom - (usrRec.bottom + clientRect.top);
        *offsetY = (bottomOffset > speed) ? speed : bottomOffset;
    }
    else
    {
        return false;
    }

    return true;
}

bool BitmapHandler::IsMovableLeft(int direct)
{
    return (direct < 0) && (usrRec.left != 0);
}

bool BitmapHandler::IsMovableRight(int direct, RECT clientRect)
{
    return (direct > 0) && (usrRec.right + clientRect.left != clientRect.right);
}

bool BitmapHandler::IsMovableTop(int direct)
{
    return (direct < 0) && (usrRec.top != 0);
}

bool BitmapHandler::IsMovableBottom(int direct, RECT clientRect)
{
    return (direct > 0) && (usrRec.bottom + clientRect.top != clientRect.bottom);
}
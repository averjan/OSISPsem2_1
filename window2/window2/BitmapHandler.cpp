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
    GenerateMotionAngle();
}

void BitmapHandler::Draw()
{
    TransparentBlt(canvasHdc, usrRec.left, usrRec.top, width, height, bmpContext,
        0, 0, width, height, GetSysColor(COLOR_WINDOW));
}

bool BitmapHandler::MoveRect(int offsetX, int offsetY)
{
    CalculatePossibleXOffset(&offsetX);

    CalculatePossibleYOffset(&offsetY);

    OffsetRect(&usrRec, offsetX, offsetY);
    return true;
}

bool BitmapHandler::CalculatePossibleXOffset(int* offsetX)
{
    if (*offsetX > 0)
    {
        int a = 12;
    }

    int direct = sgn(*offsetX);
    RECT clientRect;
    GetClientRect(parentHwnd, &clientRect);
    if (IsMovableLeft(direct))
    {
        *offsetX = (usrRec.left > abs(*offsetX)) ? *offsetX : direct * usrRec.left;
    }
    else if (IsMovableRight(direct, clientRect))
    {
        int rightOffset = clientRect.right - (usrRec.right + clientRect.left);
        *offsetX = (rightOffset > *offsetX) ? *offsetX : rightOffset;
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
        *offsetY = (usrRec.top > abs(*offsetY)) ? *offsetY : direct * usrRec.top;
    }
    else if (IsMovableBottom(direct, clientRect))
    {
        int bottomOffset = clientRect.bottom - (usrRec.bottom + clientRect.top);
        *offsetY = (bottomOffset > *offsetY) ? *offsetY : bottomOffset;
    }
    else
    {
        return false;
    }

    return true;
}

bool BitmapHandler::PtInBmp(POINT pt)
{
    return PtInRect(&usrRec, pt);
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

BitmapHandler::~BitmapHandler()
{
    ReleaseDC(parentHwnd, bmpContext);
    ReleaseDC(parentHwnd, canvasHdc);
}

void BitmapHandler::GenerateMotionAngle()
{
    this->motionAngle = (rand() % 360 + 1) * 3.14 / 180;
    speedX = cos(motionAngle) * step;
    speedY = sin(motionAngle) * step;
}

void BitmapHandler::AutoMoveRect()
{
    SetAutoMoveDirections(&speedX, &speedY);
    CalculatePossibleXOffset(&speedX);
    CalculatePossibleYOffset(&speedY);
    OffsetRect(&usrRec, speedX, speedY);
}

void BitmapHandler::SetAutoMoveDirections(int* offsetX, int* offsetY)
{
    int directX = sgn(*offsetX);
    int directY = sgn(*offsetY);
    RECT clientRect;
    GetClientRect(parentHwnd, &clientRect);
    if (IsLeftHit(directX) || IsRightHit(directX, clientRect))
    {
        *offsetX = -*offsetX;
    }

    if (IsTopHit(directY) || IsBottomHit(directY, clientRect))
    {
        *offsetY = -*offsetY;
    }
}

bool BitmapHandler::IsLeftHit(int direct)
{
    return (direct < 0) && (usrRec.left == 0);
}

bool BitmapHandler::IsRightHit(int direct, RECT clientRect)
{
    return (direct > 0) && (usrRec.right + clientRect.left == clientRect.right);
}

bool BitmapHandler::IsTopHit(int direct)
{
    return (direct < 0) && (usrRec.top == 0);
}

bool BitmapHandler::IsBottomHit(int direct, RECT clientRect)
{
    return (direct > 0) && (usrRec.bottom + clientRect.top == clientRect.bottom);
}
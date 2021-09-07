#pragma comment(lib, "Msimg32.lib")

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <math.h>
#include <thread>
#include "BitmapHandler.h"


const double M_PI = 3.14;

bool doingNothing = false;
HWND globalHwnd;
std::thread animation;

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

void MovePicture(HWND hwnd, RECT* rcBmp, RECT client, HDC hdcCompat, std::atomic_bool* cancellation_token);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Run the message loop.

    MSG msg = { };
    DWORD baseTime = GetTickCount64();
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;                 // device context (DC) for window
    RECT rcTmp;              // temporary rectangle
    PAINTSTRUCT ps;          // paint data for BeginPaint and EndPaint
    static POINT ptClientUL;        // client area upper left corner
    static POINT ptClientLR;        // client area lower right corner
    static HDC hdcCompat;    // DC for copying bitmap
    static POINT pt;         // x and y coordinates of cursor
    static RECT rcBmp;       // rectangle that encloses bitmap
    static RECT rcTarget;    // rectangle to receive bitmap
    static RECT rcClient;    // client-area rectangle
    static BOOL fDragRect;   // TRUE if bitmap rect. is dragged
    static HBITMAP hbmp;     // handle of bitmap to display
    static HBRUSH hbrBkgnd;  // handle of background-color brush
    static COLORREF crBkgnd; // color of client-area background
    static HPEN hpenDot;     // handle of dotted pen
    static HPEN transPen;
    static int width;
    static int height;
    static std::thread t;
    static std::atomic_bool cancellation_token;
    static UINT_PTR IDT_TIMER1;
    static bool bouncingLaunched = false;
    BITMAP bitmapInfo;
    HBRUSH hbrWhite = (HBRUSH)GetStockObject(GRAY_BRUSH);
    static BitmapHandler* bmpHand = new BitmapHandler(hwnd, L"a3.bmp");

    doingNothing = false;
    switch (uMsg)
    {
    case WM_CREATE: {
        globalHwnd = hwnd;
        SetTimer(hwnd, IDT_TIMER1, 5000, (TIMERPROC)NULL);
        // Load the bitmap resource.
        HBITMAP hbmp = (HBITMAP)LoadImage(NULL, L"a3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

        hdc = GetDC(hwnd);
        hdcCompat = CreateCompatibleDC(hdc);
        SelectObject(hdcCompat, hbmp);
        // Create a brush of the same color as the background
        // of the client area. The brush is used later to erase
        // the old bitmap before copying the bitmap into the
        // target rectangle.

        crBkgnd = GetBkColor(hdc);
        hbrBkgnd = CreateSolidBrush(crBkgnd);
        ReleaseDC(hwnd, hdc);

        // Create a dotted pen. The pen is used to draw the
        // bitmap rectangle as the user drags it.

        hpenDot = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
        transPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
        // Set the initial rectangle for the bitmap. Note that
        // this application supports only a 32- by 32-pixel
        // bitmap. The rectangle is slightly larger than the
        // bitmap.

        GetClientRect(hwnd, &rcClient);
        ptClientUL.x = rcClient.left;
        ptClientUL.y = rcClient.top;
        ptClientLR.x = rcClient.right;
        ptClientLR.y = rcClient.bottom;

        SetRect(&rcBmp, 1, 1, 130, 130);
        return 0;
    }

    case WM_PAINT:

        // Draw the bitmap rectangle and copy the bitmap into
        // it. The 32-pixel by 32-pixel bitmap is centered
        // in the rectangle by adding 1 to the left and top
        // coordinates of the bitmap rectangle, and subtracting 2
        // from the right and bottom coordinates.

        BeginPaint(hwnd, &ps);
        TransparentBlt(ps.hdc, rcBmp.left + 1, rcBmp.top + 1,
            (rcBmp.right - rcBmp.left) - 2,
            (rcBmp.bottom - rcBmp.top) - 2, hdcCompat,
            0, 0, 128, 128, GetSysColor(COLOR_WINDOW));
        EndPaint(hwnd, &ps);
        break;

    case WM_ERASEBKGND:
        hdc = (HDC)wParam;
        SetMapMode(hdc, MM_ANISOTROPIC);
        SetWindowExtEx(hdc, 100, 100, NULL);
        SetViewportExtEx(hdc, rcClient.right, rcClient.bottom, NULL);
        FillRect(hdc, &rcClient, hbrWhite);
        return 1L;

    case WM_MOVE:
    case WM_SIZE:

        // Convert the client coordinates of the client-area
        // rectangle to screen coordinates and save them in a
        // rectangle. The rectangle is passed to the ClipCursor
        // function during WM_LBUTTONDOWN processing.

        GetClientRect(hwnd, &rcClient);
        ptClientUL.x = rcClient.left;
        ptClientUL.y = rcClient.top;
        ptClientLR.x = rcClient.right;
        ptClientLR.y = rcClient.bottom;
        ClientToScreen(hwnd, &ptClientUL);
        ClientToScreen(hwnd, &ptClientLR);
        SetRect(&rcClient, ptClientUL.x, ptClientUL.y,
            ptClientLR.x, ptClientLR.y);
        return 0;

    case WM_LBUTTONDOWN:

        // Restrict the mouse cursor to the client area. This
        // ensures that the window receives a matching
        // WM_LBUTTONUP message.

        ClipCursor(&rcClient);
        // Save the coordinates of the mouse cursor.

        pt.x = (LONG)LOWORD(lParam);
        pt.y = (LONG)HIWORD(lParam);

        // If the user has clicked the bitmap rectangle, redraw
        // it using the dotted pen. Set the fDragRect flag to
        // indicate that the user is about to drag the rectangle.

        if (PtInRect(&rcBmp, pt))
        {
            hdc = GetDC(hwnd);
            SelectObject(hdc, transPen);
            Rectangle(hdc, rcBmp.left, rcBmp.top, rcBmp.right,
                rcBmp.bottom);
            fDragRect = TRUE;
            ReleaseDC(hwnd, hdc);
        }

        return 0;

    case WM_MOUSEMOVE:

        if (bouncingLaunched)
        {
            cancellation_token = true;
            t.join();
            bouncingLaunched = false;
        }

        SetTimer(hwnd, IDT_TIMER1, 5000, (TIMERPROC)NULL);
        // Draw a target rectangle or drag the bitmap rectangle,
        // depending on the status of the fDragRect flag.

        if ((wParam && MK_LBUTTON)
            && fDragRect)
        {

            // Set the mix mode so that the pen color is the
            // inverse of the background color.
            int offsetX = LOWORD(lParam) - pt.x;
            int offsetY = HIWORD(lParam) - pt.y;
            if (((sgn(offsetY) < 0) && (rcBmp.top + offsetY < 0))
                || ((sgn(offsetY) > 0) && (rcBmp.bottom + ptClientUL.y + offsetY > ptClientLR.y)))
            {
                return 0;
            }

            if (((sgn(offsetX) < 0) && (rcBmp.left + offsetX < 0))
                || ((sgn(offsetX) > 0) && (rcBmp.right + ptClientUL.x + offsetX > ptClientLR.x)))
            {
                return 0;
            }

            hdc = GetDC(hwnd);
            SetROP2(hdc, R2_WHITE);

            // Select the dotted pen into the DC and erase
            // the previous bitmap rectangle by drawing
            // another rectangle on top of it.

            SelectObject(hdc, transPen);
            Rectangle(hdc, rcBmp.left, rcBmp.top,
                rcBmp.right, rcBmp.bottom);

            // Set the new coordinates of the bitmap rectangle,
            // then redraw it.

            OffsetRect(&rcBmp, LOWORD(lParam) - pt.x,
                HIWORD(lParam) - pt.y);
            TransparentBlt(hdc, rcBmp.left + 1, rcBmp.top + 1,
                (rcBmp.right - rcBmp.left) - 2,
                (rcBmp.bottom - rcBmp.top) - 2, hdcCompat,
                0, 0, 128, 128, GetSysColor(COLOR_WINDOW));
            //SwapBuffers(hdc);
            ReleaseDC(hwnd, hdc);

            // Save the coordinates of the mouse cursor.

            pt.x = (LONG)LOWORD(lParam);
            pt.y = (LONG)HIWORD(lParam);
        }
        return 0;

    case WM_LBUTTONUP:

        if (fDragRect)
        {

            // Draw the bitmap rectangle, copy the bitmap into
            // it, and reset the fDragRect flag.

            hdc = GetDC(hwnd);
            TransparentBlt(hdc, rcBmp.left + 1, rcBmp.top + 1,
                (rcBmp.right - rcBmp.left) - 2,
                (rcBmp.bottom - rcBmp.top) - 2, hdcCompat,
                0, 0, 128, 128, GetSysColor(COLOR_WINDOW));

            ReleaseDC(hwnd, hdc);
            fDragRect = FALSE;
        }

        // Release the mouse cursor.

        ClipCursor((LPRECT)NULL);
        return 0;

    case WM_MOUSEWHEEL: {
        auto delta = (short)HIWORD(wParam);
        auto key = (short)LOWORD(wParam);
        int direct = sgn(delta);
        int offset;
        if (key == MK_SHIFT) {
            if ((direct < 0) && (rcBmp.left != 0))
            {
                offset = (rcBmp.left > 5) ? 5 : rcBmp.left;
            }
            else if ((direct > 0) && (rcBmp.right + ptClientUL.x != ptClientLR.x))
            {
                int rightOffset = ptClientLR.x - (rcBmp.right + ptClientUL.x);
                offset = (ptClientLR.x - (rcBmp.right + ptClientUL.x) > 5) ? 5 : rightOffset;
            }
            else
            {
                return 0;
            }

            OffsetRect(&rcBmp, direct * offset, 0);
        }
        else {
            if ((direct < 0) && (rcBmp.top != 0))
            {
                offset = (rcBmp.top > 5) ? 5 : rcBmp.top;
            }
            else if ((direct > 0) && (rcBmp.bottom + ptClientUL.y != ptClientLR.y))
            {
                int bottomOffset = ptClientLR.y - (rcBmp.bottom + ptClientUL.y);
                offset = (bottomOffset > 5) ? 5 : bottomOffset;
            }
            else
            {
                return 0;
            }

            OffsetRect(&rcBmp, 0, direct * offset);
        }

        hdc = GetDC(hwnd);

        SetROP2(hdc, R2_WHITE);
        Rectangle(hdc, rcBmp.left, rcBmp.top, rcBmp.right, rcBmp.bottom);
        TransparentBlt(hdc, rcBmp.left + 1, rcBmp.top + 1,
            (rcBmp.right - rcBmp.left) - 2,
            (rcBmp.bottom - rcBmp.top) - 2, hdcCompat,
            0, 0, 128, 128, GetSysColor(COLOR_WINDOW));
        // SwapBuffers(hdc);

        ReleaseDC(hwnd, hdc);
        return 0;
    }

    case WM_TIMER:
    {
        if (wParam == IDT_TIMER1)
        {
            if (!bouncingLaunched)
            {
                cancellation_token = false;
                bouncingLaunched = true;
                RECT r;
                r.left = ptClientUL.x;
                r.top = ptClientUL.y;
                r.right = ptClientLR.x;
                r.bottom = ptClientLR.y;
                t = std::thread(MovePicture, hwnd, &rcBmp, r, hdcCompat, &cancellation_token);
            }
        }

        return 0;
    }

    case WM_DESTROY:

        // Destroy the background brush, compatible bitmap,
        // and the bitmap.
        if (bouncingLaunched)
        {
            cancellation_token = true;
            t.join();
            bouncingLaunched = false;
        }

        delete bmpHand;
        KillTimer(hwnd, IDT_TIMER1);
        DeleteObject(hbrBkgnd);
        DeleteDC(hdcCompat);
        DeleteObject(hbmp);
        PostQuitMessage(0);
        break;

    default:
        doingNothing = true;
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return (LRESULT)NULL;
}

void MovePicture(HWND hwnd, RECT* rcBmp, RECT client, HDC hdcCompat, std::atomic_bool* cancellation_token)
{
    POINT ptClientUL;
    POINT ptClientLR;
    ptClientUL.x = client.left;
    ptClientUL.y = client.top;
    ptClientLR.x = client.right;
    ptClientLR.y = client.bottom;
    double angle = (rand() % 360 + 1) * 3.14 / 180;
    const int step = 5;
    HPEN transPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
    HDC hdc = GetDC(hwnd);
    int offsetX = cos(angle) * step;
    int offsetY = sin(angle) * step;

    while (!*cancellation_token) {
        int directX = sgn(offsetX);
        int directY = sgn(offsetY);

        if (((directX < 0) && ((*rcBmp).left == 0)) || ((directX > 0) && ((*rcBmp).right + ptClientUL.x == ptClientLR.x)))
        {
            offsetX = -offsetX;
        }
        if (((directY < 0) && ((*rcBmp).top == 0)) || ((directY > 0) && ((*rcBmp).bottom + ptClientUL.y == ptClientLR.y)))
        {
            offsetY = -offsetY;
        }

        if ((directX < 0) && ((*rcBmp).left + offsetX < 0))
        {
            offsetX = directX * (*rcBmp).left;
        }
        else if ((directX > 0) && ((*rcBmp).right + ptClientUL.x + offsetX > ptClientLR.x))
        {
            int rightOffset = ptClientLR.x - ((*rcBmp).right + ptClientUL.x);
            offsetX = rightOffset;
        }
        
        if ((directY < 0) && ((*rcBmp).top + offsetY < 0))
        {
            offsetY = directY * (*rcBmp).top;
        }
        else if ((directY > 0) && ((*rcBmp).bottom + ptClientUL.y + offsetY > ptClientLR.y))
        {
            int bottomOffset = ptClientLR.y - ((*rcBmp).bottom + ptClientUL.y);
            offsetY = bottomOffset;
        }

        OffsetRect(rcBmp, offsetX, offsetY);
        SetROP2(hdc, R2_WHITE);
        Rectangle(hdc, (*rcBmp).left, (*rcBmp).top,
            (*rcBmp).right, (*rcBmp).bottom);
        SelectObject(hdc, transPen);
        /*
        Rectangle(hdc, (*rcBmp).left, (*rcBmp).top,
            (*rcBmp).right, (*rcBmp).bottom);
        
        StretchBlt(hdc, (*rcBmp).left + 1, (*rcBmp).top + 1,
            ((*rcBmp).right - (*rcBmp).left) - 2,
            ((*rcBmp).bottom - (*rcBmp).top) - 2, hdcCompat,
            0, 0, 128, 128, SRCCOPY);
            */
        TransparentBlt(hdc, (*rcBmp).left + 1, (*rcBmp).top + 1,
            ((*rcBmp).right - (*rcBmp).left) - 2,
            ((*rcBmp).bottom - (*rcBmp).top) - 2, hdcCompat,
            0, 0, 128, 128, GetSysColor(COLOR_WINDOW));

        Sleep(50);
    }

    ReleaseDC(hwnd, hdc);
}
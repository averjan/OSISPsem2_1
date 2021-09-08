#pragma comment(lib, "Msimg32.lib")

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <math.h>
#include <thread>
#include "BitmapHandler.h"


const double M_PI = 3.14;

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

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
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;          // paint data for BeginPaint and EndPaint
    static POINT pt;         // x and y coordinates of cursor
    static BOOL fDragRect;   // TRUE if bitmap rect. is dragged
    static UINT_PTR IDT_TIMER2 = 1;
    static UINT_PTR IDT_TIMER1 = 1001;
    static bool bouncingLaunched = false;
    HBRUSH hbrWhite = (HBRUSH)GetStockObject(GRAY_BRUSH);
    static BitmapHandler* bmpHand = new BitmapHandler(hwnd, L"a3.bmp");
    int speed = 5;
    static RECT rcClient;
    switch (uMsg)
    {
        case WM_CREATE: {
            SetTimer(hwnd, IDT_TIMER1, 5000, (TIMERPROC)NULL);

            return 0;
        }

        case WM_TIMER:
        {
            int a;
            if (wParam == IDT_TIMER1)
            {
                if (!bouncingLaunched)
                {
                    bmpHand->GenerateMotionAngle();
                    SetTimer(hwnd, IDT_TIMER2, 50, (TIMERPROC)NULL);
                    bouncingLaunched = true;
                }
            }
            
            else if (wParam == IDT_TIMER2)
            {
                bmpHand->AutoMoveRect();
                RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
            }
            
            return 0;
        }

        case WM_PAINT:
        {
            BeginPaint(hwnd, &ps);
            FillRect(ps.hdc, &ps.rcPaint, hbrWhite);
            bmpHand->Draw();
            EndPaint(hwnd, &ps);
            break;
        }

        case WM_MOVE:
        case WM_SIZE:
        {
            GetClientRect(hwnd, &rcClient);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            ClipCursor(&rcClient);

            pt.x = (LONG)LOWORD(lParam);
            pt.y = (LONG)HIWORD(lParam);
            if (bmpHand->PtInBmp(pt))
            {
                fDragRect = TRUE;
            }

            return 0;
        }

        case WM_MOUSEMOVE: {

            if (bouncingLaunched)
            {
                KillTimer(hwnd, IDT_TIMER2);
                bouncingLaunched = false;
            }

            SetTimer(hwnd, IDT_TIMER1, 5000, (TIMERPROC)NULL);

            if ((wParam && MK_LBUTTON)
                && fDragRect)
            {
                int offsetX = LOWORD(lParam) - pt.x;
                int offsetY = HIWORD(lParam) - pt.y;
                bmpHand->MoveRect(offsetX, offsetY);
                pt.x = (LONG)LOWORD(lParam);
                pt.y = (LONG)HIWORD(lParam);
                RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
            }
            return 0;
        }

        case WM_LBUTTONUP: {

            if (fDragRect)
            {
                fDragRect = FALSE;
            }

            ClipCursor((LPRECT)NULL);
            return 0;
        }

        case WM_MOUSEWHEEL: {
            if (bouncingLaunched)
            {
                KillTimer(hwnd, IDT_TIMER2);
                bouncingLaunched = false;
            }

            auto delta = (short)HIWORD(wParam);
            auto key = (short)LOWORD(wParam);
            int direct = sgn(delta);
            if (key == MK_SHIFT) {
                bmpHand->MoveRect(direct * speed, 0);
                RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
            }
            else {
                bmpHand->MoveRect(0, direct * speed);
                RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
            }
            return 0;
        }

        case WM_DESTROY:
        {
            if (bouncingLaunched)
            {
                KillTimer(hwnd, IDT_TIMER2);
            }

            delete bmpHand;
            KillTimer(hwnd, IDT_TIMER1);
            PostQuitMessage(0);
            break;
        }
        default:
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    return (LRESULT)NULL;
}
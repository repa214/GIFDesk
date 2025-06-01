#include "window_proc.h"
#include "opengl_proc.h"
#include "settings.h"
#include "loadgif.h"
#include "language.h"
#include "resource.h"

HWND hwnd;
HDC hdc;
HGLRC hRC;
WNDCLASSEX wcex;
MSG msg;
PAINTSTRUCT ps;
RECT rect;
HINSTANCE hInstance;
int nCmdShow;
POINT p;
HMENU hMenu;
HMENU hSubMenu;
HMENU hLangMenu;
LONG_PTR exStyle;
FILE *file;
HBITMAP appIcon;
RECT res;

HWND hwnd_2;
WNDCLASSEX wcex_2;
HDC hdc_2;
HGLRC hRC_2;
MSG msg_2;
HWND hTrackbar;
HRGN hRgn;
HWND hButton;
HDC hdc_b;
HFONT hFont;
HWND hEdit;
HWND hButtonUp, hButtonDown;

pthread_t render;

int DESTROY_WINDOW = 0;
int WAITING = 0;
int HOVERED = 0, HOVERED_DOWN = 0, HOVERED_UP = 0;
int pos = 0;
float trackbar_size = 0;

/**
        DRAG AND DROP
**/

void DropFiles(HDROP hDrop) {
    char filename[MAX_PATH];
    DragQueryFile(hDrop, 0, filename, MAX_PATH);

    filetype = CheckFile((char const *)filename);
    if (filetype) {
        WriteSettings(filename, size, TASKBAR, TOPMOST, LANGGIF);

        hwnd = FindWindow(NULL, APP_NAME);
        GetWindowRect(hwnd, &rect);
        DisableOpenGL(hwnd, hdc, hRC);
        DestroyWindow(hwnd);

        fc = 0; k = 0;

        hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST,
                              "Window",
                              APP_NAME,
                              WS_POPUP | WS_VISIBLE,
                              rect.left,
                              rect.top,
                              (width * size < 10.0) ? 10.0 : width * size,
                              (height * size < 10.0) ? 10.0 : height * size,
                              NULL,
                              NULL,
                              hInstance,
                              NULL);

        exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

        if (!TASKBAR) exStyle |= WS_EX_TOOLWINDOW;
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

        SetLayeredWindowAttributes(hwnd, 0x0, 0, LWA_COLORKEY);

        ShowWindow(hwnd, SW_SHOWDEFAULT);
        DragAcceptFiles(hwnd, TRUE);

        EnableOpenGL(hwnd, &hdc, &hRC);

        LoadFile((char const *)filename, GIF_FORMAT);

        SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);

        if (width * size > res.right - res.left) size = ((float)res.right - (float)res.left) / (float)width;
        if (height * size > res.bottom - res.top) size = ((float)res.bottom - (float)res.top) / (float)height;

        SetWindowPos(hwnd,
                     (TOPMOST) ? HWND_TOPMOST : HWND_NOTOPMOST,
                     0,
                     0,
                     (width * size < 10.0) ? 10.0 : width * size,
                     (height * size < 10.0) ? 10.0 : height * size,
                     SWP_NOMOVE);

        if (TOPMOST) SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);
        else SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);
    }
    else MessageBox(NULL, lang.notGIF[LANGGIF], APP_NAME, MB_ICONEXCLAMATION | MB_TOPMOST);


    DragFinish(hDrop);
}

/**
        BITMAP ICON
**/

void GetApplicationIcon() {
    HBRUSH hBrushBk = CreateSolidBrush(RGB(240, 240, 240));
    HBRUSH hBrushHovered = CreateSolidBrush(RGB(210, 210, 210));

    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(MENU_ICON));
    HDC hdc = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hdc);
    appIcon = CreateCompatibleBitmap(hdc, 16, 16);
    SelectObject(hMemDC, appIcon);

    DrawIconEx(hMemDC, 0, 0, hIcon, 16, 16, 0, NULL, DI_NORMAL);

    DeleteDC(hMemDC);
    ReleaseDC(NULL, hdc);
    DestroyIcon(hIcon);

    DeleteObject(hBrushBk);
    DeleteObject(hBrushHovered);
}

/**
        MAIN WINDOW
**/

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DROPFILES: {
            DropFiles((HDROP)wParam);
        }   break;

        case WM_SETCURSOR: {
            if (LOWORD(lParam) == HTCLIENT) SetCursor(LoadCursor(NULL, IDC_SIZEALL));
        }   break;

        case WM_QUIT: {
            DESTROY_WINDOW = 1;
        }   break;

        case WM_CLOSE: {
            DESTROY_WINDOW = 1;
        }   break;

        case WM_SIZE: {
            InvalidateRect(hwnd, NULL, FALSE);
        }   break;

        case WM_LBUTTONDOWN: {
            wglMakeCurrent(NULL, NULL);
            if (!IsWindow(hwnd_2)) {
                pthread_create(&render, NULL, &RenderThread, NULL);

                SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

                DESTROY_WINDOW = 1; pthread_join(render, NULL); DESTROY_WINDOW = 0;
            }
            else {
                SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            }
        }   break;

        case WM_RBUTTONDOWN: {
            if (!IsWindow(hwnd_2) && !WAITING) {
                SetCursor(LoadCursor(NULL, IDC_ARROW));

                wglMakeCurrent(NULL, NULL);
                pthread_create(&render, NULL, RenderThread, NULL);

                hMenu = CreatePopupMenu();
                hSubMenu = CreatePopupMenu();
                hLangMenu = CreatePopupMenu();
                GetCursorPos(&p);
                sprintf(str_size, lang.scaleGIF[LANGGIF], size * 100);

                AppendMenu(hSubMenu, MF_STRING, 5, lang.tlcGIF[LANGGIF]);
                AppendMenu(hSubMenu, MF_STRING, 6, lang.trcGIF[LANGGIF]);
                AppendMenu(hSubMenu, MF_STRING, 7, lang.blcGIF[LANGGIF]);
                AppendMenu(hSubMenu, MF_STRING, 8, lang.brcGIF[LANGGIF]);

                AppendMenu(hLangMenu, MF_STRING, 9, "English");
                AppendMenu(hLangMenu, MF_STRING, 10, "Русский");

                AppendMenu(hMenu, MF_STRING, 1, "");

                MENUITEMINFO mii;
                ZeroMemory(&mii, sizeof(MENUITEMINFO));
                mii.cbSize = sizeof(MENUITEMINFO);
                mii.fMask = MIIM_BITMAP | MIIM_ID | MIIM_STRING;
                mii.dwTypeData = APP_NAME;
                mii.hbmpItem = appIcon;

                SetMenuItemInfo(hMenu, 1, FALSE, &mii);

                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hMenu, MF_STRING, 1, lang.changeGIF[LANGGIF]);
                AppendMenu(hMenu, MF_STRING, 2, str_size);
                AppendMenu(hMenu, MF_STRING | (TASKBAR) ? MF_CHECKED : 0, 3, lang.showiconGIF[LANGGIF]);
                AppendMenu(hMenu, MF_STRING | (TOPMOST) ? MF_CHECKED : 0, 4, lang.topmostGIF[LANGGIF]);
                AppendMenu(hMenu, MF_POPUP | MF_STRING, (UINT_PTR)hSubMenu, lang.movewindowGIF[LANGGIF]);
                AppendMenu(hMenu, MF_POPUP | MF_STRING, (UINT_PTR)hLangMenu, lang.langGIF[LANGGIF]);
                AppendMenu(hMenu, MF_STRING, 11, lang.exitGIF[LANGGIF]);

                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);

                DestroyMenu(hMenu);

                DESTROY_WINDOW = 1; pthread_join(render, NULL); DESTROY_WINDOW = 0;
            }
        }   break;

        case WM_COMMAND: {
            switch (wParam) {
                /** Open file... **/
                case 1: {
                    OPENFILENAME ofn;

                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = NULL;
                    ofn.lpstrFile = filename;
                    ofn.lpstrFile[0] = '\0';
                    ofn.lpstrFilter = OFNfilter;
                    ofn.nMaxFile = sizeof(filename);
                    ofn.lpstrTitle = lang.selectGIF[LANGGIF];
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                    if (TOPMOST) SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);
                    WAITING = 1;

                    DragAcceptFiles(hwnd, FALSE);
                    if (GetOpenFileName(&ofn)) {
                        filetype = CheckFile((char const *)filename);
                        if (filetype) {
                            SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
                            WriteSettings(filename, size, TASKBAR, TOPMOST, LANGGIF);

                            hwnd = FindWindow(NULL, APP_NAME);
                            GetWindowRect(hwnd, &rect);
                            DisableOpenGL(hwnd, hdc, hRC);
                            DestroyWindow(hwnd);
                            fc = 0; k = 0;

                            hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST,
                                                  "Window",
                                                  APP_NAME,
                                                  WS_POPUP | WS_VISIBLE,
                                                  rect.left,
                                                  rect.top,
                                                  CollisionWidth(),
                                                  CollisionHeight(),
                                                  NULL,
                                                  NULL,
                                                  hInstance,
                                                  NULL);

                            exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

                            if (!TASKBAR) exStyle |= WS_EX_TOOLWINDOW;
                            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

                            SetLayeredWindowAttributes(hwnd, 0x0, 0, LWA_COLORKEY);

                            ShowWindow(hwnd, SW_SHOWDEFAULT);
                            DragAcceptFiles(hwnd, TRUE);

                            EnableOpenGL(hwnd, &hdc, &hRC);

                            LoadFile((char const *)filename, GIF_FORMAT);

                            SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);

                            if (CollisionWidth() > res.right - res.left) size = ((float)res.right - (float)res.left) / (float)width;
                            if (height * size > res.bottom - res.top) size = ((float)res.bottom - (float)res.top) / (float)height;

                            SetWindowPos(hwnd,
                                         (TOPMOST) ? HWND_TOPMOST : HWND_NOTOPMOST,
                                         0,
                                         0,
                                         CollisionWidth(),
                                         CollisionHeight(),
                                         SWP_NOMOVE);

                            SetCursor(LoadCursor(NULL, IDC_ARROW));
                        }
                        else MessageBox(NULL, lang.notGIF[LANGGIF], APP_NAME, MB_ICONEXCLAMATION | MB_TOPMOST);
                    }
                    else ReadSettings(0);
                    WAITING = 0;
                    ZeroMemory(&ofn, sizeof(ofn));
                }   break;

                /** Scale (%.0f%%) **/
                case 2: {
                    GetCursorPos(&p);
                    int px = 0;

                    if (!RegisterClassEx(&wcex_2));

                    wglMakeCurrent(NULL, NULL);
                    pthread_create(&render, NULL, &RenderThread, NULL);

                    WAITING = 1;

                    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

                    DESTROY_WINDOW = 1; pthread_join(render, NULL); DESTROY_WINDOW = 0;

                    texCoord[1] = 2 / ((size > 2) ? 2 : size);
                    texCoord[2] = 2 / ((size > 2) ? 2 : size);
                    texCoord[3] = 2 / ((size > 2) ? 2 : size);
                    texCoord[4] = 2 / ((size > 2) ? 2 : size);
                    sprintf(str_size, "%.0f", size * 100);

                    trackbar_size = size;
                    size = 2;

                    SetWindowPos(hwnd,
                         NULL,
                         0,
                         0,
                         CollisionWidth(),
                         CollisionHeight(),
                         SWP_NOMOVE);

                    pthread_create(&render, NULL, &RenderThread, NULL);

                    DragAcceptFiles(hwnd, FALSE);

                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);

                    if (p.x + 204 > res.right - res.left) { px = res.right - res.left - 204; }
                    else { px = p.x; }

                    hwnd_2 = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                                            "Window_2",
                                            APP_NAME,
                                            WS_POPUP,
                                            px,
                                            p.y - 20,
                                            204,
                                            73,
                                            hwnd,
                                            NULL,
                                            hInstance,
                                            NULL);

                    hTrackbar = CreateWindowEx(0,
                                               TRACKBAR_CLASS,
                                               NULL,
                                               WS_TABSTOP | WS_CHILD | WS_VISIBLE | TBS_NOTICKS | TBS_BOTH,
                                               -1,
                                               5,
                                               206,
                                               24,
                                               hwnd_2,
                                               NULL,
                                               NULL,
                                               NULL);

                    hButton = CreateWindowEx(0,
                                             "BUTTON",
                                             "Button",
                                             WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                                             4,
                                             42,
                                             126,
                                             24,
                                             hwnd_2,
                                             (HMENU)1,
                                             NULL,
                                             NULL);

                    hEdit = CreateWindowEx(0, "EDIT", str_size, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                                    134, 43, 40, 22, hwnd_2, NULL, NULL, NULL);
                    hButtonUp = CreateWindowEx(0, "BUTTON", "+", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW,
                                 174, 43, 25, 11, hwnd_2, (HMENU)2, NULL, NULL);
                    hButtonDown = CreateWindowEx(0, "BUTTON", "-", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW | BS_NOTIFY,
                                 174, 54, 25, 11, hwnd_2, (HMENU)3, NULL, NULL);

                    hFont = CreateFont(16,
                                       0,
                                       0,
                                       0,
                                       FW_NORMAL,
                                       FALSE,
                                       FALSE,
                                       FALSE,
                                       DEFAULT_CHARSET,
                                       OUT_DEFAULT_PRECIS,
                                       CLIP_DEFAULT_PRECIS,
                                       DEFAULT_QUALITY,
                                       DEFAULT_QUALITY,
                                       "Segoe UI");

                    SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);

                    hFont = CreateFont(17,
                                       0,
                                       0,
                                       0,
                                       FW_NORMAL,
                                       FALSE,
                                       FALSE,
                                       FALSE,
                                       DEFAULT_CHARSET,
                                       OUT_DEFAULT_PRECIS,
                                       CLIP_DEFAULT_PRECIS,
                                       DEFAULT_QUALITY,
                                       DEFAULT_QUALITY,
                                       "Segoe UI");

                    SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

                    hFont = CreateFont(16,
                                       0,
                                       0,
                                       0,
                                       FW_NORMAL,
                                       FALSE,
                                       FALSE,
                                       FALSE,
                                       DEFAULT_CHARSET,
                                       OUT_DEFAULT_PRECIS,
                                       CLIP_DEFAULT_PRECIS,
                                       DEFAULT_QUALITY,
                                       DEFAULT_QUALITY,
                                       "Segoe UI Symbol");

                    SendMessage(hButtonUp, WM_SETFONT, (WPARAM)hFont, TRUE);
                    SendMessage(hButtonDown, WM_SETFONT, (WPARAM)hFont, TRUE);

                    SendMessage(hTrackbar, TBM_SETRANGE, TRUE, MAKELONG(1, 200));
                    SendMessage(hTrackbar, TBM_SETPOS, TRUE, trackbar_size * 100);
                    SendMessage(hTrackbar, TBM_SETTIC, 200, 0);
                    pos = trackbar_size;

                    hRgn = CreateRoundRectRgn(0, 0, 204, 73, 5, 5);
                    SetWindowRgn(hwnd_2, hRgn, TRUE);

                    SetLayeredWindowAttributes(hwnd_2, 0x0, 0, LWA_COLORKEY);
                    ShowWindow(hwnd_2, SW_SHOWDEFAULT);

                    // Для троллинга
                    // SystemParametersInfo(SPI_SETDOUBLECLICKTIME, 15, NULL, 0);

                    while (GetMessage(&msg_2, NULL, 0, 0)) {
                        TranslateMessage(&msg_2);
                        DispatchMessage(&msg_2);
                    }

                    SystemParametersInfo(SPI_SETDOUBLECLICKTIME, 500, NULL, 0);

                    DESTROY_WINDOW = 1; pthread_join(render, NULL); DESTROY_WINDOW = 0;
                    size = trackbar_size;
                    DestroyWindow(hwnd_2);
                    WriteSettings(filename, size, TASKBAR, TOPMOST, LANGGIF);

                    SetWindowPos(hwnd,
                                 (TOPMOST) ? HWND_TOPMOST : HWND_NOTOPMOST,
                                 0,
                                 0,
                                 CollisionWidth(),
                                 CollisionHeight(),
                                 SWP_NOMOVE);

                    texCoord[1] = 1;
                    texCoord[2] = 1;
                    texCoord[3] = 1;
                    texCoord[4] = 1;

                    DragAcceptFiles(hwnd, TRUE);
                    WAITING = 0;
                }   break;

                /** Show taskbar icon **/
                case 3: {
                    exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

                    if (TASKBAR) {
                        exStyle |= WS_EX_TOOLWINDOW;
                        TASKBAR = 0;
                    }
                    else {
                        exStyle &= ~WS_EX_TOOLWINDOW;
                        TASKBAR = 1;
                    }

                    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
                    WriteSettings(filename, size, TASKBAR, TOPMOST, LANGGIF);
                    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW);
                }   break;

                /** Always on top **/
                case 4: {
                    if (TOPMOST) {
                        TOPMOST = 0; SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);
                    }
                    else {
                        TOPMOST = 1; SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);
                    }
                    WriteSettings(filename, size, TASKBAR, TOPMOST, LANGGIF);
                }   break;

                /** Top left corner **/
                case 5: {
                    SetWindowPos(hwnd,
                         NULL,
                         0,
                         0,
                         0,
                         0,
                         SWP_NOSIZE);
                }   break;

                /** Top right corner **/
                case 6: {
                    SetWindowPos(hwnd,
                         NULL,
                         GetSystemMetrics(SM_CXSCREEN) - (width) * size - 1,
                         0,
                         0,
                         0,
                         SWP_NOSIZE);
                }   break;

                /** Bottom left corner **/
                case 7: {
                    RECT res;
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);

                    SetWindowPos(hwnd,
                         NULL,
                         0,
                         res.bottom - res.top - (height) * size,
                         0,
                         0,
                         SWP_NOSIZE);
                }   break;

                /** Bottom right corner **/
                case 8: {
                    RECT res;
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);

                    SetWindowPos(hwnd,
                         NULL,
                         res.right - res.left - (width) * size,
                         res.bottom - res.top - (height) * size,
                         0,
                         0,
                         SWP_NOSIZE);
                }   break;

                /** English **/
                case 9: {
                    LANGGIF = 0;
                    WriteSettings(filename, size, TASKBAR, TOPMOST, LANGGIF);
                }   break;

                /** Русский **/
                case 10: {
                    LANGGIF = 1;
                    WriteSettings(filename, size, TASKBAR, TOPMOST, LANGGIF);
                }   break;

                /** Exit **/
                case 11: DESTROY_WINDOW = 1; break;

            }   break;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_ESCAPE) {
                DESTROY_WINDOW = 1;
            }
        }   break;
        default: return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

/**
        SCALE WINDOW
**/

LRESULT CALLBACK WindowProc_2(HWND hwnd_2, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    hwnd = GetParent(hwnd_2);
    switch (uMsg) {
        case WM_QUIT: {
            PostQuitMessage(0);
        }   break;

        case WM_CLOSE: {
            PostQuitMessage(0);
        }   break;

        case WM_LBUTTONDOWN: {
            SetFocus(hwnd_2);
            SendMessage(hwnd_2, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }   break;

        case WM_SETCURSOR: {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            GetCursorPos(&p);

            GetWindowRect(hButton, &rect);
            if (PtInRect(&rect, p) && !HOVERED) { HOVERED = 1; InvalidateRect(hButton, NULL, TRUE); }
            else if (!PtInRect(&rect, p) && HOVERED) { HOVERED = 0; InvalidateRect(hButton, NULL, TRUE); }

            GetWindowRect(hButtonUp, &rect);
            if (PtInRect(&rect, p) && !HOVERED_UP) { HOVERED_UP = 1; InvalidateRect(hButtonUp, NULL, TRUE); }
            else if (!PtInRect(&rect, p) && HOVERED_UP) { HOVERED_UP = 0; InvalidateRect(hButtonUp, NULL, TRUE); }

            GetWindowRect(hButtonDown, &rect);
            if (PtInRect(&rect, p) && !HOVERED_DOWN) { HOVERED_DOWN = 1; InvalidateRect(hButtonDown, NULL, TRUE); }
            else if (!PtInRect(&rect, p) && HOVERED_DOWN) { HOVERED_DOWN = 0; InvalidateRect(hButtonDown, NULL, TRUE); }
        }   break;

        case WM_HSCROLL: {
            SetFocus(hwnd_2);
            pos = SendMessage(hTrackbar, TBM_GETPOS, 0, 0);

            texCoord[1] = 2 / ((float)pos / 100);
            texCoord[2] = 2 / ((float)pos / 100);
            texCoord[3] = 2 / ((float)pos / 100);
            texCoord[4] = 2 / ((float)pos / 100);

            trackbar_size = (float)pos / 100;

            sprintf(str_size, "%.0f", trackbar_size * 100);
            SetWindowText(hEdit, str_size);

            if (!DRAWING) ShowFrame(k);
        }   break;

        case WM_PAINT: {
            hdc_2 = BeginPaint(hwnd_2, &ps);

            HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));
            FillRect(hdc_2, &ps.rcPaint, hBrush);

            RECT rect = {4, 37, 198, 38};

            hBrush = CreateSolidBrush(RGB(210, 210, 210));
            FillRect(hdc_2, &rect, hBrush);

            DeleteObject(hBrush);
            EndPaint(hwnd_2, &ps);
        }   break;

        case WM_DRAWITEM: {
            SetCursor(LoadCursor(NULL, IDC_ARROW));

            LPDRAWITEMSTRUCT pDrawItem = (LPDRAWITEMSTRUCT)lParam;

            if (pDrawItem->hwndItem == hButton) {
                HDC hdc_b = pDrawItem->hDC;
                RECT rect = pDrawItem->rcItem;

                SetBkMode(hdc_b, TRANSPARENT);
                HBRUSH hBrush = CreateSolidBrush(
                    (HOVERED) ? RGB(225, 225, 225) : RGB(240, 240, 240)
                );
                FillRect(hdc_b, &rect, hBrush);
                DeleteObject(hBrush);
                DrawText(hdc_b, lang.saveGIF[LANGGIF], -1, &rect, DT_VCENTER | DT_SINGLELINE);
            }

            if (pDrawItem->hwndItem == hButtonUp) {
                HDC hdc_bd = pDrawItem->hDC;
                RECT rect = pDrawItem->rcItem;

                SetBkMode(hdc_bd, TRANSPARENT);
                HBRUSH hBrush = CreateSolidBrush(
                    (HOVERED_UP) ? RGB(200, 200, 200) : RGB(215, 215, 215)
                );
                FillRect(hdc_bd, &rect, hBrush);
                hBrush = CreateSolidBrush(RGB(0, 0, 0));
                SelectObject(hdc_bd, hBrush);

                POINT triangle[3] = { {13, 3}, {18, 8}, {8, 8} };
                Polygon(hdc_bd, triangle, 3);
                DeleteObject(hBrush);
            }

            if (pDrawItem->hwndItem == hButtonDown) {
                HDC hdc_bd = pDrawItem->hDC;
                RECT rect = pDrawItem->rcItem;

                SetBkMode(hdc_bd, TRANSPARENT);
                HBRUSH hBrush = CreateSolidBrush(
                    (HOVERED_DOWN) ? RGB(200, 200, 200) : RGB(215, 215, 215)
                );
                FillRect(hdc_bd, &rect, hBrush);
                hBrush = CreateSolidBrush(RGB(0, 0, 0));
                SelectObject(hdc_bd, hBrush);

                POINT triangle[3] = { {8, 3}, {18, 3}, {13, 8} };
                Polygon(hdc_bd, triangle, 3);
                DeleteObject(hBrush);
            }
        }   break;

        case WM_COMMAND: {
            if ((HWND)lParam == hEdit && HIWORD(wParam) == EN_CHANGE) {
                GetWindowText(hEdit, str_size, sizeof(str_size));
                trackbar_size = atof(str_size) / 100;

                if (trackbar_size > 10) {
                    trackbar_size = 10;
                    SetWindowText(hEdit, "1000");
                    MessageBeep(MB_ICONWARNING);
                }

                if (trackbar_size <= 0) {
                    trackbar_size = 0.01;
                    SetWindowText(hEdit, "1");
                    MessageBeep(MB_ICONWARNING);
                }

                if (trackbar_size < 2) {
                    texCoord[1] = 2 / ((float)trackbar_size);
                    texCoord[2] = 2 / ((float)trackbar_size);
                    texCoord[3] = 2 / ((float)trackbar_size);
                    texCoord[4] = 2 / ((float)trackbar_size);

                    SendMessage(hTrackbar, TBM_SETPOS, TRUE, trackbar_size * 100);
                    if (!DRAWING) ShowFrame(k);
                }
                else {
                    texCoord[1] = 1;
                    texCoord[2] = 1;
                    texCoord[3] = 1;
                    texCoord[4] = 1;

                    SendMessage(hTrackbar, TBM_SETPOS, TRUE, 200);
                    if (!DRAWING) ShowFrame(k);
                }
            }

            if (LOWORD(wParam) == 2 && trackbar_size < 10) {
                trackbar_size += 0.01;

                sprintf(str_size, "%.0f", trackbar_size * 100);
                SetWindowText(hEdit, str_size);
                if (trackbar_size < 2) {

                    texCoord[1] = 2 / ((float)trackbar_size);
                    texCoord[2] = 2 / ((float)trackbar_size);
                    texCoord[3] = 2 / ((float)trackbar_size);
                    texCoord[4] = 2 / ((float)trackbar_size);

                    SendMessage(hTrackbar, TBM_SETPOS, TRUE, trackbar_size * 100);
                    if (!DRAWING) ShowFrame(k);
                }
            }

            if (LOWORD(wParam) == 3 && trackbar_size > 0.01) {
                trackbar_size -= 0.01;

                texCoord[1] = 2 / ((float)trackbar_size);
                texCoord[2] = 2 / ((float)trackbar_size);
                texCoord[3] = 2 / ((float)trackbar_size);
                texCoord[4] = 2 / ((float)trackbar_size);

                sprintf(str_size, "%.0f", trackbar_size * 100);
                SetWindowText(hEdit, str_size);

                SendMessage(hTrackbar, TBM_SETPOS, TRUE, trackbar_size * 100);
                if (!DRAWING) ShowFrame(k);
            }

            switch (wParam) {
                case 1: {
                    SendMessage(hwnd_2, WM_CLOSE, 0, 0);
                }   break;
            }   break;
        }   break;

        case WM_KEYDOWN: {
            if (wParam == VK_ESCAPE || wParam == VK_RETURN) {
                PostQuitMessage(0);
            }
            if (wParam == VK_UP || wParam == VK_NUMPAD8) {
                SendMessage(hwnd_2, WM_COMMAND, 2, 0);
            }
            if (wParam == VK_DOWN || wParam == VK_NUMPAD2) {
                SendMessage(hwnd_2, WM_COMMAND, 3, 0);
            }
        }   break;

        default: return DefWindowProc(hwnd_2, uMsg, wParam, lParam);
    }
    return 0;
}


/**
        WINDOW SETTINGS
**/

void WcexInit(WNDCLASSEX *wcex, const char *lpszClassName, WNDPROC Proc) {
    (*wcex).cbSize =           sizeof(WNDCLASSEX);
    (*wcex).style =            CS_OWNDC;
    (*wcex).lpfnWndProc =      Proc;
    (*wcex).cbClsExtra =       0;
    (*wcex).cbWndExtra =       0;
    (*wcex).hInstance =        hInstance;
    (*wcex).hIcon =            (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 64, 64, LR_DEFAULTCOLOR);
    (*wcex).hCursor =          LoadCursor(NULL, IDC_ARROW);
    (*wcex).hbrBackground =    (HBRUSH)GetStockObject(BLACK_BRUSH);
    (*wcex).lpszMenuName =     NULL;
    (*wcex).lpszClassName =    lpszClassName;
    (*wcex).hIconSm =          (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 64, 64, LR_DEFAULTCOLOR);
}

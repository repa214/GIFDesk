#include "window.h"

/** Windows Processing **/

int WindowInit(Window* window, const char* lpszClassName, WNDPROC Proc)
{
    WcexInit(&window->wcex, lpszClassName, (WNDPROC)Proc, window->hinstance);
    if (!RegisterClassEx(&window->wcex)) return 1;
    window->isactive = 0;

    return 0;
}

int LoadWindow(Window* window, Settings* st, Window* parent,
               const char* classname,
               int xoffset, int yoffset, int width, int height,
               int settb, int settm, int setdaf, int setgl)
/**
        settb:  set WS_EX_TOOLWINDOW
        settm:  set HWND_TOPMOST
        setdaf: set DragAcceptFiles
        setgl:  set OPENGL
**/
{
    LONG_PTR style = WS_EX_LAYERED;
    if (settb && st->taskbar)
        style |= WS_EX_APPWINDOW;
    else
        style |= WS_EX_TOOLWINDOW;

    window->hwnd = CreateWindowEx(style,
                                  classname,
                                  APP_NAME,
                                  WS_POPUP | WS_VISIBLE,
                                  xoffset,
                                  yoffset,
                                  width,
                                  height,
                                  (parent) ? parent->hwnd : NULL,
                                  NULL,
                                  window->hinstance,
                                  NULL);
    SetFocus(window->hwnd);

    if (settm) SetWindowPos(window->hwnd, (st->topmost) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);

    SetLayeredWindowAttributes(window->hwnd, 0x0, 0, LWA_COLORKEY);
    if (!setgl) SetLayeredWindowAttributes(window->hwnd, 0x0, 0, LWA_ALPHA);

    ShowWindow(window->hwnd, SW_SHOWDEFAULT); window->isactive = 1;
    DragAcceptFiles(window->hwnd, (setdaf) ? TRUE : FALSE);

    if (setgl) EnableOpenGL(window->hwnd, &window->hdc, &window->hrc);

    return 0;
}

void LoadTrackBar(Trackbar* trackbar, Window* main_window,
                  int xoffset, int yoffset, int width, int height, int tooltip,
                  int vfrom, int vto, int sp, int id)
{
    trackbar->hwnd = CreateWindowEx(0,
                                  TRACKBAR_CLASS,
                                  NULL,
                                  WS_TABSTOP | WS_CHILD | WS_VISIBLE | (tooltip ? TBS_TOOLTIPS : 0),
                                  xoffset,
                                  yoffset,
                                  width,
                                  height,
                                  main_window->hwnd,
                                  (HMENU)id,
                                  NULL,
                                  NULL);

    SendMessage(trackbar->hwnd, TBM_SETRANGE, TRUE, MAKELONG(vfrom, vto));
    SendMessage(trackbar->hwnd, TBM_SETPOS, TRUE, sp);
    SendMessage(trackbar->hwnd, TBM_SETTIC, vto, 0);

}

void LoadButton(Button* button, Window* main_window,
                int xoffset, int yoffset, int width, int height, int rgn,
                const char* text, int menu, const char* font_name)
{
    button->hwnd = CreateWindowEx(0,
                                  "BUTTON",
                                  text,
                                  WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                                  xoffset,
                                  yoffset,
                                  width,
                                  height,
                                  main_window->hwnd,
                                  (HMENU)menu,
                                  NULL,
                                  NULL);

    HFONT font = CreateFont(16,
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
                            font_name);

    SendMessage(button->hwnd, WM_SETFONT, (WPARAM)font, TRUE);

    HRGN hrgn = CreateRoundRectRgn(0, 0, width, height, rgn, rgn);
    SetWindowRgn(button->hwnd, hrgn, TRUE);
}

void ReloadWindow(Window* window, Settings* st, Data* dt)
{
    PostMessage(window->hwnd, WM_USER, 3, 0);
}

void ReleaseWindow(Window* window)
{
    window->isactive = 0;
    DisableOpenGL(window->hwnd, window->hdc, window->hrc);
    DestroyWindow(window->hwnd);
}

void WcexInit(WNDCLASSEX* wcex, const char* lpszClassName, WNDPROC Proc, HINSTANCE hInstance)
{
    (*wcex).cbSize =           sizeof(WNDCLASSEX);
    (*wcex).style =            CS_OWNDC;
    (*wcex).lpfnWndProc =      Proc;
    (*wcex).cbClsExtra =       0;
    (*wcex).cbWndExtra =       0;
    (*wcex).hInstance =        hInstance;
    (*wcex).hIcon =            (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 64, 64, LR_DEFAULTCOLOR);
    (*wcex).hCursor =          LoadCursor(NULL, IDC_SIZEALL);
    (*wcex).hbrBackground =    (HBRUSH)GetStockObject(BLACK_BRUSH);
    (*wcex).lpszMenuName =     NULL;
    (*wcex).lpszClassName =    lpszClassName;
    (*wcex).hIconSm =          (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 64, 64, LR_DEFAULTCOLOR);
}

/** OpenGL Processing **/

void EnableOpenGL(HWND hwnd, HDC* hdc, HGLRC* hRC) {
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    *hdc = GetDC(hwnd);

    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 8;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hdc, &pfd);

    SetPixelFormat(*hdc, iFormat, &pfd);

    *hRC = wglCreateContext(*hdc);

    wglMakeCurrent(*hdc, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hdc, HGLRC hRC) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hdc);
}



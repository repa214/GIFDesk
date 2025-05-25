#ifndef WINDOW_PROC_H_INCLUDED
#define WINDOW_PROC_H_INCLUDED

#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <pthread.h>
#include <commctrl.h>
#include "../Libraries/GIF_LOAD/gif_load.h"

extern int DESTROY_WINDOW;

extern HWND hwnd;
extern HDC hdc;
extern HGLRC hRC;
extern WNDCLASSEX wcex;
extern MSG msg;
extern pthread_t thread;
extern PAINTSTRUCT ps;
extern RECT rect;
extern HINSTANCE hInstance;
extern int nCmdShow;
extern POINT p;
extern HMENU hMenu;
extern HMENU hSubMenu;
extern HMENU hLangMenu;
extern LONG_PTR exStyle;
extern FILE *file;
extern HBITMAP appIcon;
extern RECT res;

extern HWND hwnd_2;
extern WNDCLASSEX wcex_2;
extern HDC hdc_2;
extern HGLRC hRC_2;
extern MSG msg_2;
extern HWND hTrackbar;
extern HRGN hRgn;
extern HWND hButton;
extern HDC hdc_b;
extern HFONT hFont;

extern pthread_t render;

extern char str_size[11];
extern int WAITING;
extern int HOVERED;
extern int SETTING_POS;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc_2(HWND hwnd_2, UINT uMsg, WPARAM wParam, LPARAM lParam);
void WcexInit(WNDCLASSEX *wcex, const char *lpszClassName, WNDPROC Proc);
void GetApplicationIcon();

#endif // WINDOW_PROC_H_INCLUDED

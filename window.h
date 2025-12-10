#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include "types.h"
#include "render.h"

int WindowInit(Window* window, const char* lpszClassName, WNDPROC Proc);

int LoadWindow(Window* window, Settings* st, Window* parent,
               const char* classname,
               int xoffset, int yoffset, int width, int height,
               int settb, int settm, int setdaf, int setgl);

void LoadTrackBar(Trackbar* trackbar, Window* main_window,
                  int xoffset, int yoffset, int width, int height, int tooltip,
                  int vfrom, int vto, int sp, int id);

void LoadButton(Button* button, Window* main_window,
                int xoffset, int yoffset, int width, int height, int rgn,
                const char* text, int menu);

void ReloadWindow(Window* window, Settings* st, Data* dt);
void ReleaseWindow(Window* window);

void WcexInit(WNDCLASSEX* wcex, const char *lpszClassName, WNDPROC Proc, HINSTANCE hInstance);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void EnableOpenGL(HWND hwnd, HDC* hdc, HGLRC* hRC);
void DisableOpenGL(HWND hwnd, HDC hdc, HGLRC hRC);

#endif // WINDOW_H_INCLUDED

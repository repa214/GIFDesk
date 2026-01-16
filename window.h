#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include "types.h"
#include "render.h"

int WindowInit(Window* window, const char* lpszclassname, WNDPROC proc);

int LoadWindow(Window* window, Settings* st, Window* parent,
               Render* rd, const char* classname,
               int xoffset, int yoffset, int width, int height,
               int settb, int settm, int setdaf, int setgl, int alpha);

void LoadTrackBar(Trackbar* trackbar, Window* main_window,
                  int xoffset, int yoffset, int width, int height, int tooltip,
                  int vfrom, int vto, int sp, int id);

void LoadButton(Button* button, Window* main_window,
                int xoffset, int yoffset, int width, int height, int rgn,
                const char* text, int menu, const char* font_name);

void ReloadWindow(Window* window, Settings* st, Data* dt, uint8_t autoscaling);
void ReleaseWindow(Window* window);

void WcexInit(WNDCLASSEX* wcex, const char* lpszclassname, WNDPROC proc, HINSTANCE hinstance);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void EnableOpenGL(Window* window, Render* rd, HWND hwnd, HDC* hdc, HGLRC* hRC);
void DisableOpenGL(HWND hwnd, HDC hdc, HGLRC hRC);

#endif // WINDOW_H_INCLUDED

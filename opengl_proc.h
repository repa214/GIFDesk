#ifndef OPENGL_PROC_H_INCLUDED
#define OPENGL_PROC_H_INCLUDED

#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <pthread.h>
#include <commctrl.h>
#include <unistd.h>
#include <time.h>
#include "../Libraries/GIF_LOAD/gif_load.h"

extern HDC hdc;
extern HGLRC hRC;
extern float vertex[];
extern float texCoord[];
extern int DESTROY_WINDOW;
extern int k;
extern struct timeval t_start, t_current;
extern double start, current;
extern int DRAWING;
extern int start_animating;
extern float inaccuracy;

int CollisionWidth();
int CollisionHeight();
void VSleep(double s);
double GetTime();
void EnableOpenGL(HWND hwnd, HDC* hdc, HGLRC* hRC);
void DisableOpenGL(HWND hwnd, HDC hdc, HGLRC hRC);
void ShowFrame(int k);
void* RenderThread(void *arg);

#endif // OPENGL_PROC_H_INCLUDED

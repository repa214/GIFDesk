#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <pthread.h>
#include <commctrl.h>
#include "../Libraries/GIF_LOAD/gif_load.h"

#define APP_NAME "GIFDesk 0.78"

extern char settings_path[260];
extern char filename[260];
extern char str_size[11];
extern float size;
extern int TASKBAR;
extern int TOPMOST;

char* GetSettingsPath();
int WriteSettings(const char *filename, float size, int taskbar, int topmost);
int ReadSettings(int fi);

#endif // SETTINGS_H_INCLUDED

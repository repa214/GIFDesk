#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <pthread.h>
#include <commctrl.h>

#define APP_NAME "GIFDesk 0.116"

extern char settings_path[260];
extern char filename[260];
extern char str_size[11];
extern float size;
extern int TASKBAR;
extern int TOPMOST;
extern const char OFNfilter[190];

char* GetSettingsPath();
void WriteSettings(const char *filename, float size, int taskbar, int topmost, int lang);
int ReadSettings(int fi);

#endif // SETTINGS_H_INCLUDED

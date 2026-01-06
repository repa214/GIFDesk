#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#include "types.h"
#include "render.h"
#include "data.h"

void GetSettingsPath(Settings* st);

void* _LoadSettings(void* arg);
void _LoadDropFile(HDROP drop, Window* window, Settings* st, Data* dt, Render* rd);

uint8_t GetSettings(Settings* st);
uint8_t SetSettings(Window* window, Settings* st, Data* dt);
void SwapFilenames(Settings* st);

void WriteSettings(Settings* st);

#endif // SETTINGS_H_INCLUDED

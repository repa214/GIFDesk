#ifndef DATA_H_INCLUDED
#define DATA_H_INCLUDED

#include "types.h"
#include "render.h"

#define INCORRECT 0
#define GIF 1
#define APNG 2
#define WEBP 3
#define AVIF 4
#define MNG 5

#define INVALIDE_FORMAT "The file format is invalid or not supported."
#define INVALIDE_LOAD "An error occurred while processing the file: %u"

void DataInit(Data* dt);

uint8_t CheckFile(const char* filename, Data* dt);

uint8_t _CheckGIF(void* data, long size);
uint8_t _CheckAPNG(void* data, long size);
uint8_t _CheckWEBP(void* data, long size);
uint8_t _CheckAVIF(void* data, long size);
uint8_t _CheckMNG(void* data, long size);


uint8_t LoadFile(Window* window, Settings* st, Data* dt, unsigned char filetype);

uint8_t _LoadGIF(Window* window, Settings* st, Data* dt);
uint8_t _LoadAPNG(Window* window, Settings* st, Data* dt);
uint8_t _LoadWEBP(Window* window, Settings* st, Data* dt);
uint8_t _LoadAVIF(Window* window, Settings* st, Data* dt);
uint8_t _LoadMNG(Window* window, Settings* st, Data* dt);

void _GLImage(Window* window, Data* dt);
void _ChangeTexFilt(Window* window, Data* dt, GLint param);

void ClearMedia(Data* dt);
void ClearFileData(Data* dt);
void ClearData(Data* dt);

void ShowData(Data* dt);

#endif // DATA_H_INCLUDED

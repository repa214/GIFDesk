#ifndef LOADGIF_H_INCLUDED
#define LOADGIF_H_INCLUDED

#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <pthread.h>
#include <commctrl.h>
#include "gif/gif_load.h"
#include "webp/decode.h"
#include "webp/demux.h"
#include "apng/png.h"

#define GIF_FORMAT 1
#define WEBP_FORMAT 2
#define PNG_FORMAT 3
#define APNG_FORMAT 4

extern void *filedata;
extern long filesize;
extern int filetype;
extern GLuint *textures;
extern int width;
extern int height;
extern int checkwidth;
extern int checkheight;
extern int fc;
extern double *delays;
extern int past_mode;
extern int frames;

void LoadProgress();
void BindFrame();
void WriteGIFFrames(void *anim, struct GIF_WHDR *whdr);
void CheckGIFFrames(void *data, struct GIF_WHDR *whdr);
void LoadFile(const char *filename, int type);
int CheckFile(const char *filename);

#endif // LOADGIF_H_INCLUDED

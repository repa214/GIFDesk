#ifndef LOADGIF_H_INCLUDED
#define LOADGIF_H_INCLUDED

#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <pthread.h>
#include <commctrl.h>
#include "../Libraries/GIF_LOAD/gif_load.h"

extern GLuint *textures;
extern int width;
extern int height;
extern int checkwidth;
extern int checkheight;
extern int fc;
extern double *delays;
extern int past_mode;
extern int tran_lp;
extern int tran_t;
extern int frames;

void WriteFrames(void *anim __attribute__((unused)), struct GIF_WHDR *whdr);
void LoadTextures(const char *filename, int tran_lp);
int CheckExtension(const char *filename, int fs);
void RenderThread();

#endif // LOADGIF_H_INCLUDED

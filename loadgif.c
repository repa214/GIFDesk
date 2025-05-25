#include "loadgif.h"
#include "opengl_proc.h"
#include "window_proc.h"
#include "settings.h"

GLuint *textures;
unsigned char *frame;
int width = 0;
int height = 0;
int checkwidth = 0;
int checkheight = 0;
int fc = 0;
double *delays;
int past_mode = 0;
int tran_t = 0;
int frames = 0;

/**
        WriteFrames
**/

void WriteFrames(void *anim __attribute__((unused)), struct GIF_WHDR *whdr) {
    textures = (GLuint *)realloc(textures, sizeof(GLuint) * (fc + 1));
    delays = (double *)realloc(delays, sizeof(double) * (fc + 1));
    *(delays + fc) = (whdr->time) ? (double)whdr->time / 100 : 0.1;

    if (whdr->mode == GIF_BKGD && past_mode == GIF_BKGD) memset(frame, 0, (width + 1) * (height + 1) * 4);
    else if (whdr->mode == GIF_CURR && past_mode == GIF_BKGD) memset(frame, 0, (width + 1) * (height + 1) * 4);
    else if (whdr->mode == GIF_PREV) memset(frame, 0, (width + 1) * (height + 1) * 4);

    unsigned int index = (width * whdr->fryo) * 4;

    for (long y = 0; y < whdr->fryd; y++) {
        index += whdr->frxo * 4;
        for (long x = 0; x < whdr->frxd; x++) {

            uint8_t i = whdr->bptr[y * whdr->frxd + x];

            if (whdr->tran != i) {
                frame[index] = (whdr->cpal[i].R) ? whdr->cpal[i].R : whdr->cpal[i].R + 1;
                frame[index + 1] = (whdr->cpal[i].G) ? whdr->cpal[i].G : whdr->cpal[i].G + 1;
                frame[index + 2] = (whdr->cpal[i].B) ? whdr->cpal[i].B : whdr->cpal[i].B + 1;
                frame[index + 3] = 255;
            }

            index += 4;
        }
        index += (width - whdr->frxd - whdr->frxo) * 4;
    }

    index = 0;
    for (long y = 0; y < height; y++) {
        for (long x = 0; x < width; x++) {
            if (x >= width - 1 || y >= height - 1) frame[index + 3] = 0;
            index += 4;
        }
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ZERO);

    glGenTextures(1, &textures[fc]);
    glBindTexture(GL_TEXTURE_2D, textures[fc]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                                0, GL_RGBA, GL_UNSIGNED_BYTE, frame);

    glBindTexture(GL_TEXTURE_2D, 0);

    BeginPaint(hwnd, &ps);

    HBRUSH hBrush = CreateSolidBrush(RGB(190, 190, 190));
    GetClientRect(hwnd, &rect);
    SelectObject(hdc, hBrush);

    Rectangle(hdc, -1, -1, width * size, 30);
    hBrush = CreateSolidBrush(RGB(0, 255, 0));
    SelectObject(hdc, hBrush);
    Rectangle(hdc, -1, -1, (int)(((float)fc / (float)frames) * (float)width * size), 30);

    DeleteObject(hBrush);
    EndPaint(hwnd, &ps);

    InvalidateRect(hwnd, NULL, FALSE);

    past_mode = whdr->mode;
    fc++;
}

/**
        LoadTextures
**/

void LoadTextures(const char *filename, int tran_lp)
{
    textures = (GLuint *)malloc(sizeof(GLuint) * 1);
    frame = (unsigned char *)malloc((width + 1) * (height + 1) * 4);
    memset(frame, 0, (width + 1) * (height + 1) * 4);

    FILE *file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    void *data = malloc(size);
    fread(data, 1, size, file);
    fclose(file);

    GIF_Load(data, size, WriteFrames, NULL, NULL, 0);

    free(data); free(frame);
}

/**
        CheckFrames
**/

void CheckFrames(void *data, struct GIF_WHDR *whdr) {
    checkwidth = whdr->xdim;
    checkheight = whdr->ydim;
}

/**
        CheckExtension
**/

int CheckExtension(const char *filename, int fs)
{
    checkwidth = 0; checkheight = 0;
    void *data;

    FILE *file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    data = malloc(size);
    fread(data, 1, size, file);
    fclose(file);

    int frame_count = GIF_Load(data, size, CheckFrames, NULL, NULL, 0);
    free(data);

    if (checkwidth && checkheight) {
        width = checkwidth; height = checkheight;
        return frame_count;
    }

    return 0;
}

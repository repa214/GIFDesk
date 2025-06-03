#include "loadgif.h"
#include "opengl_proc.h"
#include "window_proc.h"
#include "settings.h"

void *filedata = NULL;
long filesize = 0;
int filetype = 0;
GLuint *textures = NULL;
unsigned char *frame = NULL;
int width = 0;
int height = 0;
int checkwidth = 0;
int checkheight = 0;
int fc = 0;
double *delays = NULL;
int past_mode = 0;
int frames = 0;
WebPAnimDecoder* dec;

/**    Показывает полосу загрузки    **/

void LoadProgress() {
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
}

/**    Генерирует текстуру для кадра    **/

void BindFrame() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenTextures(1, &textures[fc]);
    glBindTexture(GL_TEXTURE_2D, textures[fc]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width + 1, height + 1,
                                0, GL_RGBA, GL_UNSIGNED_BYTE, frame);

    glBindTexture(GL_TEXTURE_2D, 0);
}

/**    Записывает кадры в память    **/

void WriteGIFFrames(void *anim, struct GIF_WHDR *whdr) {
    textures = (GLuint *)realloc(textures, sizeof(GLuint) * (fc + 1));
    delays = (double *)realloc(delays, sizeof(double) * (fc + 1));
    *(delays + fc) = (whdr->time) ? (double)whdr->time / 100 : 0.1;

    if (whdr->mode == GIF_BKGD && past_mode == GIF_BKGD) memset(frame, 0, (width + 1) * (height + 1) * 4);
    else if (whdr->mode == GIF_CURR && past_mode == GIF_BKGD) memset(frame, 0, (width + 1) * (height + 1) * 4);
    else if (whdr->mode == GIF_PREV) memset(frame, 0, (width + 1) * (height + 1) * 4);

    unsigned int index = ((width + 1) * whdr->fryo) * 4;

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
        index += ((width + 1) - whdr->frxd - whdr->frxo) * 4;
    }

    BindFrame();
    LoadProgress();

    past_mode = whdr->mode;
    fc++;
}

void WriteWEBPFrames() {
    WebPData webp_data = { filedata, (size_t)filesize };

    WebPAnimDecoderOptions dec_options;
    WebPAnimDecoderOptionsInit(&dec_options);
    dec_options.color_mode = MODE_RGBA;

    dec = WebPAnimDecoderNew(&webp_data, &dec_options);

    int prev_timestamp = 0;
    while (WebPAnimDecoderHasMoreFrames(dec)) {
        unsigned char *buff = NULL;
        textures = (GLuint *)realloc(textures, sizeof(GLuint) * (fc + 1));
        delays = (double *)realloc(delays, sizeof(double) * (fc + 1));

        int timestamp;
        if (!WebPAnimDecoderGetNext(dec, &buff, &timestamp)) {
            printf("Error: Failed to decode frame\n");
            break;
        }

        *(delays + fc) = (double)(timestamp - prev_timestamp) / 1000;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int src_idx = 4 * (y * width + x);
                int dst_idx = 4 * (y * (width + 1) + x);
                memcpy(frame + dst_idx, buff + src_idx, 4);
            }
        }

        BindFrame();
        LoadProgress();

        prev_timestamp = timestamp;
        fc++;

    }
    WebPAnimDecoderDelete(dec); dec = NULL;
}

/**   Check frames functions   **/

void CheckGIFFrames(void *data, struct GIF_WHDR *whdr) {
    width = whdr->xdim;
    height = whdr->ydim;
    frames++;
}

int CheckWEBPFrames() {
    WebPData webp_data = { filedata, (size_t)filesize };
    WebPAnimDecoderOptions dec_options;
    WebPAnimDecoderOptionsInit(&dec_options);
    dec_options.color_mode = MODE_RGBA;

    dec = WebPAnimDecoderNew(&webp_data, &dec_options);
    if (!dec) {
        return 0;
    }

    WebPAnimInfo anim_info;
    if (!WebPAnimDecoderGetInfo(dec, &anim_info)) {
        WebPAnimDecoderDelete(dec);
        return 0;
    }

    width = anim_info.canvas_width;
    height = anim_info.canvas_height;
    frames = (int)anim_info.frame_count;

    WebPAnimDecoderDelete(dec); dec = NULL;
    return 1;
}


/**   Loads file to memory   **/

void LoadFile(const char *filename, int type)
{
    if (delays != NULL) { free(delays); delays = NULL; }
    if (textures != NULL) { free(textures); textures = NULL; }

    frame = (unsigned char *)malloc((width + 1) * (height + 1) * 4);
    memset(frame, 0, (width + 1) * (height + 1) * 4);

    switch (type) {
        case GIF_FORMAT: {
            GIF_Load(filedata, filesize, WriteGIFFrames, NULL, NULL, 0);
        }   break;
        case WEBP_FORMAT: {
            WriteWEBPFrames();
        }   break;
    }

    if (filedata != NULL) { free(filedata); filedata = NULL; }
    if (frame != NULL) { free(frame); frame = NULL; }
}

/**   Check if extension exists and is correct format   **/

int CheckFile(const char *filename)
{
    frames = 0;

    FILE *file = fopen(filename, "rb");
    if (file == NULL) return 0;

    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    filedata = malloc(filesize);

    fread(filedata, 1, filesize, file);
    fclose(file); file = NULL;

    if (GIF_Load(filedata, filesize, CheckGIFFrames, NULL, NULL, 0)) {
        return GIF_FORMAT;
    }
    else if (CheckWEBPFrames()) {
        return WEBP_FORMAT;
    }
    else if (0) { // APNG
        return 0;
    }
    else if (0) { // AVIF
        return 0;
    }
    else if (0) { // MNG
        return 0;
    }
    else {
        free(filedata); filedata = NULL;
        return 0;
    }
}



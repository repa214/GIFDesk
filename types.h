#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <gl/gl.h>

#include <windows.h>

#include <commctrl.h>
#include <locale.h>
#include <inttypes.h>

#include <pthread.h>

#include "gif/gif_lib.h"
#include "apng/png.h"
#include "webp/decode.h"
#include "webp/demux.h"
#include "avif/avif.h"

#define APP_NAME "GIFDesk 1.0.2"

#define IDI_ICON 1
#define MENU_ICON 2

#define ID_SELECT 3003
#define ID_EXIT 3004

#define IDD_DIALOG_BEXIT 1000
#define IDD_DIALOG_BEXIT_E 1001
#define IDD_DIALOG_BCANCEL 1002
#define IDD_DIALOG_BCANCEL_E 1003

typedef struct
{
    HWND hwnd;
    HDC hdc;
    HGLRC hrc;
    WNDCLASSEX wcex;
    HINSTANCE hinstance;

    int hovered;
    int isactive;

} Window;

typedef struct
{
    HWND hwnd;
    HDC hdc;
    HGLRC hrc;
    HFONT font;
    HFONT arrow;

    int hovered;
    int isactive;
} Button;

typedef struct
{
    HWND hwnd;
    HDC hdc;
    HGLRC hrc;

    int hovered;
    int isactive;
} Trackbar;

typedef struct
{
    char settings_path[MAX_PATH];
    char filename[MAX_PATH];
    char buff_filename[MAX_PATH];
    const char ofnfilter[233];
    char str_size[32];

    float size;
    int taskbar;
    int topmost;
    int lang;

    /** doesn`t contains in file **/

    int sfu; /// show frame updates

    /** for temporary changes **/

    float trackbar_size;
    int pos;

    OPENFILENAME ofn;

} Settings;

typedef struct
{
    void* data; long size;

    GLuint* textures;
    float* frame_points;
    float* delays;

    unsigned char* frame;
    unsigned char* buff;

    int width;
    int height;
    int count;

    unsigned char prev_mode;
    unsigned char prev_frxo;
    unsigned char prev_fryo;
    unsigned char prev_frxd;
    unsigned char prev_fryd;

    uint8_t error;
    /// returns 0 if fine and >1 if file is not valid
    /// returns 0x01 ... 0x3F (1-63)     _LoadGIF
    /// returns 0x40 ... 0x7F (64-127)   _LoadAPNG
    /// returns 0x80 ... 0xBF (128-191)  _LoadWEBP
    /// returns 0xC0 ... 0xDF (192-223)  _LoadAVIF
    /// returns 0xE0 ... 0xFE (224-254)  _LoadMNG

    /// (maybe) FILE_CORRUPTED              1
    /// D_GIF_ERR_OPEN_FAILED               2    /* And DGif possible errors. */
    /// D_GIF_ERR_READ_FAILED               3
    /// D_GIF_ERR_NOT_GIF_FILE              4
    /// D_GIF_ERR_NO_SCRN_DSCR              5
    /// D_GIF_ERR_NO_IMAG_DSCR              6
    /// D_GIF_ERR_NO_COLOR_MAP              7
    /// D_GIF_ERR_WRONG_RECORD              8
    /// D_GIF_ERR_DATA_TOO_BIG              9
    /// D_GIF_ERR_NOT_ENOUGH_MEM            10
    /// D_GIF_ERR_CLOSE_FAILED              11
    /// D_GIF_ERR_NOT_READABLE              12
    /// D_GIF_ERR_IMAGE_DEFECT              13
    /// D_GIF_ERR_EOF_TOO_SOON              14

    /// (Compromised)
    /// APNG_ERR_CREATE_READ_STRUCT         64
    /// APNG_ERR_CREATE_INFO_STRUCT         65
    /// APNG_ERR_ALLOC_ROW_POINTERS         67
    /// APNG_ERR_ALLOC_ROW_POINTER          68

    /// WEBP_ERR_NOT_WEBP_FILE              128
    /// WEBP_ERR_DECODER_OPTIONS            129
    /// VP8_STATUS_OUT_OF_MEMORY            130
    /// VP8_STATUS_INVALID_PARAM            131
    /// VP8_STATUS_BITSTREAM_ERROR          132
    /// VP8_STATUS_UNSUPPORTED_FEATURE      133
    /// VP8_STATUS_SUSPENDED                134
    /// VP8_STATUS_USER_ABORT               135
    /// VP8_STATUS_NOT_ENOUGH_DATA          136
    /// VP8_STATUS_UNKNOWN                  137
    /// WEBP_ERR_GET_ANIM_INFO              138

    /// AVIF_ERR_CREATE_DECODER             192
    /// AVIF_ERR_SET_INFO                   193
    /// AVIF_RESULT_NO_CONTENT              194
    /// AVIF_RESULT_NO_YUV_FORMAT_SELECTED  195
    /// AVIF_RESULT_REFORMAT_FAILED         196
    /// AVIF_RESULT_UNSUPPORTED_DEPTH       197
    /// AVIF_RESULT_BMFF_PARSE_FAILED       198
    /// AVIF_RESULT_NO_AV1_ITEMS_FOUND      199
    /// AVIF_RESULT_DECODE_COLOR_FAILED     200
    /// AVIF_RESULT_DECODE_ALPHA_FAILED     201
    /// AVIF_RESULT_COLOR_ALPHA_SIZE_MISMATCH 202
    /// AVIF_RESULT_ISPE_SIZE_MISMATCH      203
    /// AVIF_RESULT_NO_CODEC_AVAILABLE      204
    /// AVIF_RESULT_NO_IMAGES_REMAINING     205
    /// AVIF_RESULT_INVALID_EXIF_PAYLOAD    206
    /// AVIF_RESULT_INVALID_IMAGE_GRID      207
    /// AVIF_RESULT_INVALID_CODEC_SPECIFIC_OPTION 208
    /// AVIF_RESULT_TRUNCATED_DATA          209
    /// AVIF_RESULT_IO_NOT_SET              210
    /// AVIF_RESULT_IO_ERROR                211
    /// AVIF_RESULT_WAITING_ON_IO           212
    /// AVIF_RESULT_INVALID_ARGUMENT        213
    /// AVIF_RESULT_NOT_IMPLEMENTED         214
    /// AVIF_RESULT_OUT_OF_MEMORY           215
    /// AVIF_RESULT_CANNOT_CHANGE_SETTING   216
    /// AVIF_RESULT_INCOMPATIBLE_IMAGE      217
    /// AVIF_ERR_CONVERT_YUV_TO_RGB         218
    /// AVIF_ERR_UNKNOWN                    219

    /// S_MEMORY_ALLOC_ERROR          15   (For evey format)
    /// (maybe) FILE_CORRUPTED        66   (For evey format)

    /// INVALIDE_HEADER_FORMAT        255
} Data;

typedef struct
{
    int frame; // current_frame
    int render_thread;

    double start_time, current_time, inaccuracy;

    int change_frames;
    int framed_trackbar;
    int loading;
} Render;

#endif // TYPES_H_INCLUDED

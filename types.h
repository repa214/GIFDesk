#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#define SINLINE inline static

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#include <gl/gl.h>
#include <setjmp.h>

#include "libgif/gif_lib.h"
#include "libapng/png.h"
#include "libapng/pngconf.h"
#include "libapng/pnglibconf.h"
#include "libwebp/decode.h"
#include "libwebp/demux.h"

//--------------------------------------------------
// APP
//--------------------------------------------------

#define APP_NAME "GIFDesk"
#define APP_NAME_VER "GIFDesk 1.4b.2"
#define APP_GENERAL_VER "Version 1.4b.2"
// build-NNDDMMYY
#define APP_BUILD_DESC "Public second beta build-02070626"
#define RESOURCE_ICON 1

//--------------------------------------------------
// ENUMES
//--------------------------------------------------

enum {MANAGER_BTN_NEWFILE = 1, MANAGER_BTN_FILE, MANAGER_BTN_SETTINGS, MANAGER_BTN_BACK,
      MANAGER_BTN_PFRAME, MANAGER_BTN_MFRAME, MANAGER_BTN_FRAMEPAUSE,
      MANAGER_BTN_PSPEED, MANAGER_BTN_MSPEED,
      MANAGER_BTN_PSCALE, MANAGER_BTN_MSCALE,

//    ST1                  ST2                   ST3
      MANAGER_BTN_GENERAL, MANAGER_BTN_TEMPLATE, MANAGER_BTN_PLAYBACK,
      MANAGER_BTN_MSPEED_ST2, MANAGER_BTN_PSPEED_ST2,
      MANAGER_BTN_MSCALE_ST2, MANAGER_BTN_PSCALE_ST2,

      MANAGER_STATIC_LABEL, MANAGER_STATIC_DESCRIPTION, MANAGER_STATIC_FRAMES, MANAGER_STATIC_SPEED,
      MANAGER_STATIC_SCALE,
      MANAGER_STATIC_DM,

      MANAGER_STATIC_LABELST, MANAGER_STATIC_DESCRIPTIONST,
      MANAGER_STATIC_SPEED_ST2, MANAGER_STATIC_SCALE_ST2,
      MANAGER_STATIC_LABEL_ST2, MANAGER_STATIC_DESCRIPTION_ST2,
      MANAGER_STATIC_DM_ST2, MANAGER_STATIC_LABELST3,

      MANAGER_SCROLL_CONFIG, MANAGER_SCROLL_FILES,

      MANAGER_SWITCH_LABEL, MANAGER_SWITCH_AOT, MANAGER_SWITCH_STI,
      MANAGER_SWITCH_DM, MANAGER_SWITCH_HH, MANAGER_SWITCH_CT, MANAGER_SWITCH_LSB,
      MANAGER_SWITCH_MSTC,
      MANAGER_SWITCH_AOT_ST2, MANAGER_SWITCH_STI_ST2,
      MANAGER_SWITCH_DM_ST2, MANAGER_SWITCH_HH_ST2, MANAGER_SWITCH_CT_ST2,
      MANAGER_SWITCH_SP,

      MANAGER_RECT_TITLE, MANAGER_RECT_DIV, MANAGER_RECT_DIV_LINE,

//    ST1                 ST2
      MANAGER_RECT_DIVST, MANAGER_RECT_DIVST2,
      MANAGER_RECT_DIV_LINEST, MANAGER_RECT_DIV_ENDST,
      MANAGER_RECT_TITLE_ST2, MANAGER_RECT_DIV_LINE_ST2,
      MANAGER_RECT_DIVST3,

      MANAGER_SCROLLLINEAR_FRAMES, MANAGER_SCROLLLINEAR_SPEED, MANAGER_SCROLLLINEAR_SCALE,
      MANAGER_SCROLLLINEAR_SPEED_ST2, MANAGER_SCROLLLINEAR_SCALE_ST2,

//    ST1                 ST2                 ST3
      MANAGER_SCROLL_ST1, MANAGER_SCROLL_ST2, MANAGER_SCROLL_ST3,

      MANAGER_TEXTBOX_FRAMES, MANAGER_TEXTBOX_SPEED, MANAGER_TEXTBOX_SCALE,
      MANAGER_TEXTBOX_SPEED_ST2, MANAGER_TEXTBOX_SCALE_ST2,

      MANAGER_THREAD_CREATEOBJECT, MANAGER_THREAD_CREATEWINDOW,
      MANAGER_THREAD_DELETEOBJECT, MANAGER_THREAD_DELETEWINDOW,

      POPUP_OPENFILE = 1024, POPUP_SAVESETTINGS_AS, POPUP_SAVESETTINGS, POPUP_DELETEFILE, POPUP_CONFIGUREFILE,
      POPUP_MOVEWINDOW_LTC, POPUP_MOVEWINDOW_LLC, POPUP_MOVEWINDOW_C, POPUP_MOVEWINDOW_RTC, POPUP_MOVEWINDOW_RLC,
      POPUP_OPENGIFDESK, POPUP_QUIT, POPUP_CLOSEFILES
};

enum {POS_NULL, POS_LTC, POS_LLC, POS_RTC, POS_RLC, POS_C};
enum {NO_FORMAT, GIF, APNG, WEBP, AVIF, CUSTOM};
enum {FONT_NULL_ID, FONT_MAIN_ID, FONT_CORBEL_ID, FONT_LABEL_NAME_ID, FONT_LABEL_ID, FONT_DESCRIPTION_ID,
      FONT_BTN_ID, FONT_MARLETT_ID, FONT_ARIAL_ID, FONT_FRAMES_ID};

//--------------------------------------------------
// COLORS
//--------------------------------------------------

#define MGR_COLOR_BKG                           RGB(252, 252, 252)

// RECT                                         disabled            enabled             hovered             pressed
#define MGR_COLOR_RECT_DEFAULT                  RGB(235, 235, 235), RGB(235, 235, 235), RGB(235, 235, 235), RGB(235, 235, 235)
#define MGR_COLOR_RECT_DIV_LINE                 RGB(225, 225, 225), RGB(225, 225, 225), RGB(225, 225, 225), RGB(225, 225, 225)
#define MGR_COLOR_RECT_DIV                      RGB(245, 245, 245), RGB(245, 245, 245), RGB(245, 245, 245), RGB(245, 245, 245)
// for background rect
#define MGR_COLOR_RECT_BKG                      MGR_COLOR_BKG,      MGR_COLOR_BKG,      MGR_COLOR_BKG,      MGR_COLOR_BKG

// STATIC BACKGROUND
#define MGR_COLOR_STATIC_BACKGROUND_DEFAULT     MGR_COLOR_BKG,      MGR_COLOR_BKG,      MGR_COLOR_BKG,      MGR_COLOR_BKG
#define MGR_COLOR_STATIC_DIV                    RGB(245, 245, 245), RGB(245, 245, 245), RGB(245, 245, 245), RGB(245, 245, 245)
// STATIC TEXT
#define MGR_COLOR_STATIC_TEXT_DEFAULT           RGB(130, 130, 130), RGB(30, 30, 30),    RGB(30, 30, 30),    RGB(30, 30, 30)
#define MGR_COLOR_STATIC_TEXT_DESCRIPTION       RGB(170, 170, 170), RGB(110, 110, 110), RGB(110, 110, 110), RGB(110, 110, 110)
#define MGR_COLOR_STATIC_TEXT_BRIGHT            RGB(130, 130, 130), RGB(130, 130, 130), RGB(130, 130, 130), RGB(130, 130, 130)

// BUTTON BACKGROUND
#define MGR_COLOR_BUTTON_BACKGROUND_DEFAULT     MGR_COLOR_BKG,      MGR_COLOR_BKG,      RGB(235, 235, 235), RGB(220, 220, 220)
#define MGR_COLOR_BUTTON_BACKGROUND_DEFAULT_S   MGR_COLOR_BKG,      RGB(240, 240, 240), RGB(225, 225, 225), RGB(210, 210, 210)
#define MGR_COLOR_BUTTON_BACKGROUND_DIV         RGB(245, 245, 245), RGB(230, 230, 230), RGB(220, 220, 220), RGB(205, 205, 205)
#define MGR_COLOR_BUTTON_BACKGROUND_DIV_T       RGB(245, 245, 245), RGB(210, 210, 210), RGB(195, 195, 195), RGB(185, 185, 185)
// BUTTON TEXT
#define MGR_COLOR_BUTTON_TEXT_DEFAULT           RGB(110, 110, 110), RGB(30, 30, 30),    RGB(30, 30, 30),    RGB(50, 50, 50)

// SWITCH BACKGROUND
#define MGR_COLOR_SWITCH_BACKGROUND_DEFAULT     MGR_COLOR_BKG,      MGR_COLOR_BKG,      MGR_COLOR_BKG,      MGR_COLOR_BKG
#define MGR_COLOR_SWITCH_BACKGROUND_DIV         RGB(245, 245, 245), RGB(245, 245, 245), RGB(245, 245, 245), RGB(245, 245, 245)
// SWITCH SLIDER
#define MGR_COLOR_SWITCH_SLIDER_DEFAULT         RGB(235, 235, 235), RGB(225, 225, 225), RGB(225, 225, 225), RGB(225, 225, 225)
// SWITCH SLIDERT
#define MGR_COLOR_SWITCH_SLIDERT_DEFAULT        RGB(235, 235, 235), RGB(205, 205, 205), RGB(205, 205, 205), RGB(205, 205, 205)
// SWITCH DOT
#define MGR_COLOR_SWITCH_DOT_DEFAULT            RGB(210, 210, 210), RGB(150, 150, 150), RGB(145, 145, 145), RGB(145, 145, 145)

// SCROLLBAR BACKGROUND
#define MGR_COLOR_SCROLLBAR_BACKGROUND_DEFAULT  MGR_COLOR_BKG,      MGR_COLOR_BKG,      MGR_COLOR_BKG,      MGR_COLOR_BKG
// SCROLLBAR SLIDER
#define MGR_COLOR_SCROLLBAR_SLIDER_DEFAULT      MGR_COLOR_BKG,      RGB(205, 205, 205), RGB(195, 195, 195), RGB(180, 180, 180)

// LISTBOX BACKGROUND
#define MGR_COLOR_LISTBOX_BACKGROUND_DEFAULT    RGB(240, 240, 240), RGB(235, 235, 235), RGB(225, 225, 225), RGB(220, 220, 220)
// LISTBOX ARROW
#define MGR_COLOR_LISTBOX_ARROW_DEFAULT         RGB(215, 215, 215), RGB(165, 165, 165), RGB(135, 135, 135), RGB(130, 130, 130)
// LISTBOX TEXT
#define MGR_COLOR_LISTBOX_TEXT_DEFAULT          RGB(110, 110, 110), RGB(30, 30, 30),    RGB(30, 30, 30),    RGB(50, 50, 50)

// SCROLLLINEAR BACKGROUND
#define MGR_COLOR_SCRLINEAR_BACKGROUND_DEFAULT  MGR_COLOR_BKG,      MGR_COLOR_BKG,      MGR_COLOR_BKG,      MGR_COLOR_BKG
#define MGR_COLOR_SCRLINEAR_BACKGROUND_DIV      RGB(245, 245, 245), RGB(245, 245, 245), RGB(245, 245, 245), RGB(245, 245, 245)
// SCROLLLINEAR LINE
#define MGR_COLOR_SCRLINEAR_LINE_DEFAULT        RGB(215, 215, 215), RGB(225, 225, 225), RGB(220, 220, 220), RGB(220, 220, 220)
#define MGR_COLOR_SCRLINEAR_LINE_DARK           RGB(1, 1, 1),       RGB(1, 1, 1),       RGB(1, 1, 1),       RGB(1, 1, 1)
// SCROLLLINEAR ARROW
#define MGR_COLOR_SCRLINEAR_ARROW_DEFAULT       RGB(215, 215, 215), RGB(185, 185, 185), RGB(170, 170, 170), RGB(160, 160, 160)
#define MGR_COLOR_SCRLINEAR_ARROW_DARK          RGB(1, 1, 1),       RGB(1, 1, 1),       RGB(1, 1, 1),       RGB(1, 1, 1)

// TEXTBOX BACKGROUND
#define MGR_COLOR_TEXTBOX_DIV                   RGB(245, 245, 245)

// NULL
#define MGR_COLOR_DARK                          RGB(1, 1, 1),       RGB(1, 1, 1),       RGB(1, 1, 1),       RGB(1, 1, 1)

//--------------------------------------------------
// OPENGL
//--------------------------------------------------

#define GL_CLAMP_TO_EDGE 0x812F

//--------------------------------------------------
// WINAPI
//--------------------------------------------------

#define MGR_MINWINDOWSIZEX 520
#define MGR_MINWINDOWSIZEY 100
#define WM_SCRLNR          (WM_USER + 1)
#define ID_TRAY            900
#define WM_TRAYNOTIFY      (ID_TRAY + 1)
#define DPI_DEFAULT        96.0

//--------------------------------------------------
// GIFDESK
//--------------------------------------------------

typedef struct {
    unsigned char* data;
    size_t size;
    size_t position;
}
GIFReader;

typedef struct {
    uint16_t index, obj_index;
    void* mgr;

    /** WINAPI **/
    HWND window;
    HDC hdc;
    HGLRC hrc;
    HINSTANCE hinst;

#define SETTINGS_AOT 0x1
#define SETTINGS_STI 0x2
#define SETTINGS_DM  0x4
#define SETTINGS_HH  0x8
#define SETTINGS_CT 0x10
    char filepath[MAX_PATH], filename[MAX_PATH];
    float size;
    int16_t x, y;
    uint16_t speed;
    uint8_t flags, transparency, lang;
    uint8_t settings_pos, sw_show, sfu /* Will be cut soon */, pos;
    OPENFILENAME ofn;

    /** DATA **/
    void* data; size_t data_size;
    GLuint* textures; unsigned int count;
    float *frame_points, *delays, *lengths;
    uint8_t *frame, *buff;
    uint16_t width, height, npotwidth, npotheight;

    /** RENDER **/
    uint16_t render_frame;
    uint8_t render_thread;
    double start_time;
    uint8_t change_frames, schange_frames, render_pos;
    float glversion;
    float vertex[8], texcoord[8];
}
GIFDesk;

//--------------------------------------------------
// MANAGER
//--------------------------------------------------

typedef struct {
    unsigned long disabled, enabled, hovered, pressed;
}
COLORSTATE;

typedef struct {
    uint16_t index, id, gfk_index;
    void* parent;

    /** WINAPI **/
    HWND window;
    HRGN rgn;
    HFONT marlett;

#define OBJ_RECT      1
#define OBJ_STATIC    2
#define OBJ_BUTTON    3
#define OBJ_SWITCH    4
#define OBJ_SCROLLBAR 5
#define OBJ_LISTBOX   6
#define OBJ_SCRLINEAR 7
#define OBJ_TEXTBOX   8
    uint8_t type;
#define OBJ_BOOL                0x1
#define OBJ_HOVERED             0x2
#define OBJ_CHOVERED            0x4
#define OBJ_CHECKED             0x8
#define OBJ_REPEAT             0x10
#define OBJ_LBSOBJ             0x20
#define OBJ_TRACKBAR_OVERFLOW  0x40
    uint8_t flags;
#define OBJ_ALIGNR 0x1
#define OBJ_ALIGNB 0x2
#define OBJ_ALIGNRB (OBJ_ALIGNR | OBJ_ALIGNB)
#define OBJ_ALIGNBR (OBJ_ALIGNB | OBJ_ALIGNR)
#define OBJ_ALIGNW 0x4
#define OBJ_ALIGNH 0x8
#define OBJ_ALIGNWH (OBJ_ALIGNW | OBJ_ALIGNH)
#define OBJ_ALIGNHW (OBJ_ALIGNH | OBJ_ALIGNW)
#define OBJ_ALIGNRH (OBJ_ALIGNR | OBJ_ALIGNH)
#define OBJ_ALIGNBW (OBJ_ALIGNB | OBJ_ALIGNW)
#define OBJ_LBSDS 0x10
    uint8_t align;
    uint16_t aol, aot, aor, aob;
    uint16_t tol, tot, tor, tob;
    int rgnw, rgnh;
    int swsize;
#define OBJECT_COLORSTATE_COUNT 4
    COLORSTATE colormod[OBJECT_COLORSTATE_COUNT];
    unsigned int dt_format;
    uint32_t tab;
    int (*btnup)(void*), (*btndown)(void*);
    /** OBJ_SCROLLBAR **/
    int* stm; unsigned int stm_count; int stm_offset;
    int stm_object_ids[4];
    RECT stm_rect;
    int lts;
    RECT stm_mouse_rect;
    /** OBJ_LISTBOX **/
    void* lbsh;
    /** OBJ_SCRLINEAR **/
    int scr_index, scr_current, scr_min, scr_max, scr_info;
}
Object;

typedef struct {
    void* parent;   // Reference to parent object

    /** WINAPI **/

    HWND window, window_tray;
    HFONT* fonts; unsigned int fonts_count;
    MSG message;
    WNDCLASSEX wcex, wcextray, lbswcex, gfkwcex;
    NOTIFYICONDATA nid;

    /** DPI MANAGEMENT **/

    unsigned int dpi;
    float scale_dpi;

#define MGR_MMWnF     0x1
#define MGR_MMWF      0x2
#define MGR_MMSF      0x4
#define MGR_ST1       0x8
#define MGR_ST2       0x10
#define MGR_ST3       0x20
#define MGR_ST4       0x40
#define MGR_ST5       0x80
    uint32_t tab;                    // 32 tabs

    uint16_t error;                  // ???
    int width_buff, height_buff;     // _InvalidateResizedItems
    int cpx_buff, cpy_buff;          // _HandleManagerMouseMove
    int casb, last_casb;             // Current Active ScrollBar
    unsigned int lbsh_count;         // ListBox count

    /** DATA **/

    Object* objects; unsigned int objects_count;
    int context_id;
    GIFDesk* gfk; uint16_t gfk_count; int gfk_current;
    HANDLE glmutex;              // Denies multiple OpenGL usage
    char settings_path[MAX_PATH], ofnfilter[233], buff_filepath[MAX_PATH];
    uint8_t is_loading;

    /** SETTINGS (TEMPLATED) **/

    float size;
#define SETTINGS_MSTC 0x1
#define SETTINGS_SP   0x2
#define SETTINGS_CoTW 0x4   // Close on Tray Warning
    uint8_t settings_ver, settings, transparency, lang, flags, sfu;
    uint16_t speed;
    RECT rc_load, rc_release;
}
Manager;

typedef struct {
    Manager* manager;
    GIFDesk* gfk;
    uint16_t gfk_index;
}
GIFDeskLoop;

//--------------------------------------------------
// DEFINES
//--------------------------------------------------

// Default Button Delegate On Button Up
SINLINE int DEFBTNBTNUPDEL(void* args) { return 0; }
// Default Button Delegate On Button Down
SINLINE int DEFBTNBTNDWNDEL(void* args) { return 0; }

// Default ScrollBar Delegate On Button Up
SINLINE int DEFSCRBTNUPDEL(void* args) {
    Manager* manager = (Manager *)args;
    if (manager->casb >= 0) return 0;
    SetTimer(manager->window, 1, 10, NULL);

    return 0;
}
// Default ScrollBar Delegate On Button Down
SINLINE int DEFSCRBTNDWNDEL(void* args) { return 0; }

// Default Linear Scroll Delegate On Button Up
SINLINE int DEFSCLBTNUPDEL(void* args) {
    Manager* manager = (Manager *)args;
    if (manager->casb >= 0) return 0;
    SetTimer(manager->window, 2, 10, NULL);

    return 0;
}
// Default Linear Scroll Delegate On Button Down
SINLINE int DEFSCLBTNDWNDEL(void* args) { return 0; }

Object* _CreateManagerButton(Manager*, int, int, int, int, BOOL, int,
                             int, LPCTSTR, BOOL);

DWORD WINAPI GIFDeskFromParams(Manager*, uint16_t, char*, float, int16_t, int16_t,
                               uint16_t, uint8_t, uint8_t,
                               uint8_t, uint8_t);

//--------------------------------------------------
// MANAGER
//--------------------------------------------------

enum {
    // 4096
    MANAGER_ERR_UNKNOWN = 0x1000,

    MANAGER_ERR_CREATE_MUTEX, MANAGER_ERR_CREATE_WCEX, MANAGER_ERR_CREATE_WCEXTRAY, MANAGER_ERR_CREATE_LBSWCEX,
    MANAGER_ERR_CREATE_GFKWCEX, MANAGER_ERR_CREATE_WINDOW, MANAGER_ERR_CREATE_TRAY, MANAGER_ERR_CREATE_FONT,
    MANAGER_ERR_CREATE_OBJ, MANAGER_ERR_GET_OFNFILTER, MANAGER_ERR_GET_EXEPATH, MANAGER_ERR_GFK_NULL,

    // 8192
    MANAGER_WARN_SETTINGS = 0x2000, MANAGER_WARN_PARSE_SETTINGS, MANAGER_WARN_GIFDESK_ALLOC,
    MANAGER_WARN_FILE_NEXIST, MANAGER_WARN_NO_FORMAT, MANAGER_WARN_CREATE_WINDOW,
    MANAGER_WARN_FRAME_ALLOC, MANAGER_WARN_BUFF_ALLOC, MANAGER_WARN_DELAYS_ALLOC,
    MANAGER_WARN_LENGTHS_ALLOC, MANAGER_WARN_FRAMEP_ALLOC, MANAGER_WARN_TEXTURES_ALLOC,
    MANAGER_WARN_CREATE_OBJ, MANAGER_WARN_GFKLOOP_NULL, MANAGER_WARN_CREATE_LOOP,
    MANAGER_WARN_CREATE_THREAD, MANAGER_WARN_QUERY_TOO_MUCH, MANAGER_WARN_REG,
    MANAGER_WARN_OBJCOUNT_OVERFLOW, MANAGER_WARN_APPLY_WINDOW,

    // 12288
    MANAGER_WARN_OPENGL_NA = 0x3000, MANAGER_WARN_OPENGL_INVALID_ENUM,
    MANAGER_WARN_OPENGL_INVALID_VALUE, MANAGER_WARN_OPENGL_INVALID_OPERATION,
    MANAGER_WARN_OPENGL_STACK_OVERFLOW, MANAGER_WARN_OPENGL_STACK_UNDERFLOW,
    MANAGER_WARN_OPENGL_OUT_OF_MEMORY, MANAGER_WARN_OPENGL_OUT_OF_CONTEXT,

    // 16384
    MANAGER_WARN_GIF_OPEN_FAILED = 0x4000, MANAGER_WARN_GIF_READ_FAILED,
    MANAGER_WARN_GIF_NOT_GIF_FILE, MANAGER_WARN_GIF_NO_SCRN_DSCR,
    MANAGER_WARN_GIF_NO_IMAG_DSCR, MANAGER_WARN_GIF_NO_COLOR_MAP,
    MANAGER_WARN_GIF_WRONG_RECORD, MANAGER_WARN_GIF_DATA_TOO_BIG,
    MANAGER_WARN_GIF_NOT_ENOUGH_MEM, MANAGER_WARN_GIF_CLOSE_FAILED,
    MANAGER_WARN_GIF_NOT_READABLE, MANAGER_WARN_GIF_IMAGE_DEFECT,
    MANAGER_WARN_GIF_EOF_TOO_SOON, MANAGER_WARN_GIF_NULL,

    // 20480
    MANAGER_WARN_APNG_STRUCT = 0x5000, MANAGER_WARN_APNG_INFO,
    MANAGER_WARN_APNG_JUMP, MANAGER_WARN_APNG_POINTERS, MANAGER_WARN_APNG_POINTER,

    // 24576
    MANAGER_WARN_WEBP_INFO = 0x6000, MANAGER_WARN_WEBP_OPTIONS,
    MANAGER_WARN_WEBP_OUT_OF_MEMORY, MANAGER_WARN_WEBP_INVALID_PARAM, MANAGER_WARN_WEBP_BITSTREAM_ERROR,
    MANAGER_WARN_WEBP_UNSUPPORTED_FEATURE, MANAGER_WARN_WEBP_SUSPENDED, MANAGER_WARN_WEBP_USER_ABORT,
    MANAGER_WARN_WEBP_NOT_ENOUGH_DATA, MANAGER_WARN_WEBP_UNKNOWN, MANAGER_WARN_WEBP_DECODER,

    MANAGER_ERR_NULL = 0xFFFF
};

SINLINE void _DeleteObject(Object* obj) {
    if (obj) {
        if (obj->rgn) { DeleteObject(obj->rgn); obj->rgn = NULL; }
        if (obj->window) { DestroyWindow(obj->window); obj->window = NULL; }
        if (obj->stm) { free(obj->stm); obj->stm = NULL; }

        if (obj->lbsh) {
            Manager* lbsh = (Manager*)obj->lbsh;
            if (lbsh->window) DestroyWindow(lbsh->window);
            free(lbsh); obj->lbsh = NULL;
        }
    }
}

SINLINE int ManagerDestroy(Manager* manager) {
    if (manager) {
        SendMessage(manager->window, WM_COMMAND, POPUP_CLOSEFILES, 1);

        if (manager->objects) {
            for (int i = manager->objects_count - 1; i >= 0; i--)
                _DeleteObject(&manager->objects[i]);

            free(manager->objects); manager->objects = NULL;
        }

        if (manager->fonts) {
            for (unsigned int i = 0; i < manager->fonts_count; i++) {
                if (manager->fonts[i])
                    DeleteObject(manager->fonts[i]);
            }

            free(manager->fonts); manager->fonts = NULL;
        }

        if (manager->window) DestroyWindow(manager->window);
        if (manager->window_tray) {
            DestroyWindow(manager->window_tray);
            Shell_NotifyIcon(NIM_DELETE, NULL);
        }
        if (manager->glmutex) CloseHandle(manager->glmutex);
        free(manager); manager = NULL;
    }

    exit(EXIT_SUCCESS);
}

SINLINE void ManagerHandleError(Manager* manager) {
    char err[256];
    if (manager) {
        switch (manager->error) {
            case MANAGER_ERR_CREATE_MUTEX:
                sprintf(err, "0x%.8X (%d)\nFailed to create mutex. Handle is invalid. Please restart the app.", manager->error, manager->error);
                break;
            case MANAGER_ERR_CREATE_WCEX:
                sprintf(err, "0x%.8X (%d)\nFailed to create window class extension. Please restart the app.", manager->error, manager->error);
                break;
            case MANAGER_ERR_CREATE_WCEXTRAY:
                sprintf(err, "0x%.8X (%d)\nFailed to create listbox window class extension. Please restart the app.", manager->error, manager->error);
                break;
            case MANAGER_ERR_CREATE_LBSWCEX:
                sprintf(err, "0x%.8X (%d)\nFailed to create listbox window class extension. Please restart the app.", manager->error, manager->error);
                break;
            case MANAGER_ERR_CREATE_GFKWCEX:
                sprintf(err, "0x%.8X (%d)\nFailed to create animation window class extension. Please restart the app.", manager->error, manager->error);
                break;
            case MANAGER_ERR_CREATE_WINDOW:
                sprintf(err, "0x%.8X (%d)\nFailed to create main application window. Please restart the app.", manager->error, manager->error);
                break;
            case MANAGER_ERR_CREATE_FONT:
                sprintf(err, "0x%.8X (%d)\nFailed to create required font. Please restart the app.", manager->error, manager->error);
                break;
            case MANAGER_ERR_CREATE_TRAY:
                sprintf(err, "0x%.8X (%d)\nFailed to create system tray icon. Please restart the app.", manager->error, manager->error);
                break;
            case MANAGER_ERR_CREATE_OBJ:
                sprintf(err, "0x%.8X (%d)\nFailed to create an UI object. Please restart the app.", manager->error, manager->error);
                break;
            case MANAGER_ERR_GET_OFNFILTER:
                sprintf(err, "0x%.8X (%d)\nFailed to retrieve Open File dialog filter. Please restart the app.", manager->error, manager->error);
                break;
            case MANAGER_ERR_GFK_NULL:
                sprintf(err, "0x%.8X (%d)\nFailed to create tray window class extension. Please restart the app.", manager->error, manager->error);
                break;
            case MANAGER_ERR_NULL:
                sprintf(err, "0x%.8X (%d)\nAn unknown error has occurred. Please restart the app.", manager->error, manager->error);
                break;

/// --------------------------------------------------------------------
            case MANAGER_WARN_SETTINGS: // Not used or deprecated
                sprintf(err, "0x%.8X (%d) ", manager->error, manager->error);
                break;
            case MANAGER_WARN_PARSE_SETTINGS:
                sprintf(err, "0x%.8X (%d)\nFailed to parse configuration file. Some settings may be ignored.", manager->error, manager->error);
                break;
            case MANAGER_WARN_GIFDESK_ALLOC:
                sprintf(err, "0x%.8X (%d)\nCould not allocate memory for animation descriptor.", manager->error, manager->error);
                break;
            case MANAGER_WARN_FILE_NEXIST:
                sprintf(err, "0x%.8X (%d)\nSelected file does not exist.", manager->error, manager->error);
                break;
            case MANAGER_WARN_NO_FORMAT:
                sprintf(err, "0x%.8X (%d)\nUnsupported or unknown image format.", manager->error, manager->error);
                break;
            case MANAGER_WARN_CREATE_WINDOW:
                sprintf(err, "0x%.8X (%d)\nCould not create an animation window.", manager->error, manager->error);
                break;
            case MANAGER_WARN_FRAME_ALLOC:
                sprintf(err, "0x%.8X (%d)\nCould not allocate memory for a frame buffer. Please try again later.", manager->error, manager->error);
                break;
            case MANAGER_WARN_BUFF_ALLOC:
                sprintf(err, "0x%.8X (%d)\nCould not allocate memory for a frame buffer. Please try again later.", manager->error, manager->error);
                break;
            case MANAGER_WARN_DELAYS_ALLOC:
                sprintf(err, "0x%.8X (%d)\nFailed to allocate memory for a delays buffer. Please try again later.", manager->error, manager->error);
                break;
            case MANAGER_WARN_LENGTHS_ALLOC:
                sprintf(err, "0x%.8X (%d)\nFailed to allocate memory for a frame buffer. Please try again later.", manager->error, manager->error);
                break;
            case MANAGER_WARN_FRAMEP_ALLOC:
                sprintf(err, "0x%.8X (%d)\nFailed to allocate memory for a frame points buffer. Please try again later.", manager->error, manager->error);
                break;
            case MANAGER_WARN_TEXTURES_ALLOC:
                sprintf(err, "0x%.8X (%d)\nFailed to allocate memory for a animation textures. Please try again later.", manager->error, manager->error);
                break;
            case MANAGER_WARN_CREATE_OBJ:
                sprintf(err, "0x%.8X (%d)\nFailed to create an UI object for animation. Please try again later.", manager->error, manager->error);
                break;
            case MANAGER_WARN_GFKLOOP_NULL:
                sprintf(err, "0x%.8X (%d)\nFailed to create render loop. Handle is invalid. Please try again later.", manager->error, manager->error);
                break;
            case MANAGER_WARN_CREATE_LOOP:
                sprintf(err, "0x%.8X (%d)\nFailed to create render loop. Handle is invalid. Please try again later.", manager->error, manager->error);
                break;
            case MANAGER_WARN_CREATE_THREAD:
                sprintf(err, "0x%.8X (%d)\nFailed to create render thread. Handle is invalid. Please try again later.", manager->error, manager->error);
                break;
            case MANAGER_WARN_QUERY_TOO_MUCH:
                sprintf(err, "0x%.8X (%d)\nYou can select no more than one file at a time.", manager->error, manager->error);
                break;
            case MANAGER_WARN_REG:
                sprintf(err, "0x%.8X (%d)\nFailed to access or modify Windows registry. Please restart the application with administrator rights.", manager->error, manager->error);
                break;
            case MANAGER_WARN_OBJCOUNT_OVERFLOW: // Not used or deprecated
                sprintf(err, "0x%.8X (%d) ", manager->error, manager->error);
                break;
            case MANAGER_WARN_APPLY_WINDOW:
                sprintf(err, "0x%.8X (%d)\nFailed to create an UI object for animation.\nSeems you've reached the file limit.", manager->error, manager->error);
                break;
/// --------------------------------------------------------------------
            case MANAGER_WARN_OPENGL_NA: // Not used or deprecated
                sprintf(err, "0x%.8X (%d) ", manager->error, manager->error);
                break;
            case MANAGER_WARN_OPENGL_INVALID_ENUM:
                sprintf(err, "0x%.8X (%d)\nOpenGL error: invalid enumerator used.", manager->error, manager->error);
                break;
            case MANAGER_WARN_OPENGL_INVALID_VALUE:
                sprintf(err, "0x%.8X (%d)\nOpenGL error: invalid numeric value passed.", manager->error, manager->error);
                break;
            case MANAGER_WARN_OPENGL_INVALID_OPERATION:
                sprintf(err, "0x%.8X (%d)\nOpenGL error: invalid operation in current state.", manager->error, manager->error);
                break;
            case MANAGER_WARN_OPENGL_STACK_OVERFLOW:
                sprintf(err, "0x%.8X (%d)\nOpenGL error: stack overflow.", manager->error, manager->error);
                break;
            case MANAGER_WARN_OPENGL_STACK_UNDERFLOW:
                sprintf(err, "0x%.8X (%d)\nOpenGL error: stack underflow.", manager->error, manager->error);
                break;
            case MANAGER_WARN_OPENGL_OUT_OF_MEMORY:
                sprintf(err, "0x%.8X (%d)\nOpenGL error: out of graphics memory.", manager->error, manager->error);
                break;
            case MANAGER_WARN_OPENGL_OUT_OF_CONTEXT:
                sprintf(err, "0x%.8X (%d)\nOpenGL error: unexpected error.", manager->error, manager->error);
                break;
/// --------------------------------------------------------------------
            case MANAGER_WARN_GIF_OPEN_FAILED:
                sprintf(err, "0x%.8X (%d)\nFailed to open GIF file. Please try to open another file.", manager->error, manager->error);
                break;
            case MANAGER_WARN_GIF_READ_FAILED:
                sprintf(err, "0x%.8X (%d)\nFailed to read GIF data. Please try to open another file.", manager->error, manager->error);
                break;
            case MANAGER_WARN_GIF_NOT_GIF_FILE:
                sprintf(err, "0x%.8X (%d)\nFile is not a valid GIF image. Please try to open another file.", manager->error, manager->error);
                break;
            case MANAGER_WARN_GIF_NO_SCRN_DSCR:
                sprintf(err, "0x%.8X (%d)\nGIF has no screen descriptor. File is corrupted. Please try to open another file.", manager->error, manager->error);
                break;
            case MANAGER_WARN_GIF_NO_IMAG_DSCR:
                sprintf(err, "0x%.8X (%d)\nGIF has no image descriptor. File is corrupted. Please try to open another file.", manager->error, manager->error);
                break;
            case MANAGER_WARN_GIF_NO_COLOR_MAP:
                sprintf(err, "0x%.8X (%d)\nGIF missing required color map. Please try to open another file.", manager->error, manager->error);
                break;
            case MANAGER_WARN_GIF_WRONG_RECORD:
                sprintf(err, "0x%.8X (%d)\nUnexpected record type in GIF stream. Please try to open another file.", manager->error, manager->error);
                break;
            case MANAGER_WARN_GIF_DATA_TOO_BIG:
                sprintf(err, "0x%.8X (%d)\nGIF data block exceeds supported size. Please try to open another file.", manager->error, manager->error);
                break;
            case MANAGER_WARN_GIF_NOT_ENOUGH_MEM:
                sprintf(err, "0x%.8X (%d)\nNot enough memory to decode GIF. Please try to open another file.", manager->error, manager->error);
                break;
            case MANAGER_WARN_GIF_CLOSE_FAILED:
                sprintf(err, "0x%.8X (%d)\nFailed to close GIF file properly. Please try to open another file.", manager->error, manager->error);
                break;
            case MANAGER_WARN_GIF_NOT_READABLE:
                sprintf(err, "0x%.8X (%d)\nCannot read GIF file. Please try to open another file.", manager->error, manager->error);
                break;
            case MANAGER_WARN_GIF_IMAGE_DEFECT:
                sprintf(err, "0x%.8X (%d)\nGIF is corrupted. Please try to open another file.", manager->error, manager->error);
                break;
            case MANAGER_WARN_GIF_EOF_TOO_SOON:
                sprintf(err, "0x%.8X (%d)\nUnexpected end of file while reading GIF. Please try to open another file.", manager->error, manager->error);
                break;
            case MANAGER_WARN_GIF_NULL:
                sprintf(err, "0x%.8X (%d)\nInternal GIF structure is null. Please try to open another file.", manager->error, manager->error);
                break;
/// --------------------------------------------------------------------
            case MANAGER_WARN_APNG_STRUCT:
                sprintf(err, "0x%.8X (%d)\nFailed to initialize APNG structure. Please try again later.", manager->error, manager->error);
                break;
            case MANAGER_WARN_APNG_INFO:
                sprintf(err, "0x%.8X (%d)\nFailed to retrieve APNG image information.", manager->error, manager->error);
                break;
            case MANAGER_WARN_APNG_JUMP:
                sprintf(err, "0x%.8X (%d)\nFailed to setjmp to jmpbuf. Please try again later.", manager->error, manager->error);
                break;
            case MANAGER_WARN_APNG_POINTERS:
                sprintf(err, "0x%.8X (%d)\nFailed to allocate memory for an image rows. Please try again later.", manager->error, manager->error);
                break;
            case MANAGER_WARN_APNG_POINTER:
                sprintf(err, "0x%.8X (%d)\nFailed to allocate memory for an image row bytes. Please try again later.", manager->error, manager->error);
                break;
/// --------------------------------------------------------------------
            case MANAGER_WARN_WEBP_INFO:
                sprintf(err, "0x%.8X (%d)\nFailed to retrieve WebP image info.", manager->error, manager->error);
                break;
            case MANAGER_WARN_WEBP_OPTIONS:
                sprintf(err, "0x%.8X (%d)\nInvalid WebP decoding options.", manager->error, manager->error);
                break;
            case MANAGER_WARN_WEBP_OUT_OF_MEMORY:
                sprintf(err, "0x%.8X (%d)\nDecoder ran out of memory. Please try again later.", manager->error, manager->error);
                break;
            case MANAGER_WARN_WEBP_INVALID_PARAM:
                sprintf(err, "0x%.8X (%d)\nInvalid parameter passed to decoder.", manager->error, manager->error);
                break;
            case MANAGER_WARN_WEBP_BITSTREAM_ERROR:
                sprintf(err, "0x%.8X (%d)\nCorrupted or invalid WebP bitstream.", manager->error, manager->error);
                break;
            case MANAGER_WARN_WEBP_UNSUPPORTED_FEATURE:
                sprintf(err, "0x%.8X (%d)\nWebP feature not supported by this decoder.", manager->error, manager->error);
                break;
            case MANAGER_WARN_WEBP_SUSPENDED:
                sprintf(err, "0x%.8X (%d)\nWebP decoding was suspended.", manager->error, manager->error);
                break;
            case MANAGER_WARN_WEBP_USER_ABORT:
                sprintf(err, "0x%.8X (%d)\nWebP decoding was aborted by something else.", manager->error, manager->error);
                break;
            case MANAGER_WARN_WEBP_NOT_ENOUGH_DATA:
                sprintf(err, "0x%.8X (%d)\nNot enough data to decode WebP image.", manager->error, manager->error);
                break;
            case MANAGER_WARN_WEBP_UNKNOWN:
                sprintf(err, "0x%.8X (%d)\nUnknown WebP decoding error.", manager->error, manager->error);
                break;
            case MANAGER_WARN_WEBP_DECODER:
                sprintf(err, "0x%.8X (%d)\nGeneral WebP decoder failure.", manager->error, manager->error);
                break;
/// --------------------------------------------------------------------
            default:
                break;
        }
        if (manager->error < 0x2000) {
            MessageBox(NULL, err, APP_NAME_VER, MB_ICONERROR);
            ManagerDestroy(manager);
        }
        else {
            MessageBox(NULL, err, APP_NAME_VER, MB_ICONWARNING);
            manager->is_loading = 0;
            DragAcceptFiles(manager->window, TRUE);
            return;
        }
    }
}

#endif // TYPES_H_INCLUDED

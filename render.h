#ifndef RENDER_H_INCLUDED
#define RENDER_H_INCLUDED

#include "types.h"
#include "window.h"
#include "settings.h"

#define BTN_TEXT_COLOR RGB(30, 30, 30)
#define BTN_MARLETT_COLOR RGB(90, 90, 90)
 #define BTN_HOVERED RGB(242, 242, 242)
//#define BTN_HOVERED RGB(190, 190, 190)
#define BTN_NOT_HOVERED RGB(250, 250, 250)

/// for 2560x1440 res
/// minimal height: 36

#define POPUP_WIDTH 250
#define POPUP_HEIGHT 206

#define PB_WIDTH 225
#define PB_HEIGHT 117

#define IM_WIDTH 225
#define IM_HEIGHT 36

#define WC_WIDTH 225
#define WC_HEIGHT 92

#define PW_WIDTH 225
#define PW_HEIGHT 62

#define MWT_WIDTH 225
#define MWT_HEIGHT 137

#define MAIN_WINDOW 1, 1, 1, 1
#define POPUP_MENU 0, 1, 0, 0

#define WM_UPDATE_ALPHA (WM_USER + 100)

extern HANDLE thread;

typedef struct
{
    Window* window;
    Window* window_popup;

    Window* window_wc;
    Window* window_pb;
    Window* window_im;
    Window* window_pw;
    Window* window_mwt;

    Window* window_debug;

    /** Popup Menu **/

    Button* btn_title;
    Button* btn_openfile;

    Button* label_window_scale;

    Button* label_scale;
    Button* btn_add_scale;
    Button* btn_subtract_scale;

    Trackbar* trackbar_scale;

    Button* label_playback;

    Button* label_frames;
    Button* btn_prev_frame;
    Button* btn_play;
    Button* btn_next_frame;

    Trackbar* trackbar_frames;

    Button* label_speed;
    Button* btn_slow_rewind;
    Button* btn_fast_rewind;
    Button* btn_slow_wind;
    Button* btn_fast_wind;

    Trackbar* trackbar_speed;

    Button* label_transparency;

    Trackbar* trackbar_transparency;

    Button* btn_taskbar;

    Button* btn_frame_updates;

    Button* label_interaction;

    Button* btn_ignore_input;

    Button* label_pin_window;

    Button* btn_pin_default;
    Button* btn_pin_top;
    Button* btn_pin_bottom;

    Button* label_move_window;

    Button* btn_move_topleft;
    Button* btn_move_topright;
    Button* btn_move_center;
    Button* btn_move_left;
    Button* btn_move_right;

    Button* btn_close_window;

    Data* dt;
    Render* rd;
    Settings* st;

} RenderPtr;

extern RenderPtr rptr;

void RptrInit(RenderPtr* rptr, Settings* st, Data* dt, Render* rd,
              Window* window, Window* window_popup, Window* window_debug,
              Window* window_wc, Window* window_pb, Window* window_im, Window* window_pw, Window* window_mwt,

              Trackbar* trackbar_scale, Trackbar* trackbar_frames, Trackbar* trackbar_speed, Trackbar* trackbar_transparency,

              Button* btn_title, Button* btn_openfile, Button* label_window_scale, Button* label_scale, Button* btn_add_scale,
              Button* btn_subtract_scale, Button* label_playback, Button* label_frames, Button* btn_prev_frame, Button* btn_play,
              Button* btn_next_frame, Button* label_speed, Button* btn_slow_rewind, Button* btn_fast_rewind, Button* btn_slow_wind,
              Button* btn_fast_wind, Button* label_transparency, Button* btn_frame_updates,
              Button* label_interaction, Button* label_ignore_input, Button* label_pin_window, Button* btn_pin_default,
              Button* btn_pin_top, Button* btn_pin_bottom, Button* label_move_window, Button* btn_move_topleft,
              Button* btn_move_topright, Button* btn_move_center, Button* btn_move_left, Button* btn_move_right, Button* btn_close_window,
              Button* btn_taskbar);

void Loop(RenderPtr* rptr);

/** **/

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK BusyWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK PopupMenuProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK PBProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); /// Playback
LRESULT CALLBACK IMProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); /// Interaction
LRESULT CALLBACK WCProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); /// Window
LRESULT CALLBACK PWProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); /// Pin window
LRESULT CALLBACK MWTProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); /// Move window to

void ReleaseHover(RenderPtr* rptr, HWND hwnd);
void _InvalidateButton(LPDRAWITEMSTRUCT item, Button* button,
                       const char* text, int left, int activated, int arrow);
/// void _InvalidateTrackBar(LPDRAWITEMSTRUCT item, Window* window, Window* cf_window);

int _IsButtonHovered(Button* button, POINT* p, int arrowed);
/// void _IsTrackBarHovered(Window* window, Window* cf_window, POINT* p);

int _GetCollisionSize(int n, float size);

void _ChangeScaleTrackBar(Window* window, Window* window_popup,
                          Trackbar* scale_trackbar, Button* scale_button,
                          Settings* st, Data* dt,
                          Render* rd, int pos);

/**
        OpenGL Proccessing
**/

void ShowFrame(Window* window, Data* dt, Render* rd, Settings* st);
void ShowLoadLine(Window* window, Data* dt, Settings* st, float pt);

DWORD WINAPI RenderThread(LPVOID arg);
void* ShowPopupThread(LPVOID arg);
void* ShowLowerPopupThread(LPVOID arg);

/**
        Time Proccessing
**/

double GetTime();
uint8_t ChangeFrame(Data* dt, Render* rd);

#endif // RENDER_H_INCLUDED

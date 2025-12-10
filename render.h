#ifndef RENDER_H_INCLUDED
#define RENDER_H_INCLUDED

#include "types.h"
#include "window.h"
#include "settings.h"

/// for 2560x1440 res

#define POPUP_WIDTH 250
#define POPUP_HEIGHT 400

#define MWT_WIDTH 225
#define MWT_HEIGHT 137

#define MAIN_WINDOW 1, 1, 1, 1
#define POPUP_MENU 0, 1, 0, 0

#define WM_UPDATE_ALPHA (WM_USER + 100)

extern HANDLE thread;

typedef struct
{
    /** windows **/
    Window* window;
    Window* popup_window;
    Window* debug_window;
    Window* mwt_window;

    /** trackbars **/
    Trackbar* scale_trackbar;
    Trackbar* frames_trackbar;

    /** buttons **/
    Button* title_button;
    Button* openfile_button;
    Button* scale_button;
    Button* addscale_button;
    Button* decscale_button;
    Button* pause_button;
    Button* sfp_button;
    Button* sti_button;
    Button* aot_button;

    Button* mwt_button;
    Button* tlc_button;
    Button* trc_button;
    Button* cnr_button;
    Button* blc_button;
    Button* brc_button;

    Button* language_button;
    Button* exit_button;

    Data* dt;
    Render* rd;
    Settings* st;

} RenderPtr;

extern RenderPtr rptr;

void RptrInit(RenderPtr* rptr, Settings* st, Data* dt, Render* rd,
              Window* window, Window* popup_window, Window* debug_window, Window* mwt_window,

              Trackbar* scale_trackbar, Trackbar* frames_trackbar,

              Button* title_button, Button* openfile_button, Button* scale_button,
              Button* addscale_button, Button* decscale_button, Button* pause_button,
              Button* sfp_button, Button* sti_button, Button* aot_button, Button* mwt_button,
              Button* tlc_button, Button* trc_button, Button* cnr_button, Button* blc_button,
              Button* brc_button, Button* language_button, Button* exit_button);

void Loop(RenderPtr* rptr);

/** **/

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK BusyWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK PopupMenuProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK MWTProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

void _InvalidateButton(LPDRAWITEMSTRUCT item, Button* button,
                       const char* text, int left, int activated, int arrow);
/// void _InvalidateTrackBar(LPDRAWITEMSTRUCT item, Window* window, Window* cf_window);

int _IsButtonHovered(Button* button, POINT* p, int arrowed);
/// void _IsTrackBarHovered(Window* window, Window* cf_window, POINT* p);

int _GetCollisionSize(int n, float size);

void _ChangeScaleTrackBar(Window* window, Window* popup_window,
                          Trackbar* scale_trackbar, Button* scale_button,
                          Settings* st, Data* dt,
                          Render* rd, int pos);

/** **/

void ShowFrame(Window* window, Data* dt, Render* rd, Settings* st);
void ShowLoadLine(Window* window, Data* dt, Settings* st, float pt);

DWORD WINAPI RenderThread(LPVOID arg);
void* ShowThread(LPVOID arg);

/** **/

double GetTime();
uint8_t ChangeFrame(Data* dt, Render* rd);

#endif // RENDER_H_INCLUDED

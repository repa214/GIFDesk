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
    Window* window;
    Window* popup_window;
    Window* debug_window;

    /** trackbars **/
    Window* scale_trackbar;
    Window* frames_trackbar;

    /** buttons **/
    Window* title_button;
    Window* openfile_button;
    Window* scale_button;
    Window* addscale_button;
    Window* decscale_button;
    Window* pause_button;
    Window* sfp_button;
    Window* sti_button;
    Window* aot_button;

    Window* mwt_button;
    Window* tlc_button;
    Window* trc_button;
    Window* cnr_button;
    Window* blc_button;
    Window* brc_button;

    Window* language_button;

    Window* exit_button;

    Window* mwt_window;

    Data* dt;
    Render* rd;
    Settings* st;

} RenderPtr;

extern RenderPtr rptr;

void RptrInit(RenderPtr* rptr, Settings* st, Data* dt, Render* rd,
              Window* window, Window* popup_window, Window* debug_window,

              Window* scale_trackbar, Window* frames_trackbar,

              Window* title_button, Window* openfile_button, Window* scale_button,
              Window* addscale_button, Window* decscale_button, Window* pause_button,
              Window* sfp_button, Window* sti_button, Window* aot_button, Window* mwt_button,
              Window* tlc_button, Window* trc_button, Window* cnr_button, Window* blc_button,
              Window* brc_button, Window* language_button, Window* exit_button, Window* mwt_window);

void Loop(RenderPtr* rptr);

/** **/

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK BusyWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK PopupMenuProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK MWTProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

void _InvalidateButton(LPDRAWITEMSTRUCT item, Window* window,
                       const char* text, int left, int activated, int arrow);
void _InvalidateTrackBar(LPDRAWITEMSTRUCT item, Window* window, Window* cf_window);

int _IsButtonHovered(Window* window, POINT* p, int arrowed);
void _IsTrackBarHovered(Window* window, Window* cf_window, POINT* p);

int _GetCollisionSize(int n, float size);

void _ChangeScaleTrackBar(Window* window, Window* popup_window,
                          Window* scale_trackbar, Window* scale_button,
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

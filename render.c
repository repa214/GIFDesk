#include "render.h"

float vertex[] = {-1, -1, 0,
                   1, -1, 0,
                   1,  1, 0,
                  -1,  1, 0};
float texCoord[] = {0, 1,
                    1, 1,
                    1, 0,
                    0, 0};

RenderPtr rptr;
HANDLE thread;
pthread_t thread_t;
MSG msg;

void RptrInit(RenderPtr* rptr, Settings* st, Data* dt, Render* rd,
              Window* window, Window* window_popup, Window* window_debug,
              Window* window_wc, Window* window_pb, Window* window_im, Window* window_pw, Window* window_mwt,

              Trackbar* trackbar_scale, Trackbar* trackbar_frames, Trackbar* trackbar_speed, Trackbar* trackbar_transparency,

              Button* btn_title, Button* btn_openfile, Button* label_window_scale, Button* label_scale, Button* btn_add_scale,
              Button* btn_subtract_scale, Button* label_playback, Button* label_frames, Button* btn_prev_frame, Button* btn_play,
              Button* btn_next_frame, Button* label_speed, Button* btn_slow_rewind, Button* btn_fast_rewind, Button* btn_slow_wind,
              Button* btn_fast_wind, Button* label_transparency, Button* btn_frame_updates,
              Button* label_interaction, Button* btn_ignore_input, Button* label_pin_window, Button* btn_pin_default,
              Button* btn_pin_top, Button* btn_pin_bottom, Button* label_move_window, Button* btn_move_topleft,
              Button* btn_move_topright, Button* btn_move_center, Button* btn_move_left, Button* btn_move_right, Button* btn_close_window,
              Button* btn_taskbar)
{
    rptr->window = window;
    rptr->window_popup = window_popup;
    rptr->window_debug = window_debug;
    rptr->window_wc = window_wc;
    rptr->window_pb = window_pb;
    rptr->window_im = window_im;
    rptr->window_pw = window_pw;
    rptr->window_mwt = window_mwt;

    /** trackbars **/

    rptr->trackbar_scale = trackbar_scale;
    rptr->trackbar_frames = trackbar_frames;
    rptr->trackbar_speed = trackbar_speed;
    rptr->trackbar_transparency = trackbar_transparency;

    /** buttons **/

    rptr->btn_openfile = btn_openfile;
    rptr->btn_title = btn_title;
    rptr->label_window_scale = label_window_scale;
    rptr->label_scale = label_scale;
    rptr->btn_add_scale = btn_add_scale;
    rptr->btn_subtract_scale = btn_subtract_scale;
    rptr->label_playback = label_playback;
    rptr->label_frames = label_frames;
    rptr->btn_prev_frame = btn_prev_frame;
    rptr->btn_play = btn_play;
    rptr->btn_next_frame = btn_next_frame;
    rptr->label_speed = label_speed;
    rptr->btn_slow_rewind = btn_slow_rewind;
    rptr->btn_fast_rewind = btn_fast_rewind;
    rptr->btn_slow_wind = btn_slow_wind;
    rptr->btn_fast_wind = btn_fast_wind;
    rptr->label_transparency = label_transparency;
    rptr->btn_frame_updates = btn_frame_updates;
    rptr->label_interaction = label_interaction;
    rptr->btn_ignore_input = btn_ignore_input;
    rptr->label_pin_window = label_pin_window;
    rptr->btn_pin_default = btn_pin_default;
    rptr->btn_pin_top = btn_pin_top;
    rptr->btn_pin_bottom = btn_pin_bottom;
    rptr->label_move_window = label_move_window;
    rptr->btn_move_topleft = btn_move_topleft;
    rptr->btn_move_topright = btn_move_topright;
    rptr->btn_move_center = btn_move_center;
    rptr->btn_move_left = btn_move_left;
    rptr->btn_move_right = btn_move_right;
    rptr->btn_close_window = btn_close_window;
    rptr->btn_taskbar = btn_taskbar;
    ReleaseHover(rptr, NULL);

    rptr->dt = dt;
    rptr->rd = rd;
    rptr->st = st;
}

/// Main Loop

void Loop(RenderPtr* rptr)
{
    MSG msg;
    rptr->rd->frame = 0;
    rptr->rd->render_thread = 1;
    rptr->rd->start_time = GetTime();
    rptr->rd->change_frames = 1;
    rptr->rd->inaccuracy = 0;
    rptr->rd->framed_trackbar = 0;
    rptr->rd->loading = 0;

    rptr->dt->prev_mode = 0;
    rptr->dt->prev_frxo = 0;
    rptr->dt->prev_fryo = 0;
    rptr->dt->prev_frxd = 0;
    rptr->dt->prev_fryd = 0;

    rptr->st->sfu = 0;

    /// Main Loop

    while (IsWindow(rptr->window->hwnd)) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!IsWindow(rptr->window->hwnd)) continue;

        if (!rptr->rd->loading && ChangeFrame(rptr->dt, rptr->rd)) ShowFrame(rptr->window, rptr->dt, rptr->rd, rptr->st);
        Sleep(1);
    }

}

/**
        Windows Proccessing
**/

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static POINT p;
    static RECT rect, res;
    static HRGN hrgn = NULL;

    switch (msg)
    {
        /// -------------------
        case WM_CLOSE: {
            if (IsWindow(rptr.window->hwnd))
                DestroyWindow(rptr.window->hwnd);
        }   break;

        /// -------------------
        case WM_DROPFILES: {
            DragAcceptFiles(hwnd, FALSE);

            _LoadDropFile((HDROP)wparam, rptr.window, rptr.st, rptr.dt, rptr.rd);

            DragAcceptFiles(hwnd, TRUE);
        }   break;

        /// -------------------
        case WM_LBUTTONDOWN: {
            thread = CreateThread(
                        NULL,
                        0,
                        RenderThread,
                        &rptr,
                        0,
                        NULL
                    );

            if (!rptr.window_popup->isactive) SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

            rptr.rd->render_thread = 0;
            WaitForSingleObject(thread, INFINITE); CloseHandle(thread);
            rptr.rd->render_thread = 1;
        }   break;

        /// -------------------
        case WM_RBUTTONDOWN: {
            ReleaseWindow(rptr.window_popup);
            ReleaseHover(&rptr, NULL);

            GetCursorPos(&p);
            SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

            if (p.x + POPUP_WIDTH > rect.right - rect.left) { p.x -= POPUP_WIDTH; }
            if (p.y + POPUP_HEIGHT > rect.bottom - rect.top) { p.y -= POPUP_HEIGHT; }

            /** Resize Window **/

            texCoord[1] = 2 / ((rptr.st->size > 2) ? 2 : rptr.st->size);
            texCoord[2] = 2 / ((rptr.st->size > 2) ? 2 : rptr.st->size);
            texCoord[3] = 2 / ((rptr.st->size > 2) ? 2 : rptr.st->size);
            texCoord[4] = 2 / ((rptr.st->size > 2) ? 2 : rptr.st->size);

            rptr.st->trackbar_size = rptr.st->size; rptr.st->size = 2;

            SetWindowPos(rptr.window->hwnd,
                         HWND_NOTOPMOST,
                         0,
                         0,
                         _GetCollisionSize(rptr.dt->width, rptr.st->size),
                         _GetCollisionSize(rptr.dt->height, rptr.st->size),
                         SWP_NOMOVE);

            DragAcceptFiles(rptr.window->hwnd, FALSE);

            thread = CreateThread(
                        NULL,
                        0,
                        RenderThread,
                        &rptr,
                        0,
                        NULL
                    );

            /** Popup Menu **/

            LoadWindow(rptr.window_popup, rptr.st, NULL,
                       "window_popup", p.x + 5, p.y + 5,
                       1, 1, POPUP_MENU);

            /** Buttons **/

            int y = 5;
            LoadButton(rptr.btn_title, rptr.window_popup,
                       5, y, POPUP_WIDTH - 10, 25, 15,
                       " ", 1, "Segoe UI");

            LoadButton(rptr.btn_openfile, rptr.window_popup,
                       5, y += 34, POPUP_WIDTH - 10, 25, 15,
                       " ", 2, "Segoe UI");

            LoadButton(rptr.label_playback, rptr.window_popup,
                       5, y += 25, POPUP_WIDTH - 10, 25, 15,
                       " ", 3, "Segoe UI");

//            LoadButton(rptr.label_interaction, rptr.window_popup,
//                       5, y += 25, POPUP_WIDTH - 10, 25, 15,
//                       " ", 4);

            LoadButton(rptr.label_window_scale, rptr.window_popup,
                       5, y += 25, POPUP_WIDTH - 10, 25, 15,
                       " ", 5, "Segoe UI");

            LoadButton(rptr.label_pin_window, rptr.window_popup,
                       5, y += 25, POPUP_WIDTH - 10, 25, 15,
                       " ", 6, "Segoe UI");

            LoadButton(rptr.label_move_window, rptr.window_popup,
                       5, y += 25, POPUP_WIDTH - 10, 25, 15,
                       " ", 7, "Segoe UI");

            LoadButton(rptr.btn_close_window, rptr.window_popup,
                       5, y += 34, POPUP_WIDTH - 10, 25, 15,
                       " ", 8, "Segoe UI");

            SetWindowPos(rptr.window_popup->hwnd,
                         NULL,
                         0,
                         0,
                         POPUP_WIDTH,
                         POPUP_HEIGHT,
                         SWP_NOMOVE);

            hrgn = CreateRoundRectRgn(0, 0,
                                      POPUP_WIDTH, POPUP_HEIGHT,
                                      10, 10);

            SetWindowRgn(rptr.window_popup->hwnd, hrgn, TRUE);

            rptr.rd->render_thread = 0;
            WaitForSingleObject(thread, INFINITE); CloseHandle(thread);
            rptr.rd->render_thread = 1;
        }   break;

        /// -------------------
        case WM_COMMAND: {
            switch (wparam) {
                /// -------------------
                case 1:
                    thread = CreateThread(
                        NULL,
                        0,
                        _LoadSettings,
                        &rptr,
                        0,
                        NULL
                    );

                    CloseHandle(thread);
                    break;
            }
        }   break;

        /// -------------------
        case WM_USER: {
            switch (wparam) {
                case 2:
                    SetWindowPos(rptr.window->hwnd,
                             HWND_NOTOPMOST,
                             0,
                             0,
                             0,
                             0,
                             SWP_NOMOVE | SWP_NOSIZE);
                    break;
                case 3:
                    printf("WM_USER: 3 |");

                    SetWindowPos(rptr.window->hwnd,
                             (rptr.st->topmost) ? HWND_TOPMOST : HWND_NOTOPMOST,
                             0,
                             0,
                             _GetCollisionSize(rptr.dt->width, rptr.st->size),
                             _GetCollisionSize(rptr.dt->height, rptr.st->size),
                             SWP_NOMOVE);
                    break;
                case 4:
                    printf("WM_USER: 4 |");
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);

                    if (_GetCollisionSize(rptr.dt->width, rptr.st->size) > res.right / 2)
                        rptr.st->size = (float)res.right / 2 / (float)rptr.dt->width;

                    if (_GetCollisionSize(rptr.dt->height, rptr.st->size) > res.bottom / 2)
                        rptr.st->size = (float)res.bottom / 2 / (float)rptr.dt->height;

                    SetWindowPos(rptr.window->hwnd,
                             (rptr.st->topmost) ? HWND_TOPMOST : HWND_NOTOPMOST,
                             0,
                             0,
                             _GetCollisionSize(rptr.dt->width, rptr.st->size),
                             _GetCollisionSize(rptr.dt->height, rptr.st->size),
                             SWP_NOMOVE);
            }
        }   break;

        /// -------------------
        case WM_KEYDOWN: {
            switch (wparam) {
                case VK_SPACE:
                    if (rptr.rd->change_frames) rptr.rd->change_frames = 0;
                    else rptr.rd->change_frames = 1;
                    break;
                case VK_ESCAPE:
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
            }
        }   break;

        default: break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK BusyWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK PopupMenuProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static POINT p;
    static RECT rect, wcrect, pbrect, res;
    static HBRUSH brush = NULL;
    static PAINTSTRUCT ps;
    static HDC hdc = NULL;
    static HPEN pen = NULL;
    static HPEN prev_pen = NULL;
    static HBRUSH prev_brush = NULL;

    switch (msg)
    {
        /// -------------------
        case WM_CREATE: {
            pthread_create(&thread_t, 0, ShowPopupThread, hwnd);
        }   break;

        /// -------------------
        case WM_DESTROY: {
            rptr.rd->render_thread = 0;
            WaitForSingleObject(thread, INFINITE); CloseHandle(thread);
            rptr.rd->render_thread = 1;

            rptr.st->size = rptr.st->trackbar_size;

            SetWindowPos(rptr.window->hwnd,
                         (rptr.st->topmost) ? HWND_TOPMOST : HWND_NOTOPMOST,
                         0, 0,
                         _GetCollisionSize(rptr.dt->width, rptr.st->size),
                         _GetCollisionSize(rptr.dt->height, rptr.st->size),
                         SWP_NOMOVE);

            texCoord[1] = 1;
            texCoord[2] = 1;
            texCoord[3] = 1;
            texCoord[4] = 1;

            DragAcceptFiles(rptr.window->hwnd, TRUE);
            WriteSettings(rptr.st);

            rptr.window_popup->isactive = 0;
            PostQuitMessage(0);
        }   break;

        /// -------------------
        case WM_CLOSE: {
            DestroyWindow(hwnd);
        }   break;

        /// -------------------
        case WM_ACTIVATE: {

            GetCursorPos(&p);
            GetWindowRect(rptr.window_popup->hwnd, &rect);
            GetWindowRect(rptr.window_wc->hwnd, &wcrect);
            GetWindowRect(rptr.window_pb->hwnd, &pbrect);

            if (wparam != WA_INACTIVE ||
                IsChild(rptr.window_popup->hwnd, (HWND)lparam) ||
                IsChild(rptr.window_wc->hwnd, (HWND)lparam) ||
                (HWND)lparam == hwnd ||
                PtInRect(&rect, p) ||
                PtInRect(&wcrect, p) ||
                PtInRect(&pbrect, p)) { break; }

            PostMessage(rptr.window_popup->hwnd, WM_CLOSE, 0, 0);

            if (rptr.window_pw->isactive)
                PostMessage(rptr.window_pw->hwnd, WM_CLOSE, 0, 0);
            if (rptr.window_mwt->isactive)
                PostMessage(rptr.window_mwt->hwnd, WM_CLOSE, 0, 0);
        }   break;

        /// -------------------
        case WM_LBUTTONDOWN: {
            thread = CreateThread(
                        NULL,
                        0,
                        RenderThread,
                        &rptr,
                        0,
                        NULL
                    );

            SetFocus(rptr.window_popup->hwnd);
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

            rptr.rd->render_thread = 0;
            WaitForSingleObject(thread, INFINITE);
            rptr.rd->render_thread = 1;
        }   break;

        /// -------------------
        case WM_RBUTTONDOWN: {
            ReleaseWindow(rptr.window_popup);

            SendMessage(rptr.window->hwnd, WM_RBUTTONDOWN, HTCAPTION, 0);
        }   break;

        /// -------------------
        case WM_SETCURSOR: {
            GetCursorPos(&p);

        /// int _IsButtonHovered(Button* button, POINT* p, int arrowed)
            _IsButtonHovered(rptr.btn_title, &p, 0);
            _IsButtonHovered(rptr.btn_openfile, &p, 0);

            if (_IsButtonHovered(rptr.label_playback, &p, 1)) {
                if (rptr.window_pb->isactive == 0) {
                    GetWindowRect(rptr.window_popup->hwnd, &rect);

                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);
                    int offset = 59;
                    int popup_left = rect.left + POPUP_WIDTH;
                    int popup_top = rect.top + offset;
                    if (popup_left + PB_WIDTH > res.right) popup_left = rect.left - PB_WIDTH;
                    if (popup_top + PB_HEIGHT > res.bottom) popup_top = res.bottom - PB_HEIGHT;

                    LoadWindow(rptr.window_pb, rptr.st, NULL, "window_pb", popup_left, popup_top, PB_WIDTH, PB_HEIGHT, 0, 0, 0, 0);

                    HRGN rgn = CreateRoundRectRgn(0, 0, PB_WIDTH, PB_HEIGHT, 5, 5);
                    SetWindowRgn(rptr.window_pb->hwnd, rgn, TRUE);

                    int y = 5;

                    LoadButton(rptr.label_frames, rptr.window_pb,
                               4, y, PB_WIDTH - 10, 25, 15,
                               " ", 1, "Segoe UI");

                    LoadButton(rptr.btn_prev_frame, rptr.window_pb,
                               4, y += 25, 72, 25, 15,
                               " ", 2, "Marlett");

                    LoadButton(rptr.btn_play, rptr.window_pb,
                               76, y, 72, 25, 15,
                               " ", 3, "Marlett");

                    LoadButton(rptr.btn_next_frame, rptr.window_pb,
                               148, y, 72, 25, 15,
                               " ", 4, "Marlett");

                    LoadTrackBar(rptr.trackbar_frames, rptr.window_pb,
                         13, y += 27, PB_WIDTH - 26, 24, 0,
                         1, rptr.dt->count, rptr.rd->frame - 1, 5);

                    LoadButton(rptr.btn_frame_updates, rptr.window_pb,
                               4, y += 28, PB_WIDTH - 10, 25, 15,
                               " ", 6, "Segoe UI");
                }
            }
            else PostMessage(rptr.window_pb->hwnd, WM_USER, 1, 0);

//            if (_IsButtonHovered(rptr.label_interaction, &p, 1)) {
//                if (rptr.window_im->isactive == 0) {
//                    GetWindowRect(rptr.window_popup->hwnd, &rect);
//
//                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);
//                    int popup_left = rect.left + POPUP_WIDTH;
//                    int popup_top = rect.top + 84;
//                    if (rptr.dt->count == 1) popup_top -= 55;
//                    if (popup_left + IM_WIDTH + 5 > res.right) popup_left = rect.left - POPUP_WIDTH + 24;
//                    if (popup_top + IM_HEIGHT + 5 > res.bottom) popup_top = res.bottom - 229;
//
//                    LoadWindow(rptr.window_im, rptr.st, NULL, "window_im", popup_left, popup_top, IM_WIDTH, IM_HEIGHT, 0, 0, 0, 0);
//
//                    HRGN rgn = CreateRoundRectRgn(0, 0, IM_WIDTH, IM_HEIGHT, 5, 5);
//                    SetWindowRgn(rptr.window_im->hwnd, rgn, TRUE);
//
//                    int y = 5;
//
//                    LoadButton(rptr.btn_ignore_input, rptr.window_im,
//                               5, y, IM_WIDTH - 10, 25, 15,
//                               " ", 1);
//                }
//            }
//            else PostMessage(rptr.window_im->hwnd, WM_USER, 1, 0);

            if (_IsButtonHovered(rptr.label_window_scale, &p, 1)) {
                if (rptr.window_wc->isactive == 0) {
                    GetWindowRect(rptr.window_popup->hwnd, &rect);

                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);
                    int offset = 84;
                    int popup_left = rect.left + POPUP_WIDTH;
                    int popup_top = rect.top + offset;
                    if (popup_left + WC_WIDTH > res.right) popup_left = rect.left - WC_WIDTH;
                    if (popup_top + WC_HEIGHT > res.bottom) popup_top = res.bottom - WC_HEIGHT;

                    LoadWindow(rptr.window_wc, rptr.st, NULL, "window_wc", popup_left, popup_top, WC_WIDTH, WC_HEIGHT, 0, 0, 0, 0);

                    HRGN rgn = CreateRoundRectRgn(0, 0, WC_WIDTH, WC_HEIGHT, 5, 5);
                    SetWindowRgn(rptr.window_wc->hwnd, rgn, TRUE);

                    int y = 5;

                    LoadButton(rptr.label_scale, rptr.window_wc,
                               4, y, 150, 25, 15,
                               " ", 1, "Segoe UI");

                    LoadButton(rptr.btn_add_scale, rptr.window_wc,
                               155, y, 30, 25, 15,
                               " ", 2, "Segoe UI");

                    LoadButton(rptr.btn_subtract_scale, rptr.window_wc,
                               190, y, 30, 25, 15,
                               " ", 3, "Segoe UI");

                    LoadTrackBar(rptr.trackbar_scale, rptr.window_wc,
                         13, y += 27, WC_WIDTH - 27, 24, 0,
                         1, 200, (int)(rptr.st->trackbar_size * 100), 4);

                    LoadButton(rptr.btn_taskbar, rptr.window_wc,
                               4, y += 28, WC_WIDTH - 10, 25, 15,
                               " ", 5, "Segoe UI");
                }
            }
            else PostMessage(rptr.window_wc->hwnd, WM_USER, 1, 0);

            if (_IsButtonHovered(rptr.label_pin_window, &p, 1)) {
                if (rptr.window_pw->isactive == 0) {
                    GetWindowRect(rptr.window_popup->hwnd, &rect);

                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);
                    int offset = 109;
                    int popup_left = rect.left + POPUP_WIDTH;
                    int popup_top = rect.top + offset;
                    if (popup_left + PW_WIDTH > res.right) popup_left = rect.left - PW_WIDTH;
                    if (popup_top + PW_HEIGHT > res.bottom) popup_top = res.bottom - PW_HEIGHT;

                    LoadWindow(rptr.window_pw, rptr.st, NULL, "window_pw", popup_left, popup_top, PW_WIDTH, PW_HEIGHT, 0, 0, 0, 0);

                    HRGN rgn = CreateRoundRectRgn(0, 0, PW_WIDTH, PW_HEIGHT, 5, 5);
                    SetWindowRgn(rptr.window_pw->hwnd, rgn, TRUE);

                    int y = 5;
                    LoadButton(rptr.btn_pin_default, rptr.window_pw,
                               4, y, PW_WIDTH - 10, 25, 15,
                               " ", 1, "Segoe UI");

                    LoadButton(rptr.btn_pin_top, rptr.window_pw,
                               4, y += 25, PW_WIDTH - 10, 25, 15,
                               " ", 2, "Segoe UI");
                }
            }
            else PostMessage(rptr.window_pw->hwnd, WM_USER, 1, 0);

            if (_IsButtonHovered(rptr.label_move_window, &p, 1)) {
                if (rptr.window_mwt->isactive == 0)
                {
                    GetWindowRect(rptr.window_popup->hwnd, &rect);

                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);
                    int offset = 134;
                    int popup_left = rect.left + POPUP_WIDTH;
                    int popup_top = rect.top + offset;
                    if (popup_left + MWT_WIDTH > res.right) popup_left = rect.left - MWT_WIDTH;
                    if (popup_top + MWT_HEIGHT > res.bottom) popup_top = res.bottom - MWT_HEIGHT;

                    LoadWindow(rptr.window_mwt, rptr.st, NULL, "window_mwt", popup_left, popup_top, MWT_WIDTH, MWT_HEIGHT, 0, 0, 0, 0);

                    HRGN rgn = CreateRoundRectRgn(0, 0, MWT_WIDTH, MWT_HEIGHT, 5, 5);
                    SetWindowRgn(rptr.window_mwt->hwnd, rgn, TRUE);

                    int y = 5;
                    LoadButton(rptr.btn_move_topleft, rptr.window_mwt,
                               4, y, MWT_WIDTH - 10, 25, 15,
                               " ", 1, "Segoe UI");

                    LoadButton(rptr.btn_move_topright, rptr.window_mwt,
                               4, y += 25, MWT_WIDTH - 10, 25, 15,
                               " ", 2, "Segoe UI");

                    LoadButton(rptr.btn_move_center, rptr.window_mwt,
                               4, y += 25, MWT_WIDTH - 10, 25, 15,
                               " ", 3, "Segoe UI");

                    LoadButton(rptr.btn_move_left, rptr.window_mwt,
                               4, y += 25, MWT_WIDTH - 10, 25, 15,
                               " ", 4, "Segoe UI");

                    LoadButton(rptr.btn_move_right, rptr.window_mwt,
                               4, y += 25, MWT_WIDTH - 10, 25, 15,
                               " ", 5, "Segoe UI");
                }
            }
            else PostMessage(rptr.window_mwt->hwnd, WM_USER, 1, 0);

            _IsButtonHovered(rptr.btn_close_window, &p, 1);

        }   break;

        /// -------------------
        case WM_PAINT: {
            hdc = BeginPaint(rptr.window_popup->hwnd, &ps);

            GetClientRect(rptr.window_popup->hwnd, &rect);

            rect.left = 0; rect.top = 0; rect.right = POPUP_WIDTH; rect.bottom = POPUP_HEIGHT;
            brush = CreateSolidBrush(RGB(250, 250, 250));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            // Lines
            rect.left = 10; rect.top = 34; rect.right = POPUP_WIDTH - 10; rect.bottom = rect.top + 1;
            brush = CreateSolidBrush(RGB(210, 210, 210));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);
            rect.left = 10; rect.top = 169; rect.right = POPUP_WIDTH - 10; rect.bottom = rect.top + 1;
            brush = CreateSolidBrush(RGB(210, 210, 210));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            GetClientRect(rptr.window_popup->hwnd, &rect);
            pen = CreatePen(PS_SOLID, 5, RGB(238, 238, 238));
            prev_pen = (HPEN)SelectObject(hdc, pen);
            prev_brush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

            RoundRect(hdc, -1, -1, rect.right, rect.bottom, 10, 10);
            SelectObject(hdc, prev_pen);
            SelectObject(hdc, prev_brush);
            DeleteObject(pen);

            EndPaint(rptr.window_popup->hwnd, &ps);

        }   break;

        /// -------------------
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT)lparam;

//          _InvalidateButton(LPDRAWITEMSTRUCT item, Button* button,
//                     const char* text, int left, int activated, int arrow)
            if (item->hwndItem == rptr.btn_title->hwnd)
                _InvalidateButton(item, rptr.btn_title, APP_NAME, 35, 0);

            else if (item->hwndItem == rptr.btn_openfile->hwnd)
                _InvalidateButton(item, rptr.btn_openfile, "Open file...", 35, 0);

            else if (item->hwndItem == rptr.label_playback->hwnd)
                _InvalidateButton(item, rptr.label_playback, "Playback", 35, 0x2);

            else if (item->hwndItem == rptr.label_interaction->hwnd)
                _InvalidateButton(item, rptr.label_interaction, "Interaction", 35, 0x2);

            else if (item->hwndItem == rptr.label_window_scale->hwnd)
                _InvalidateButton(item, rptr.label_window_scale, "Window", 35, 0x2);

            else if (item->hwndItem == rptr.label_pin_window->hwnd)
                _InvalidateButton(item, rptr.label_pin_window, "Pin window", 35, 0x2);

            else if (item->hwndItem == rptr.label_move_window->hwnd)
                _InvalidateButton(item, rptr.label_move_window, "Move window to", 35, 0x2);

            else if (item->hwndItem == rptr.btn_close_window->hwnd)
                _InvalidateButton(item, rptr.btn_close_window, "Close window", 35, 0);

        }   break;

        /// -------------------
        case WM_CTLCOLORSTATIC: {
            brush = CreateSolidBrush(RGB(250, 250, 250));

            if (GetDlgCtrlID((HWND)lparam) == 13 || GetDlgCtrlID((HWND)lparam) == 14) {
                SetBkMode((HDC)wparam, TRANSPARENT);
                return (LRESULT)brush;
            }
        }   break;

        /// -------------------
        case WM_UPDATE_ALPHA: {
            uint8_t alpha = (uint8_t)lparam;
            if (hwnd != NULL) SetLayeredWindowAttributes(hwnd, 0x0, alpha, LWA_ALPHA);
        }   break;

        /// -------------------
        case WM_COMMAND: {
            switch (wparam) {
                /// -------------------
                case 1: {
                    PostMessage(rptr.window_popup->hwnd, WM_KEYDOWN, VK_ESCAPE, 0);
                    system("start https://github.com/repa214/GIFDesk/releases");
                }   break;

                /// Open file...
                case 2: {
                    if ((HWND)lparam == rptr.btn_openfile->hwnd) {
                        PostMessage(rptr.window->hwnd, WM_COMMAND, 1, 0);
                        ReleaseWindow(rptr.window_popup);
                    }
                }   break;

                /// Add scale
                case 4:
                case 327684: {
//                    if (lparam == (LPARAM)rptr.addscale_button->hwnd) {
//                        rptr.st->pos = SendMessage(rptr.scale_trackbar->hwnd, TBM_GETPOS, 0, 0) + 1;
//
//                        _ChangeScaleTrackBar(rptr.window, rptr.window_popup,
//                                             rptr.scale_trackbar, rptr.scale_button,
//                                             rptr.st, rptr.dt, rptr.rd,
//                                             rptr.st->pos);
//
//                        PostMessage(rptr.scale_trackbar->hwnd, TBM_SETPOS, TRUE, rptr.st->pos);
//                    }
                }   break;

                /// Decrease scale
                case 5:
                case 327685: {
//                    if (lparam == (LPARAM)rptr.decscale_button->hwnd) {
//                        rptr.st->pos = SendMessage(rptr.scale_trackbar->hwnd, TBM_GETPOS, 0, 0) - 1;
//
//                        _ChangeScaleTrackBar(rptr.window, rptr.window_popup,
//                                             rptr.scale_trackbar, rptr.scale_button,
//                                             rptr.st, rptr.dt, rptr.rd,
//                                             rptr.st->pos);
//
//                        PostMessage(rptr.scale_trackbar->hwnd, TBM_SETPOS, TRUE, rptr.st->pos);
//                    }
                }   break;

                /// Pause
                case 6: {
//                    if (rptr.rd->change_frames) rptr.rd->change_frames = 0;
//                    else rptr.rd->change_frames = 1;
//
//                    SetFocus(rptr.window_popup->hwnd);
                }   break;

                /// Show frame updates
                case 7: {
//                    if (rptr.st->sfu)
//                        rptr.st->sfu = 0;
//                    else
//                        rptr.st->sfu = 1;
//
//                    SetFocus(rptr.window_popup->hwnd);
//                    InvalidateRect(rptr.sfp_button->hwnd, NULL, TRUE);
                }   break;

                /// Show taskbar icon
                case 8: {
                    PostMessage(rptr.window->hwnd, WM_CLOSE, 0, 0);
//                    if ((HWND)lparam == rptr.sti_button->hwnd) {
//                        LONG_PTR style = GetWindowLong(rptr.window->hwnd, GWL_EXSTYLE);
//
//                        if (rptr.st->taskbar) {
//                            rptr.st->taskbar = 0;
//                            style |= WS_EX_TOOLWINDOW;
//                            style &= ~WS_EX_APPWINDOW;
//                        }
//                        else {
//                            rptr.st->taskbar = 1;
//                            style &= ~WS_EX_TOOLWINDOW;
//                            style |= WS_EX_APPWINDOW;
//                        }
//
//                        SetWindowLong(rptr.window->hwnd, GWL_EXSTYLE, style);
//                        SetWindowPos(rptr.window->hwnd, NULL,
//                                     0, 0, 0, 0,
//                                     SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
//
//                        if (!ts && rptr.st->taskbar) {
//                            ShowWindow(rptr.window->hwnd, SW_HIDE);
//                            ShowWindow(rptr.window->hwnd, SW_SHOW);
//                            ts = 1;
//                        }
//                        else ts = 1;
//                        SetFocus(rptr.window_popup->hwnd);
//
//                        WriteSettings(rptr.st);
//                    }
                }   break;

                /// Always on top
                case 9: {
//                    if (rptr.st->topmost)
//                        rptr.st->topmost = 0;
//                    else
//                        rptr.st->topmost = 1;
//
//                    SetWindowPos(rptr.window->hwnd, NULL,
//                             0, 0, 0, 0,
//                             SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
//
//                    SetFocus(rptr.window_popup->hwnd);
//                    InvalidateRect(rptr.aot_button->hwnd, NULL, TRUE);
//                    WriteSettings(rptr.st);
                }   break;

                /// -------------------
                case 12: {
//                    PostMessage(rptr.window->hwnd, WM_CLOSE, 0, 0);
                }   break;
            }
        }   break;

        /// -------------------
        case WM_KEYDOWN: {
            switch (wparam) {
                /// -------------------
                case VK_SPACE: {
                    if (rptr.rd->change_frames) rptr.rd->change_frames = 0;
                    else rptr.rd->change_frames = 1;
                }   break;

                /// -------------------
                case VK_ESCAPE: {
                    PostMessage(rptr.window_popup->hwnd, WM_CLOSE, 0, 0);
                }   break;

                /// -------------------
                case VK_LEFT:
                case VK_UP: {
//                    if (rptr.rd->change_frames)
//                        PostMessage(rptr.window_popup->hwnd, WM_COMMAND, 327685, (LPARAM)rptr.decscale_button->hwnd);
//                    else {
//                        SetFocus(rptr.window_popup->hwnd);
//                        int pos = SendMessage(rptr.trackbar_frames->hwnd, TBM_GETPOS, 0, 0) - 1;
//                        if (pos - 1 < 0)
//                            rptr.rd->frame = rptr.dt->count - 1;
//                        else
//                            rptr.rd->frame = pos - 1;
//                        PostMessage(rptr.trackbar_frames->hwnd, TBM_SETPOS, TRUE, pos);
//                        rptr.rd->framed_trackbar = 1; ShowFrame(rptr.window, rptr.dt, rptr.rd, rptr.st); rptr.rd->framed_trackbar = 0;
//                    }
                }   break;

                /// -------------------
                case VK_RIGHT:
                case VK_DOWN: {
//                    if (rptr.rd->change_frames)
//                        PostMessage(rptr.window_popup->hwnd, WM_COMMAND, 327684, (LPARAM)rptr.addscale_button->hwnd);
//                    else {
//                        SetFocus(rptr.window_popup->hwnd);
//                        int pos = SendMessage(rptr.trackbar_frames->hwnd, TBM_GETPOS, 0, 0);
//
//                        if (pos + 1 > rptr.dt->count) { rptr.rd->frame = 0; pos = 1; }
//                        else { rptr.rd->frame = pos; pos++; }
//
//                        SendMessage(rptr.trackbar_frames->hwnd, TBM_SETPOS, TRUE, pos);
//                        rptr.rd->framed_trackbar = 1; ShowFrame(rptr.window, rptr.dt, rptr.rd, rptr.st); rptr.rd->framed_trackbar = 0;
//                    }
                }   break;
            }
        }   break;

        default: break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK PBProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static RECT prect, brect, wrect, rect;
    static POINT p;
    static HBRUSH brush = NULL;
    static HBRUSH prev_brush = NULL;
    static HDC hdc = NULL;
    static PAINTSTRUCT ps;
    static HPEN pen = NULL;
    static HPEN prev_pen = NULL;

    switch (msg) {
        /// -------------------
        case WM_CREATE: {
            pthread_create(&thread_t, 0, ShowLowerPopupThread, hwnd);
        }   break;

        /// -------------------
        case WM_DESTROY: {
            PostQuitMessage(0);
        }   break;

        /// -------------------
        case WM_CLOSE: {
            DestroyWindow(hwnd);
            SetFocus(rptr.window_popup->hwnd);
            rptr.window_pb->isactive = 0;
        }   break;

        /// -------------------
        case WM_PAINT: {
            hdc = BeginPaint(rptr.window_pb->hwnd, &ps);

            GetClientRect(rptr.window_pb->hwnd, &rect);
            brush = CreateSolidBrush(RGB(250, 250, 250));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            pen = CreatePen(PS_SOLID, 3, RGB(238, 238, 238));
            prev_pen = (HPEN)SelectObject(hdc, pen);
            prev_brush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

            RoundRect(hdc, 0, 0, rect.right - 1, rect.bottom - 1, 10, 10);
            SelectObject(hdc, prev_pen);
            SelectObject(hdc, prev_brush);
            DeleteObject(pen);

            EndPaint(rptr.window_pb->hwnd, &ps);
        }   break;

        /// -------------------
        case WM_CTLCOLORSTATIC: {
            brush = CreateSolidBrush(RGB(250, 250, 250));

            if (GetDlgCtrlID((HWND)lparam) == 5) {
                SetBkMode((HDC)wparam, TRANSPARENT);
                return (LRESULT)brush;
            }
        }   break;

        /// -------------------
        case WM_SETCURSOR: {
            GetCursorPos(&p);

            _IsButtonHovered(rptr.label_frames, &p, 0);
            _IsButtonHovered(rptr.btn_prev_frame, &p, 0);
            _IsButtonHovered(rptr.btn_play, &p, 0);
            _IsButtonHovered(rptr.btn_next_frame, &p, 0);
            _IsButtonHovered(rptr.btn_frame_updates, &p, 0);
        }   break;

        /// -------------------
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT)lparam;

            if (item->hwndItem == rptr.label_frames->hwnd) {
                _InvalidateButton(item, rptr.label_frames, "Frames", 35, 0);
            }
            if (item->hwndItem == rptr.btn_prev_frame->hwnd) {
                _InvalidateButton(item, rptr.btn_prev_frame, "", 30, 0x4);
            }
            if (item->hwndItem == rptr.btn_play->hwnd) {
                _InvalidateButton(item, rptr.btn_play, "", 28, 0x10);
            }
            if (item->hwndItem == rptr.btn_next_frame->hwnd) {
                _InvalidateButton(item, rptr.btn_next_frame, "", 21, 0x8);
            }
            if (item->hwndItem == rptr.btn_frame_updates->hwnd) {
                _InvalidateButton(item, rptr.btn_frame_updates, "Show frame updates", 35, (uint8_t)rptr.st->sfu);
            }
        }   break;

        /// -------------------
        case WM_USER: {
            GetCursorPos(&p);

            GetWindowRect(rptr.window_pb->hwnd, &wrect);
            GetWindowRect(rptr.label_playback->hwnd, &brect);
            GetWindowRect(rptr.window_popup->hwnd, &prect);

            if (prect.left > wrect.left) brect.left -= 5;
            else brect.right += 5;

            if (!PtInRect(&wrect, p) && !PtInRect(&brect, p)) {
                PostMessage(rptr.window_pb->hwnd, WM_CLOSE, 0, 0);
                ReleaseHover(&rptr, rptr.label_playback->hwnd);
            }
        }   break;

        /// -------------------
        case WM_ACTIVATE: {
            GetCursorPos(&p);
            GetWindowRect(rptr.window_pb->hwnd, &rect);

            if (wparam != WA_INACTIVE ||
                IsChild(rptr.window_pb->hwnd, (HWND)lparam) ||
                (HWND)lparam == hwnd ||
                PtInRect(&rect, p)) break;

            PostMessage(rptr.window_pb->hwnd, WM_CLOSE, 0, 0);

//            if (rptr.window_mwt->isactive)
//                PostMessage(rptr.window_mwt->hwnd, WM_CLOSE, 0, 0);
        }   break;

        /// -------------------
        case WM_MOUSEMOVE: {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = hwnd;
            tme.dwFlags = TME_LEAVE;

            _TrackMouseEvent(&tme);
        }   break;

        /// -------------------
        case WM_MOUSELEAVE: {
            GetCursorPos(&p);

            GetWindowRect(rptr.window_pb->hwnd, &wrect);
            GetWindowRect(rptr.label_playback->hwnd, &brect);
            GetWindowRect(rptr.window_popup->hwnd, &prect);

            if (prect.left > wrect.left) brect.left -= 5;
            else brect.right += 5;

            if (!PtInRect(&wrect, p) && !PtInRect(&brect, p)) {
                PostMessage(rptr.window_pb->hwnd, WM_CLOSE, 0, 0);
                ReleaseHover(&rptr, rptr.label_playback->hwnd);
            }
            else {
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.hwndTrack = hwnd;
                tme.dwFlags = TME_LEAVE;
            }
        }   break;

        /// -------------------
        case WM_HSCROLL: {
            if ((HWND)lparam == rptr.trackbar_frames->hwnd) {
                SetFocus(rptr.window_popup->hwnd);
                rptr.rd->frame = SendMessage(rptr.trackbar_frames->hwnd, TBM_GETPOS, 0, 0) - 1;
                rptr.rd->framed_trackbar = 1; ShowFrame(rptr.window, rptr.dt, rptr.rd, rptr.st); rptr.rd->framed_trackbar = 0;
            }
        }   break;

        /// -------------------
        case WM_UPDATE_ALPHA: {
            uint8_t alpha = (uint8_t)lparam;
            SetLayeredWindowAttributes(hwnd, 0x0, alpha, LWA_ALPHA);
        }   break;

        /// -------------------
        case WM_COMMAND: {
            switch (wparam) {
                case 2:
                case 327682: {
                    if (--rptr.rd->frame < 0) rptr.rd->frame = rptr.dt->count - 1;
                }   break;

                case 3:
                case 327683: {
                    if (rptr.rd->change_frames) rptr.rd->change_frames = 0;
                    else rptr.rd->change_frames = 1;

                    SetFocus(rptr.window_popup->hwnd);
                }   break;

                case 4:
                case 327684: {
                    if (++rptr.rd->frame > rptr.dt->count - 1) rptr.rd->frame = 0;
                }   break;

                case 6:
                case 327686: {
                    if (lparam != (LPARAM)rptr.btn_frame_updates->hwnd) break;

                    if (rptr.st->sfu)
                        rptr.st->sfu = 0;
                    else
                        rptr.st->sfu = 1;

                    SetFocus(rptr.window_pb->hwnd);
                    InvalidateRect(rptr.btn_frame_updates->hwnd, NULL, TRUE);
                }   break;
            }
        }   break;

        default: break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK IMProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static RECT prect, brect, wrect, rect;
    static POINT p;
    static HBRUSH brush = NULL;
    static HBRUSH prev_brush = NULL;
    static HDC hdc = NULL;
    static PAINTSTRUCT ps;
    static HPEN pen = NULL;
    static HPEN prev_pen = NULL;

    switch (msg) {
        /// -------------------
        case WM_CREATE: {
            pthread_create(&thread_t, 0, ShowLowerPopupThread, hwnd);
        }   break;

        /// -------------------
        case WM_DESTROY: {
            PostQuitMessage(0);
        }   break;

        /// -------------------
        case WM_CLOSE: {
            DestroyWindow(hwnd);
            SetFocus(rptr.window_popup->hwnd);
            rptr.window_im->isactive = 0;
        }   break;

        /// -------------------
        case WM_PAINT: {
            hdc = BeginPaint(rptr.window_im->hwnd, &ps);

            GetClientRect(rptr.window_im->hwnd, &rect);
            brush = CreateSolidBrush(RGB(250, 250, 250));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            pen = CreatePen(PS_SOLID, 3, RGB(238, 238, 238));
            prev_pen = (HPEN)SelectObject(hdc, pen);
            prev_brush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

            RoundRect(hdc, 0, 0, rect.right - 1, rect.bottom - 1, 10, 10);
            SelectObject(hdc, prev_pen);
            SelectObject(hdc, prev_brush);
            DeleteObject(pen);

            EndPaint(rptr.window_im->hwnd, &ps);
        }   break;

        /// -------------------
        case WM_CTLCOLORSTATIC: {
            brush = CreateSolidBrush(RGB(250, 250, 250));

//            if (GetDlgCtrlID((HWND)lparam) == 4) {
//                SetBkMode((HDC)wparam, TRANSPARENT);
//                return (LRESULT)brush;
//            }
        }   break;

        /// -------------------
        case WM_SETCURSOR: {
            GetCursorPos(&p);

            _IsButtonHovered(rptr.btn_ignore_input, &p, 0);
        }   break;

        /// -------------------
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT)lparam;

            if (item->hwndItem == rptr.btn_ignore_input->hwnd) {
                _InvalidateButton(item, rptr.btn_ignore_input, "Ignore all input (except ESC)", 35, 0);
            }
        }   break;

        /// -------------------
        case WM_USER: {
            GetCursorPos(&p);

            GetWindowRect(rptr.window_im->hwnd, &wrect);
            GetWindowRect(rptr.label_interaction->hwnd, &brect);
            GetWindowRect(rptr.window_popup->hwnd, &prect);

            if (prect.left > wrect.left) brect.left -= 5;
            else brect.right += 5;

            if (!PtInRect(&wrect, p) && !PtInRect(&brect, p)) {
                PostMessage(rptr.window_im->hwnd, WM_CLOSE, 0, 0);
                ReleaseHover(&rptr, rptr.label_interaction->hwnd);
            }
        }   break;

        /// -------------------
        case WM_ACTIVATE: {
            if (wparam == WA_INACTIVE) {
                PostMessage(rptr.window_im->hwnd, WM_CLOSE, 0, 0);
                ReleaseHover(&rptr, rptr.label_interaction->hwnd);
            }
        }   break;

        /// -------------------
        case WM_MOUSEMOVE: {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = hwnd;
            tme.dwFlags = TME_LEAVE;

            _TrackMouseEvent(&tme);
        }   break;

        /// -------------------
        case WM_MOUSELEAVE: {
            GetCursorPos(&p);

            GetWindowRect(rptr.window_im->hwnd, &wrect);
            GetWindowRect(rptr.label_interaction->hwnd, &brect);
            GetWindowRect(rptr.window_popup->hwnd, &prect);

            if (prect.left > wrect.left) brect.left -= 5;
            else brect.right += 5;

            if (!PtInRect(&wrect, p) && !PtInRect(&brect, p)) {
                PostMessage(rptr.window_im->hwnd, WM_CLOSE, 0, 0);
                ReleaseHover(&rptr, rptr.label_interaction->hwnd);
            }
            else {
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.hwndTrack = hwnd;
                tme.dwFlags = TME_LEAVE;
            }
        }   break;

        /// -------------------
        case WM_UPDATE_ALPHA: {
            uint8_t alpha = (uint8_t)lparam;
            SetLayeredWindowAttributes(hwnd, 0x0, alpha, LWA_ALPHA);
        }   break;

        default: break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK WCProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static RECT prect, brect, wrect, rect;
    static POINT p;
    static HBRUSH brush = NULL;
    static HBRUSH prev_brush = NULL;
    static HDC hdc = NULL;
    static PAINTSTRUCT ps;
    static HPEN pen = NULL;
    static HPEN prev_pen = NULL;
    static uint8_t ts = 0;

    switch (msg) {
        /// -------------------
        case WM_CREATE: {
            pthread_create(&thread_t, 0, ShowLowerPopupThread, hwnd);
        }   break;

        /// -------------------
        case WM_DESTROY: {
            PostQuitMessage(0);
        }   break;

        /// -------------------
        case WM_CLOSE: {
            DestroyWindow(hwnd);
            SetFocus(rptr.window_popup->hwnd);
            rptr.window_wc->isactive = 0;
        }   break;

        /// -------------------
        case WM_PAINT: {
            hdc = BeginPaint(rptr.window_wc->hwnd, &ps);

            GetClientRect(rptr.window_wc->hwnd, &rect);
            brush = CreateSolidBrush(RGB(250, 250, 250));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            pen = CreatePen(PS_SOLID, 3, RGB(238, 238, 238));
            prev_pen = (HPEN)SelectObject(hdc, pen);
            prev_brush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

            RoundRect(hdc, 0, 0, rect.right - 1, rect.bottom - 1, 10, 10);
            SelectObject(hdc, prev_pen);
            SelectObject(hdc, prev_brush);
            DeleteObject(pen);

            EndPaint(rptr.window_wc->hwnd, &ps);
        }   break;

        /// -------------------
        case WM_CTLCOLORSTATIC: {
            brush = CreateSolidBrush(RGB(250, 250, 250));

            if (GetDlgCtrlID((HWND)lparam) == 4) {
                SetBkMode((HDC)wparam, TRANSPARENT);
                return (LRESULT)brush;
            }
        }   break;

        /// -------------------
        case WM_SETCURSOR: {
            GetCursorPos(&p);

            _IsButtonHovered(rptr.btn_add_scale, &p, 0);
            _IsButtonHovered(rptr.btn_subtract_scale, &p, 0);
            _IsButtonHovered(rptr.btn_taskbar, &p, 0);
        }   break;

        /// -------------------
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT)lparam;

            if (item->hwndItem == rptr.label_scale->hwnd) {
                sprintf(rptr.st->str_size, "Scale (%.0f%%)", rptr.st->trackbar_size * 100);
                _InvalidateButton(item, rptr.label_scale, rptr.st->str_size, 35, 0);
            }

            if (item->hwndItem == rptr.btn_add_scale->hwnd)
                _InvalidateButton(item, rptr.btn_add_scale, "+", 12, 0);

            if (item->hwndItem == rptr.btn_subtract_scale->hwnd)
                _InvalidateButton(item, rptr.btn_subtract_scale, "-", 12, 0);

            if (item->hwndItem == rptr.btn_taskbar->hwnd)
                _InvalidateButton(item, rptr.btn_taskbar, "Show taskbar icon", 35, (uint8_t)rptr.st->taskbar);
        }   break;

        /// -------------------
        case WM_USER: {
            GetCursorPos(&p);

            GetWindowRect(rptr.window_wc->hwnd, &wrect);
            GetWindowRect(rptr.label_window_scale->hwnd, &brect);
            GetWindowRect(rptr.window_popup->hwnd, &prect);

            if (prect.left > wrect.left) brect.left -= 5;
            else brect.right += 5;

            if (!PtInRect(&wrect, p) && !PtInRect(&brect, p)) {
                PostMessage(rptr.window_wc->hwnd, WM_CLOSE, 0, 0);
                ReleaseHover(&rptr, rptr.label_window_scale->hwnd);
            }
        }   break;

        /// -------------------
        case WM_ACTIVATE: {
            GetCursorPos(&p);
            GetWindowRect(rptr.window_wc->hwnd, &rect);

            if (wparam != WA_INACTIVE ||
                IsChild(rptr.window_wc->hwnd, (HWND)lparam) ||
                (HWND)lparam == hwnd ||
                PtInRect(&rect, p)) break;

            PostMessage(rptr.window_wc->hwnd, WM_CLOSE, 0, 0);

//            if (rptr.window_mwt->isactive)
//                PostMessage(rptr.window_mwt->hwnd, WM_CLOSE, 0, 0);
        }   break;

        /// -------------------
        case WM_MOUSEMOVE: {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = hwnd;
            tme.dwFlags = TME_LEAVE;

            _TrackMouseEvent(&tme);
        }   break;

        /// -------------------
        case WM_MOUSELEAVE: {
            GetCursorPos(&p);

            GetWindowRect(rptr.window_wc->hwnd, &wrect);
            GetWindowRect(rptr.label_window_scale->hwnd, &brect);
            GetWindowRect(rptr.window_popup->hwnd, &prect);

            if (prect.left > wrect.left) brect.left -= 5;
            else brect.right += 5;

            if (!PtInRect(&wrect, p) && !PtInRect(&brect, p)) {
                PostMessage(rptr.window_wc->hwnd, WM_CLOSE, 0, 0);
                ReleaseHover(&rptr, rptr.label_window_scale->hwnd);
            }
            else {
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.hwndTrack = hwnd;
                tme.dwFlags = TME_LEAVE;
            }
        }   break;

        /// -------------------
        case WM_UPDATE_ALPHA: {
            uint8_t alpha = (uint8_t)lparam;
            SetLayeredWindowAttributes(hwnd, 0x0, alpha, LWA_ALPHA);
        }   break;

        /// -------------------
        case WM_HSCROLL: {
            if ((HWND)lparam == rptr.trackbar_scale->hwnd) {
                rptr.st->pos = SendMessage(rptr.trackbar_scale->hwnd, TBM_GETPOS, 0, 0);

                _ChangeScaleTrackBar(rptr.window, rptr.window_wc,
                                     rptr.trackbar_scale, rptr.label_scale,
                                     rptr.st, rptr.dt, rptr.rd,
                                     rptr.st->pos);
            }
        }   break;

        /// -------------------
        case WM_COMMAND: {
            switch (wparam) {
                case 2:
                case 327682: {
                    if (lparam == (LPARAM)rptr.btn_add_scale->hwnd) {
                        rptr.st->pos = SendMessage(rptr.trackbar_scale->hwnd, TBM_GETPOS, 0, 0) + 1;

                        _ChangeScaleTrackBar(rptr.window, rptr.window_wc,
                                     rptr.trackbar_scale, rptr.label_scale,
                                     rptr.st, rptr.dt, rptr.rd,
                                     rptr.st->pos);

                        PostMessage(rptr.trackbar_scale->hwnd, TBM_SETPOS, TRUE, rptr.st->pos);
                    }
                }   break;

                case 3:
                case 327683: {
                    if (lparam == (LPARAM)rptr.btn_subtract_scale->hwnd) {
                        rptr.st->pos = SendMessage(rptr.trackbar_scale->hwnd, TBM_GETPOS, 0, 0) - 1;

                        _ChangeScaleTrackBar(rptr.window, rptr.window_wc,
                                             rptr.trackbar_scale, rptr.label_scale,
                                             rptr.st, rptr.dt, rptr.rd,
                                             rptr.st->pos);

                        PostMessage(rptr.trackbar_scale->hwnd, TBM_SETPOS, TRUE, rptr.st->pos);
                    }
                }   break;

                case 5: {
                    if ((HWND)lparam == rptr.btn_taskbar->hwnd) {
                        LONG_PTR style = GetWindowLong(rptr.window->hwnd, GWL_EXSTYLE);

                        if (rptr.st->taskbar) {
                            rptr.st->taskbar = 0;
                            style |= WS_EX_TOOLWINDOW;
                            style &= ~WS_EX_APPWINDOW;
                        }
                        else {
                            rptr.st->taskbar = 1;
                            style &= ~WS_EX_TOOLWINDOW;
                            style |= WS_EX_APPWINDOW;
                        }

                        SetWindowLong(rptr.window->hwnd, GWL_EXSTYLE, style);
                        SetWindowPos(rptr.window->hwnd, NULL,
                                     0, 0, 0, 0,
                                     SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

                        if (!ts && rptr.st->taskbar) {
                            ShowWindow(rptr.window->hwnd, SW_HIDE);
                            ShowWindow(rptr.window->hwnd, SW_SHOW);
                            ts = 1;
                        }
                        else ts = 1;

                        WriteSettings(rptr.st);
                    }
                }   break;
            }
        }   break;

        default: break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK PWProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static RECT prect, brect, wrect, rect;
    static POINT p;
    static HBRUSH brush = NULL;
    static HBRUSH prev_brush = NULL;
    static HDC hdc = NULL;
    static PAINTSTRUCT ps;
    static HPEN pen = NULL;
    static HPEN prev_pen = NULL;

    switch (msg) {
        /// -------------------
        case WM_CREATE: {
            pthread_create(&thread_t, 0, ShowLowerPopupThread, hwnd);
        }   break;

        /// -------------------
        case WM_DESTROY: {
            PostQuitMessage(0);
        }   break;

        /// -------------------
        case WM_CLOSE: {
            DestroyWindow(hwnd);
            SetFocus(rptr.window_popup->hwnd);
            rptr.window_pw->isactive = 0;
        }   break;

        /// -------------------
        case WM_PAINT: {
            hdc = BeginPaint(rptr.window_pw->hwnd, &ps);

            GetClientRect(rptr.window_pw->hwnd, &rect);
            brush = CreateSolidBrush(RGB(250, 250, 250));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            pen = CreatePen(PS_SOLID, 3, RGB(238, 238, 238)); // 235, 235, 235
            prev_pen = (HPEN)SelectObject(hdc, pen);
            prev_brush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

            RoundRect(hdc, 0, 0, rect.right - 1, rect.bottom - 1, 10, 10);
            SelectObject(hdc, prev_pen);
            SelectObject(hdc, prev_brush);
            DeleteObject(pen);

            EndPaint(rptr.window_pw->hwnd, &ps);
        }   break;

        /// -------------------
        case WM_SETCURSOR: {
            GetCursorPos(&p);

            _IsButtonHovered(rptr.btn_pin_default, &p, 0);
            _IsButtonHovered(rptr.btn_pin_top, &p, 0);
        }   break;

        /// -------------------
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT)lparam;

            if (item->hwndItem == rptr.btn_pin_default->hwnd)
                _InvalidateButton(item, rptr.btn_pin_default, "Default", 35, (uint8_t)!rptr.st->topmost);

            if (item->hwndItem == rptr.btn_pin_top->hwnd)
                _InvalidateButton(item, rptr.btn_pin_top, "Always on top", 35, (uint8_t)rptr.st->topmost);
        }   break;

        /** Checks whether window should exist **/

        case WM_USER: {
            GetCursorPos(&p);

            GetWindowRect(rptr.window_pw->hwnd, &wrect);
            GetWindowRect(rptr.label_pin_window->hwnd, &brect);
            GetWindowRect(rptr.window_popup->hwnd, &prect);

            if (prect.left > wrect.left) brect.left -= 5;
            else brect.right += 5;

            if (!PtInRect(&wrect, p) && !PtInRect(&brect, p)) {
                PostMessage(rptr.window_pw->hwnd, WM_CLOSE, 0, 0);
                ReleaseHover(&rptr, rptr.label_pin_window->hwnd);
            }
        }   break;

        /// -------------------
        case WM_ACTIVATE: {
            if (wparam == WA_INACTIVE) {
                PostMessage(rptr.window_pw->hwnd, WM_CLOSE, 0, 0);
                ReleaseHover(&rptr, rptr.label_pin_window->hwnd);
            }
        }   break;

        /// -------------------
        case WM_MOUSEMOVE: {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = hwnd;
            tme.dwFlags = TME_LEAVE;

            _TrackMouseEvent(&tme);
        }   break;

        /// -------------------
        case WM_MOUSELEAVE: {
            GetCursorPos(&p);

            GetWindowRect(rptr.window_pw->hwnd, &wrect);
            GetWindowRect(rptr.label_move_window->hwnd, &brect);
            GetWindowRect(rptr.window_popup->hwnd, &prect);

            if (prect.left > wrect.left) brect.left -= 5;
            else brect.right += 5;

            if (!PtInRect(&wrect, p) && !PtInRect(&brect, p)) {
                PostMessage(rptr.window_pw->hwnd, WM_CLOSE, 0, 0);
                ReleaseHover(&rptr, rptr.label_pin_window->hwnd);
            }
            else {
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.hwndTrack = hwnd;
                tme.dwFlags = TME_LEAVE;
            }
        }   break;

        /** Commands **/

        /// -------------------
        case WM_UPDATE_ALPHA: {
            uint8_t alpha = (uint8_t)lparam;
            SetLayeredWindowAttributes(hwnd, 0x0, alpha, LWA_ALPHA);
        }   break;

        /// -------------------
        case WM_COMMAND: {
            switch (wparam) {
                case 1: {
                    rptr.st->topmost = 0;

                    SetWindowPos(rptr.window->hwnd, NULL,
                             0, 0, 0, 0,
                             SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

                    SetFocus(rptr.window_popup->hwnd);
                    WriteSettings(rptr.st);
                }   break;
                case 2: {
                    rptr.st->topmost = 1;

                    SetWindowPos(rptr.window->hwnd, NULL,
                             0, 0, 0, 0,
                             SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

                    SetFocus(rptr.window_popup->hwnd);
                    WriteSettings(rptr.st);
                }   break;
            }
        }   break;

        default: break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK MWTProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static RECT prect, brect, wrect, rect, res;
    static POINT p;
    static HBRUSH brush = NULL;
    static HBRUSH prev_brush = NULL;
    static HDC hdc = NULL;
    static PAINTSTRUCT ps;
    static HPEN pen = NULL;
    static HPEN prev_pen = NULL;

    switch (msg)
    {
        /// -------------------
        case WM_CREATE: {
            pthread_create(&thread_t, 0, ShowLowerPopupThread, hwnd);
        }   break;

        /// -------------------
        case WM_DESTROY: {
            PostQuitMessage(0);
        }   break;

        /// -------------------
        case WM_CLOSE: {
            DestroyWindow(hwnd);
            SetFocus(rptr.window_popup->hwnd);
            rptr.window_mwt->isactive = 0;
        }   break;

        /// -------------------
        case WM_PAINT: {
            hdc = BeginPaint(rptr.window_mwt->hwnd, &ps);

            GetClientRect(rptr.window_mwt->hwnd, &rect);
            brush = CreateSolidBrush(RGB(250, 250, 250));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            pen = CreatePen(PS_SOLID, 3, RGB(238, 238, 238)); // 235, 235, 235
            prev_pen = (HPEN)SelectObject(hdc, pen);
            prev_brush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

            RoundRect(hdc, 0, 0, rect.right - 1, rect.bottom - 1, 10, 10);
            SelectObject(hdc, prev_pen);
            SelectObject(hdc, prev_brush);
            DeleteObject(pen);

            EndPaint(rptr.window_mwt->hwnd, &ps);
        }   break;

        /// -------------------
        case WM_SETCURSOR: {
            GetCursorPos(&p);

            _IsButtonHovered(rptr.btn_move_topleft, &p, 0);
            _IsButtonHovered(rptr.btn_move_topright, &p, 0);
            _IsButtonHovered(rptr.btn_move_center, &p, 0);
            _IsButtonHovered(rptr.btn_move_left, &p, 0);
            _IsButtonHovered(rptr.btn_move_right, &p, 0);
        }   break;

        /// -------------------
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT)lparam;

            if (item->hwndItem == rptr.btn_move_topleft->hwnd)
                _InvalidateButton(item, rptr.btn_move_topleft, "Top left corner", 35, 0);

            if (item->hwndItem == rptr.btn_move_topright->hwnd)
                _InvalidateButton(item, rptr.btn_move_topright, "Top right corner", 35, 0);

            if (item->hwndItem == rptr.btn_move_center->hwnd)
                _InvalidateButton(item, rptr.btn_move_center, "Center", 35, 0);

            if (item->hwndItem == rptr.btn_move_left->hwnd)
                _InvalidateButton(item, rptr.btn_move_left, "Bottom left corner", 35, 0);

            if (item->hwndItem == rptr.btn_move_right->hwnd)
                _InvalidateButton(item, rptr.btn_move_right, "Bottom right corner", 35, 0);
        }   break;

        /** Checks whether window should exist **/

        case WM_USER: {
            GetCursorPos(&p);

            GetWindowRect(rptr.window_mwt->hwnd, &wrect);
            GetWindowRect(rptr.label_move_window->hwnd, &brect);
            GetWindowRect(rptr.window_popup->hwnd, &prect);

            if (prect.left > wrect.left) brect.left -= 5;
            else brect.right += 5;

            if (!PtInRect(&wrect, p) && !PtInRect(&brect, p)) {
                PostMessage(rptr.window_mwt->hwnd, WM_CLOSE, 0, 0);
                ReleaseHover(&rptr, rptr.label_move_window->hwnd);
            }
        }   break;

        /// -------------------
        case WM_ACTIVATE: {
            if (wparam == WA_INACTIVE) {
                PostMessage(rptr.window_mwt->hwnd, WM_CLOSE, 0, 0);
                ReleaseHover(&rptr, rptr.label_move_window->hwnd);
            }
        }   break;

        /// -------------------
        case WM_MOUSEMOVE: {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = hwnd;
            tme.dwFlags = TME_LEAVE;

            _TrackMouseEvent(&tme);
        }   break;

        /// -------------------
        case WM_MOUSELEAVE: {
            GetCursorPos(&p);

            GetWindowRect(rptr.window_mwt->hwnd, &wrect);
            GetWindowRect(rptr.label_move_window->hwnd, &brect);
            GetWindowRect(rptr.window_popup->hwnd, &prect);

            if (prect.left > wrect.left) brect.left -= 5;
            else brect.right += 5;

            if (!PtInRect(&wrect, p) && !PtInRect(&brect, p)) {
                PostMessage(rptr.window_mwt->hwnd, WM_CLOSE, 0, 0);
                ReleaseHover(&rptr, rptr.label_move_window->hwnd);
            }
            else {
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.hwndTrack = hwnd;
                tme.dwFlags = TME_LEAVE;
            }
        }   break;

        /** Commands **/

        /// -------------------
        case WM_UPDATE_ALPHA: {
            uint8_t alpha = (uint8_t)lparam;
            SetLayeredWindowAttributes(hwnd, 0x0, alpha, LWA_ALPHA);
        }   break;

        /// -------------------
        case WM_COMMAND: {
            switch (wparam) {
                /// -------------------
                case 1: {
                    SetWindowPos(rptr.window->hwnd,
                                 NULL,
                                 0,
                                 0,
                                 0,
                                 0,
                                 SWP_NOSIZE);
                }   break;

                /// -------------------
                case 2: {
                    SetWindowPos(rptr.window->hwnd,
                                 NULL,
                                 GetSystemMetrics(SM_CXSCREEN) - (rptr.dt->width) * rptr.st->trackbar_size,
                                 0,
                                 0,
                                 0,
                                 SWP_NOSIZE);
                }   break;

                /// -------------------
                case 3: {
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);

                    SetWindowPos(rptr.window->hwnd,
                                 NULL,
                                 (res.right - res.left - rptr.dt->width * rptr.st->trackbar_size) / 2,
                                 (res.bottom - res.top - rptr.dt->height * rptr.st->trackbar_size) / 2,
                                 0,
                                 0,
                                 SWP_NOSIZE);
                }   break;

                /// -------------------
                case 4: {
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);

                    SetWindowPos(rptr.window->hwnd,
                                 NULL,
                                 0,
                                 res.bottom - res.top - (rptr.dt->height) * rptr.st->trackbar_size,
                                 0,
                                 0,
                                 SWP_NOSIZE);
                }   break;

                /// -------------------
                case 5: {
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);

                    SetWindowPos(rptr.window->hwnd,
                                 NULL,
                                 res.right - res.left - (rptr.dt->width) * rptr.st->trackbar_size,
                                 res.bottom - res.top - (rptr.dt->height) * rptr.st->trackbar_size,
                                 0,
                                 0,
                                 SWP_NOSIZE);
                }   break;

                default: break;
            }
        }   break;

        default: break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}


BOOL CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static char message[256];
    static HDC hdc;
    static HICON hicon = NULL;

    switch (msg)
    {
        /// -------------------
        case WM_INITDIALOG:
            hicon = (HICON)LoadImageW(GetModuleHandleW(NULL),
                                        MAKEINTRESOURCEW(IDI_ICON),
                                        IMAGE_ICON,
                                        GetSystemMetrics(SM_CXSMICON),
                                        GetSystemMetrics(SM_CYSMICON),
                                        0);

            SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hicon);

            switch (rptr.dt->error) {
                case 255:
                    sprintf(message, INVALIDE_FORMAT);
                    break;
                default:
                    sprintf(message, INVALIDE_LOAD, rptr.dt->error);
                    break;
            }

            SetDlgItemText(hwnd, 3002, message);
            break;

        /// -------------------
        case WM_CTLCOLORDLG:
            return (LRESULT)CreateSolidBrush(RGB(255, 255, 255));
            break;

        /// -------------------
        case WM_CTLCOLORSTATIC:
            hdc = (HDC)wparam;
            SetBkMode(hdc, TRANSPARENT);
            SetBkColor(hdc, RGB(255, 255, 255));
            SetTextColor(hdc, RGB(0, 0, 0));

            if (GetDlgCtrlID((HWND)lparam) == 3000)
            {
                HBRUSH hColorBrush = CreateSolidBrush(RGB(242, 242, 242));
                return (LRESULT)hColorBrush;
            }
            return (LRESULT)CreateSolidBrush(RGB(255, 255, 255));
            break;

        /// -------------------
        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case ID_SELECT:
                    EndDialog(hwnd, 1);
                    return TRUE;

                case ID_EXIT:
                    EndDialog(hwnd, 0);
                    return TRUE;

                break;
            }
            break;
    }
    return FALSE;
}

void ReleaseHover(RenderPtr* rptr, HWND hwnd)
{
    rptr->btn_title->hovered = 0;
    rptr->btn_openfile->hovered = 0;
    rptr->label_window_scale->hovered = 0;
    rptr->label_scale->hovered = 0;
    rptr->btn_add_scale->hovered = 0;
    rptr->btn_subtract_scale->hovered = 0;
    rptr->label_playback->hovered = 0;
    rptr->label_frames->hovered = 0;
    rptr->btn_prev_frame->hovered = 0;
    rptr->btn_play->hovered = 0;
    rptr->btn_next_frame->hovered = 0;
    rptr->label_speed->hovered = 0;
    rptr->btn_slow_rewind->hovered = 0;
    rptr->btn_fast_rewind->hovered = 0;
    rptr->btn_slow_wind->hovered = 0;
    rptr->btn_fast_wind->hovered = 0;
    rptr->label_transparency->hovered = 0;
    rptr->btn_frame_updates->hovered = 0;
    rptr->label_interaction->hovered = 0;
    rptr->btn_ignore_input->hovered = 0;
    rptr->label_pin_window->hovered = 0;
    rptr->btn_pin_default->hovered = 0;
    rptr->btn_pin_top->hovered = 0;
    rptr->btn_pin_bottom->hovered = 0;
    rptr->label_move_window->hovered = 0;
    rptr->btn_move_topleft->hovered = 0;
    rptr->btn_move_topright->hovered = 0;
    rptr->btn_move_center->hovered = 0;
    rptr->btn_move_left->hovered = 0;
    rptr->btn_move_right->hovered = 0;
    rptr->btn_close_window->hovered = 0;
    rptr->btn_taskbar->hovered = 0;

    if (hwnd != NULL) InvalidateRect(hwnd, NULL, TRUE);
}

void _InvalidateButton(LPDRAWITEMSTRUCT item, Button* button,
                       const char* text, int left, uint8_t flag)
{
    static HDC hdc = NULL;
    static HFONT font = NULL;
    static RECT rect;
    rect = item->rcItem;
    hdc = item->hDC;

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, BTN_TEXT_COLOR);
    HBRUSH hBrush = CreateSolidBrush(
        (button->hovered) ? BTN_HOVERED : BTN_NOT_HOVERED
    );
    FillRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);
    rect.left = left;
    DrawText(hdc, text, -1, &rect, DT_VCENTER | DT_SINGLELINE);

    int size = 20;
    font = CreateFont(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH, "Marlett");

    if (flag & 0x1) {
        int x = rect.left - 27;
        int y = rect.top + (rect.bottom - rect.top - size) / 2;

        SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, BTN_MARLETT_COLOR);
        TextOutW(hdc, x, y, L"a", 1);

        DeleteObject(font);
    }

    if (flag & 0x2) {
        int x = rect.left + 176;
        int y = rect.top + (rect.bottom - rect.top - size) / 2;

        SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, BTN_MARLETT_COLOR);
        TextOutW(hdc, x, y, L"8", 1);
        SetTextColor(hdc, BTN_NOT_HOVERED);
        TextOutW(hdc, x - 2, y, L"8", 1);

        DeleteObject(font);
    }

    if (flag & 0x4) {
        size = 15;
        font = CreateFont(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH, "Marlett");
        int x = rect.left;
        int y = rect.top + (rect.bottom - rect.top - size) / 2 - 1;

        SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, BTN_MARLETT_FLAG_COLOR);

        TextOutW(hdc, x, y, L"g", 1);
        SetTextColor(hdc, (button->hovered) ? BTN_HOVERED : BTN_NOT_HOVERED);
        TextOutW(hdc, x + 2, y, L"g", 1);
        SetTextColor(hdc, BTN_MARLETT_FLAG_COLOR);

        size = 20;
        font = CreateFont(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH, "Marlett");

        SelectObject(hdc, font);
        y = rect.top + (rect.bottom - rect.top - size) / 2;

        TextOutW(hdc, x + 2, y, L"3", 1);

        DeleteObject(font);
    }

    if (flag & 0x8) {
        size = 15;
        font = CreateFont(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH, "Marlett");
        int x = rect.left;
        int y = rect.top + (rect.bottom - rect.top - size) / 2 - 1;

        SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, BTN_MARLETT_FLAG_COLOR);

        TextOutW(hdc, x + 6, y, L"g", 1);
        SetTextColor(hdc, (button->hovered) ? BTN_HOVERED : BTN_NOT_HOVERED);
        TextOutW(hdc, x + 4, y, L"g", 1);
        SetTextColor(hdc, BTN_MARLETT_FLAG_COLOR);

        size = 20;
        font = CreateFont(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH, "Marlett");

        SelectObject(hdc, font);
        y = rect.top + (rect.bottom - rect.top - size) / 2;

        TextOutW(hdc, x, y, L"4", 1);

        DeleteObject(font);
    }

    if (flag & 0x10) {
        int x = rect.left;
        int y = rect.top + (rect.bottom - rect.top - size) / 2;

        size = 18;
        font = CreateFont(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH, "Marlett");

        SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, BTN_MARLETT_FLAG_COLOR);
        TextOutW(hdc, x, y, L"g", 1);
        SetTextColor(hdc, (button->hovered) ? BTN_HOVERED : BTN_NOT_HOVERED);
        TextOutW(hdc, x + 3, y, L"g", 1);
        SetTextColor(hdc, BTN_MARLETT_FLAG_COLOR);
        TextOutW(hdc, x + 6, y, L"g", 1);
        SetTextColor(hdc, (button->hovered) ? BTN_HOVERED : BTN_NOT_HOVERED);
        TextOutW(hdc, x + 9, y, L"g", 1);

        DeleteObject(font);
    }
}

int _IsButtonHovered(Button* button, POINT* p, int arrowed)
{
    static RECT rect; GetWindowRect(button->hwnd, &rect);
    if (arrowed) { rect.left -= 5; rect.right += 5; }
    if (PtInRect(&rect, *p) && !button->hovered) { button->hovered = 1; InvalidateRect(button->hwnd, NULL, TRUE); }
    else if (!PtInRect(&rect, *p) && button->hovered) { button->hovered = 0; InvalidateRect(button->hwnd, NULL, TRUE); }
    return button->hovered;
}

int _GetCollisionSize(int n, float size)
{
    return (n * size < 10.0) ? 10.0 : n * size + size;
}

void _ChangeScaleTrackBar(Window* window, Window* window_popup,
                          Trackbar* scale_trackbar, Button* scale_button,
                          Settings* st, Data* dt,
                          Render* rd, int pos)
{
    SetFocus(window_popup->hwnd);

    if (pos < 1) pos = 1;
    else if (pos > 200) pos = 200;

    texCoord[1] = 2 / ((float)pos / 100);
    texCoord[2] = 2 / ((float)pos / 100);
    texCoord[3] = 2 / ((float)pos / 100);
    texCoord[4] = 2 / ((float)pos / 100);

    st->trackbar_size = (float)pos / 100;

    sprintf(st->str_size, "Scale (%.0f%%)", st->trackbar_size * 100);
    SetWindowText(scale_button->hwnd, st->str_size);

    if (rd->change_frames)
    {
        rd->change_frames = 0; ShowFrame(window, dt, rd, st); rd->change_frames = 1;
    }
    else
        ShowFrame(window, dt, rd, st);
}

void* ShowPopupThread(void* arg)
{
    Sleep(10);
    for (uint16_t i = 20; i <= 255; i += 20) {
        PostMessage(arg, WM_UPDATE_ALPHA, 0, (LPARAM)i);
        Sleep(1);
    }
    PostMessage(arg, WM_UPDATE_ALPHA, 0, 255);
    return NULL;
}

void* ShowLowerPopupThread(void* arg)
{
    Sleep(10);
    for (uint16_t i = 20; i <= 255; i += 40) {
        PostMessage(arg, WM_UPDATE_ALPHA, 0, (LPARAM)i);
        Sleep(1);
    }
    PostMessage(arg, WM_UPDATE_ALPHA, 0, 255);
    return NULL;
}

/**
        OpenGL Proccessing
**/

void ShowFrame(Window* window, Data* dt, Render* rd, Settings* st)
{
    wglMakeCurrent(window->hdc, window->hrc);

    RECT rect; GetWindowRect(window->hwnd, &rect);

//    printf("\33[2K\r");
//    printf("Frame: %d | [%ld, %ld] | [%f, %f] [%f, %f]\n", rd->frame + 1,
//           rect.right - rect.left,
//           rect.bottom - rect.top,
//           dt->frame_points[rd->frame * 4],
//           dt->frame_points[(rd->frame * 4) + 1],
//           dt->frame_points[(rd->frame * 4) + 2],
//           dt->frame_points[(rd->frame * 4) + 3]);

    glViewport(0, 0, _GetCollisionSize(dt->width, st->size),
                     _GetCollisionSize(dt->height, st->size));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0, 0, 0, 0); glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, dt->textures[rd->frame]);

    glColor4f(1, 1, 1, 1);

    glPushMatrix();
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertex);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoord);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    /** **/
    if (st->sfu)
    {
        glViewport(0,
                   _GetCollisionSize(dt->height, st->size) - _GetCollisionSize(dt->height, st->trackbar_size),
                   _GetCollisionSize(dt->width, st->trackbar_size),
                   _GetCollisionSize(dt->height, st->trackbar_size));

        glColor4f(1.0, 0.0, 0.0, 1.0);
        glLineWidth(1.0f);

        glBegin(GL_LINE_LOOP);
            glVertex2f(dt->frame_points[rd->frame * 4],
                       dt->frame_points[rd->frame * 4 + 3]);
            glVertex2f(dt->frame_points[rd->frame * 4 + 2],
                       dt->frame_points[rd->frame * 4 + 3]);
            glVertex2f(dt->frame_points[rd->frame * 4 + 2],
                       dt->frame_points[rd->frame * 4 + 1]);
            glVertex2f(dt->frame_points[rd->frame * 4],
                       dt->frame_points[rd->frame * 4 + 1]);
        glEnd();
    }

    SwapBuffers(window->hdc);

    if (!rd->framed_trackbar)
        PostMessage(rptr.trackbar_frames->hwnd, TBM_SETPOS, TRUE, rd->frame + 1);

    if (rd->frame == dt->count - 1 && rd->change_frames) rd->frame = 0;
    else if (rd->change_frames) rd->frame++;

    wglMakeCurrent(NULL, NULL);
}

void ShowLoadLine(Window* window, Data* dt, Settings* st, float pt)
{
    wglMakeCurrent(window->hdc, window->hrc);

    RECT rect; GetWindowRect(window->hwnd, &rect);

    glViewport(0, 0, _GetCollisionSize(rect.right - rect.left, st->size),
                     _GetCollisionSize(rect.bottom - rect.top, st->size));

    glClearColor(0, 0, 0, 0); glClear(GL_COLOR_BUFFER_BIT);

    glColor4f(0.5, 0.5, 0.5, 1.0);
    glLineWidth(1.0f);

    pt *= 1.995f;

    glBegin(GL_QUADS);
        glVertex2f(-0.995f, 0.995f - pt);
        glVertex2f(-0.995f, 0.995f);
        glVertex2f(pt - 0.995f, 0.995f);
        glVertex2f(pt - 0.995f, 0.995f - pt);
    glEnd();

    SwapBuffers(window->hdc);

    wglMakeCurrent(NULL, NULL);
}

DWORD WINAPI RenderThread(LPVOID arg)
{
    RenderPtr* rptr = (RenderPtr*)arg;
    while (rptr->rd->render_thread) {
        if (ChangeFrame(rptr->dt, rptr->rd)) ShowFrame(rptr->window, rptr->dt, rptr->rd, rptr->st);
        Sleep(1);
    }

    return 0;
}

/**
        Time Proccessing
**/

double GetTime()
{
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / freq.QuadPart;
}

uint8_t ChangeFrame(Data* dt, Render* rd)
{
    rd->current_time = GetTime();

    if (rd->current_time - 0.002 > rd->start_time + dt->delays[rd->frame] + rd->inaccuracy)
    {
        rd->inaccuracy += dt->delays[rd->frame] - (rd->current_time - rd->start_time);

        if (rd->inaccuracy < -dt->delays[rd->frame] * 2)
            rd->inaccuracy = 0;

        // printf("%f | %f | %f | %f\n", rd->start_time, rd->current_time, dt->delays[rd->frame], rd->inaccuracy);

        rd->start_time = rd->current_time;
        return 1;
    }
    else return 0;
}

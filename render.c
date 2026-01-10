#include "render.h"

float vertex[] = {-1,  1,
                   1,  1,
                   1, -1,
                  -1, -1};

float texCoord[] = {0, 0,
                    1, 0,
                    1, 1,
                    0, 1};

RenderPtr rptr;
pthread_t thread_t;

void RptrInit(RenderPtr* rptr, Settings* st, Data* dt, Render* rd,
              Window* window, Window* window_popup, Window* window_debug,
              Window* window_wc, Window* window_pb, Window* window_im, Window* window_pw, Window* window_mwt,

              Trackbar* trackbar_scale, Trackbar* trackbar_frames, Trackbar* trackbar_speed, Trackbar* trackbar_transparency,

              Button* btn_title, Button* btn_openfile, Button* label_window_scale, Button* label_scale, Button* btn_add_scale,
              Button* btn_subtract_scale, Button* label_playback, Button* label_frames, Button* btn_prev_frame, Button* btn_play,
              Button* btn_next_frame, Button* label_speed, Button* btn_slow_rewind, Button* btn_fast_rewind, Button* btn_slow_wind,
              Button* btn_fast_wind, Button* label_transparency, Button* btn_frame_updates,
              Button* label_interaction, Button* btn_disable_moving, Button* btn_hide_hover,
              Button* btn_click_through, Button* btn_ignore_input,
              Button* btn_pin_top, Button* label_move_window, Button* btn_move_topleft,
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
    rptr->btn_disable_moving = btn_disable_moving;
    rptr->btn_hide_hover = btn_hide_hover;
    rptr->btn_click_through = btn_click_through;
    rptr->btn_ignore_input = btn_ignore_input;
    rptr->btn_pin_top = btn_pin_top;

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

void Loop(RenderPtr* rptr)
{
    MSG msg;
    rptr->st->trackbar_size = rptr->st->size;
    rptr->rd->frame = 0;
    rptr->rd->render_thread = 1;
    rptr->rd->start_time = GetTime();
    rptr->rd->change_frames = 1;
    rptr->rd->inaccuracy = 0;
    rptr->rd->framed_trackbar = 0;
    rptr->rd->loading = 0;

    /// Main Loop

    while (IsWindow(rptr->window->hwnd)) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!IsWindow(rptr->window->hwnd)) continue;

        if (!rptr->rd->loading /** && ChangeFrame(rptr->dt, rptr->rd, rptr->st) **/ )
//        if (!rptr->rd->loading && ChangeFrame(rptr->dt, rptr->rd, rptr->st))
            ShowFrame(rptr->window, rptr->dt, rptr->rd, rptr->st, 0);
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
        case WM_CREATE: {
            SetTimer(hwnd, 100, 100, NULL);
        }   break;

        /// -------------------
        case WM_CLOSE: {
            if (IsWindow(rptr.window->hwnd))
                DestroyWindow(rptr.window->hwnd);
        }   break;

        /// -------------------
        case WM_NCHITTEST: {
            return HTCLIENT;
        }

        /// -------------------
        case WM_DROPFILES: {
            DragAcceptFiles(hwnd, FALSE);

            _LoadDropFile((HDROP)wparam, rptr.window, rptr.st, rptr.dt, rptr.rd);

            DragAcceptFiles(hwnd, TRUE);
        }   break;

        /// -------------------
        case WM_LBUTTONDOWN: {
            if (rptr.st->ignore_input) break;

            rptr.rd->render_thread = 1; pthread_create(&thread_t, 0, RenderThread, &rptr);

            if (!rptr.window_popup->isactive && !rptr.st->disable_moving)
                SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            GetWindowRect(hwnd, &rect);
            rptr.st->x = rect.left; rptr.st->y = rect.top;
            WriteSettings(rptr.st);

            rptr.rd->render_thread = 0; pthread_join(thread_t, NULL);
        }   break;

        /// -------------------
        case WM_RBUTTONDOWN: {
            ReleaseWindow(rptr.window_popup);
            ReleaseHover(&rptr, NULL);

            GetCursorPos(&p);
            SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
            GetWindowRect(hwnd, &res);

            if (p.x + POPUP_WIDTH > rect.right - rect.left) { p.x -= POPUP_WIDTH; }
            if (p.y + POPUP_HEIGHT > rect.bottom - rect.top) { p.y -= POPUP_HEIGHT; }

            /** Resize Window **/

            rptr.rd->loading = 1;
            if (res.left + (res.right - res.left) / 2 < rect.right / 2 &&
                res.top + (res.bottom - res.top) / 2 < rect.bottom / 2)
                rptr.rd->pos = POS_LTC;
            else if (res.left + (res.right - res.left) / 2 < rect.right / 2 &&
                res.top + (res.bottom - res.top) / 2 >= rect.bottom / 2)
                rptr.rd->pos = POS_LLC;
            else if (res.left + (res.right - res.left) / 2 >= rect.right / 2 &&
                res.top + (res.bottom - res.top) / 2 < rect.bottom / 2)
                rptr.rd->pos = POS_RTC;
            else if (res.left + (res.right - res.left) / 2 >= rect.right / 2 &&
                res.top + (res.bottom - res.top) / 2 >= rect.bottom / 2)
                rptr.rd->pos = POS_RLC;
            if (res.bottom > rect.bottom + 5) rptr.rd->pos = POS_LTC;
            else if (res.right > rect.right + 5) rptr.rd->pos = POS_LTC;

            switch (rptr.rd->pos) {
                case POS_LTC:
                    SetWindowPos(rptr.window->hwnd, HWND_NOTOPMOST,
                                 0,
                                 0,
                                 _GetCollisionSize(rptr.dt->width, 2),
                                 _GetCollisionSize(rptr.dt->height, 2), SWP_NOMOVE | SWP_NOREDRAW);
                    break;
                case POS_LLC:
                    SetWindowPos(rptr.window->hwnd, HWND_NOTOPMOST,
                                 res.left,
                                 res.top - (rptr.dt->height * 2 - ((float)rptr.dt->height * rptr.st->size + 0.5)) + 1,
                                 _GetCollisionSize(rptr.dt->width, 2),
                                 _GetCollisionSize(rptr.dt->height, 2), SWP_NOREDRAW);
                    break;
                case POS_RTC:
                    SetWindowPos(rptr.window->hwnd, HWND_NOTOPMOST,
                                 res.left - (rptr.dt->width * 2 - ((float)rptr.dt->width * rptr.st->size + 0.5)) + 1,
                                 res.top,
                                 _GetCollisionSize(rptr.dt->width, 2),
                                 _GetCollisionSize(rptr.dt->height, 2), SWP_NOREDRAW);
                    break;
                case POS_RLC:
                    SetWindowPos(rptr.window->hwnd, HWND_NOTOPMOST,
                                 res.left - (rptr.dt->width * 2 - ((float)rptr.dt->width * rptr.st->size + 0.5)) + 1,
                                 res.top - (rptr.dt->height * 2 - ((float)rptr.dt->height * rptr.st->size + 0.5)) + 1,
                                 _GetCollisionSize(rptr.dt->width, 2),
                                 _GetCollisionSize(rptr.dt->height, 2), SWP_NOREDRAW);
                    break;
            }
            rptr.st->trackbar_size = rptr.st->size; rptr.st->size = 2;
            rptr.rd->loading = 0;

            DragAcceptFiles(rptr.window->hwnd, FALSE);

            /** Popup Menu **/

            LoadWindow(rptr.window_popup, rptr.st, NULL, rptr.rd,
                       "window_popup", p.x + 5, p.y + 5,
                       1, 1, POPUP_MENU_DEFAULT);

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

            LoadButton(rptr.label_interaction, rptr.window_popup,
                       5, y += 25, POPUP_WIDTH - 10, 25, 15,
                       " ", 4, "Segoe UI");

            LoadButton(rptr.label_window_scale, rptr.window_popup,
                       5, y += 25, POPUP_WIDTH - 10, 25, 15,
                       " ", 5, "Segoe UI");

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
                         SWP_NOMOVE | SWP_NOREDRAW);

            hrgn = CreateRoundRectRgn(0, 0,
                                      POPUP_WIDTH, POPUP_HEIGHT,
                                      10, 10);

            SetWindowRgn(rptr.window_popup->hwnd, hrgn, TRUE);
        }   break;

        /// -------------------
        case WM_SETCURSOR: {
            if (rptr.st->hide_on_hover)
                SetLayeredWindowAttributes(hwnd, 0, 1, LWA_ALPHA);

            if (rptr.st->disable_moving || rptr.st->hide_on_hover)
                SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_ARROW));
            else
                SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_SIZEALL));
        }   break;

        /// -------------------
        case WM_COMMAND: {
            switch (wparam) {
                /// -------------------
                case 1:
                    pthread_create(&thread_t, 0, _LoadSettings, &rptr);
                    pthread_detach(thread_t);
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
                    SetWindowPos(rptr.window->hwnd,
                             (rptr.st->topmost) ? HWND_TOPMOST : HWND_NOTOPMOST,
                             0,
                             0,
                             _GetCollisionSize(rptr.dt->width, rptr.st->size),
                             _GetCollisionSize(rptr.dt->height, rptr.st->size),
                             SWP_NOMOVE);
                    break;
                case 4:
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);

                    if (_GetCollisionSize(rptr.dt->width, rptr.st->size) > res.right / 2)
                        rptr.st->size = (float)res.right / 2 / (float)rptr.dt->width;

                    if (_GetCollisionSize(rptr.dt->height, rptr.st->size) > res.bottom / 2)
                        rptr.st->size = (float)res.bottom / 2 / (float)rptr.dt->height;

                    rptr.st->trackbar_size = rptr.st->size;

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

        /// -------------------
        case WM_TIMER: {
            GetCursorPos(&p); GetWindowRect(hwnd, &rect);
            if (!PtInRect(&rect, p) && rptr.st->hide_on_hover) {
                SetLayeredWindowAttributes(hwnd, 0x0, 0, LWA_COLORKEY);
            }
            else if (rptr.st->click_through) {
                if (GetAsyncKeyState(VK_CONTROL) & GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
                    PostMessage(rptr.window->hwnd, WM_NCHITTEST, 0, 0);
                    PostMessage(rptr.window->hwnd, WM_RBUTTONDOWN, 0, 0);
                }
            }

            if (rptr.st->click_through) {
                if (PtInRect(&rect, p) && rptr.st->hide_on_hover)
                    SetLayeredWindowAttributes(hwnd, 0, 1, LWA_ALPHA);

                LONG_PTR style = GetWindowLong(rptr.window->hwnd, GWL_EXSTYLE);
                if (style & WS_EX_TRANSPARENT) break;
                style |= WS_EX_TRANSPARENT;
                SetWindowLong(rptr.window->hwnd, GWL_EXSTYLE, style);
            }

            if (rptr.st->ignore_input) {
                DragAcceptFiles(hwnd, FALSE);

                if (rptr.st->click_through) {
                    LONG_PTR style = GetWindowLong(rptr.window->hwnd, GWL_EXSTYLE);
                    style &= ~WS_EX_TRANSPARENT;
                    rptr.st->click_through = 0;

                    SetWindowLong(rptr.window->hwnd, GWL_EXSTYLE, style);
                }

                DragAcceptFiles(hwnd, FALSE);
                SetWindowLongPtr(rptr.window->hwnd, GWLP_WNDPROC, (LONG_PTR)EscapeWindowProc);
            }
        }   break;

        default: break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK EscapeWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static POINT p;
    static RECT rect;
    switch (msg)
    {
        /// -------------------
        case WM_CREATE: {
            SetTimer(hwnd, 100, 100, NULL);
        }   break;

        /// -------------------
        case WM_SETCURSOR: {
            if (rptr.st->hide_on_hover)
                SetLayeredWindowAttributes(hwnd, 0, 1, LWA_ALPHA);

            DragAcceptFiles(hwnd, FALSE);
            SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_ARROW));
        }   return 0;

        /// -------------------
        case WM_KEYDOWN: {
            switch (wparam) {
                /// -------------------
                case VK_ESCAPE:
                    rptr.st->ignore_input = 0;
                    WriteSettings(rptr.st);
                    DragAcceptFiles(hwnd, TRUE);
                    SetWindowLongPtr(rptr.window->hwnd, GWLP_WNDPROC, (LONG_PTR)MainWindowProc);
                    break;
            }
        }   break;

        /// -------------------
        case WM_TIMER: {
            GetCursorPos(&p); GetWindowRect(hwnd, &rect);
            if (!PtInRect(&rect, p)) {
                SetLayeredWindowAttributes(hwnd, 0x0, 0, LWA_COLORKEY);
            }

            if (rptr.st->click_through) {
                LONG_PTR style = GetWindowLong(rptr.window->hwnd, GWL_EXSTYLE);
                style &= ~WS_EX_TRANSPARENT;
                rptr.st->click_through = 0;

                SetWindowLong(rptr.window->hwnd, GWL_EXSTYLE, style);
            }

            if (rptr.st->ignore_input) {
                DragAcceptFiles(hwnd, FALSE);
                SetWindowLongPtr(rptr.window->hwnd, GWLP_WNDPROC, (LONG_PTR)EscapeWindowProc);
            }
        }   break;

        default: break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK PopupMenuProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static POINT p;
    static RECT rect, res;
    static RECT wcrect, pbrect, imrect, mwtrect;
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
            pthread_detach(thread_t);
            SetTimer(hwnd, 1, 100, NULL);
        }   break;

        /// -------------------
        case WM_DESTROY: {
            rptr.rd->render_thread = 0;
            pthread_join(thread_t, NULL);
            rptr.rd->render_thread = 1;

            GetWindowRect(rptr.window->hwnd, &res);
            rptr.st->size = rptr.st->trackbar_size;

            if (rptr.dt->width * rptr.st->size < 10)
                rptr.st->size = (10 / (float)rptr.dt->width);
            if (rptr.dt->height * rptr.st->size < 10)
                rptr.st->size = (10 / (float)rptr.dt->height);

            rptr.rd->loading = 1;

            switch (rptr.rd->pos) {
                case POS_LTC:
                    SetWindowPos(rptr.window->hwnd,
                                 (rptr.st->topmost) ? HWND_TOPMOST : HWND_NOTOPMOST,
                                 0,
                                 0,
                                 _GetCollisionSize(rptr.dt->width, rptr.st->size),
                                 _GetCollisionSize(rptr.dt->height, rptr.st->size), SWP_NOMOVE | SWP_NOREDRAW);
                    break;
                case POS_LLC:
                    SetWindowPos(rptr.window->hwnd,
                                 (rptr.st->topmost) ? HWND_TOPMOST : HWND_NOTOPMOST,
                                 res.left,
                                 res.top + (rptr.dt->height * 2 - ((float)rptr.dt->height * rptr.st->size + 0.5)),
                                 _GetCollisionSize(rptr.dt->width, rptr.st->size),
                                 _GetCollisionSize(rptr.dt->height, rptr.st->size), SWP_NOREDRAW);
                    break;
                case POS_RTC:
                    SetWindowPos(rptr.window->hwnd,
                                 (rptr.st->topmost) ? HWND_TOPMOST : HWND_NOTOPMOST,
                                 res.left + (rptr.dt->width * 2 - ((float)rptr.dt->width * rptr.st->size + 0.5)),
                                 res.top,
                                 _GetCollisionSize(rptr.dt->width, rptr.st->size),
                                 _GetCollisionSize(rptr.dt->height, rptr.st->size), SWP_NOREDRAW);
                    break;
                case POS_RLC:
                    SetWindowPos(rptr.window->hwnd,
                                 (rptr.st->topmost) ? HWND_TOPMOST : HWND_NOTOPMOST,
                                 res.left + (rptr.dt->width * 2 - ((float)rptr.dt->width * rptr.st->size + 0.5)),
                                 res.top + (rptr.dt->height * 2 - ((float)rptr.dt->height * rptr.st->size + 0.5)),
                                 _GetCollisionSize(rptr.dt->width, rptr.st->size),
                                 _GetCollisionSize(rptr.dt->height, rptr.st->size), SWP_NOREDRAW);
                    break;
                case POS_C:
                    SetWindowPos(rptr.window->hwnd,
                                 (rptr.st->topmost) ? HWND_TOPMOST : HWND_NOTOPMOST,
                                 res.left + (rptr.dt->width * 2 - ((float)rptr.dt->width * rptr.st->size + 0.5)) / 2,
                                 res.top + (rptr.dt->height * 2 - ((float)rptr.dt->height * rptr.st->size + 0.5)) / 2,
                                 _GetCollisionSize(rptr.dt->width, rptr.st->size),
                                 _GetCollisionSize(rptr.dt->height, rptr.st->size), SWP_NOREDRAW);
                    break;
            }
            rptr.rd->loading = 0;

            DragAcceptFiles(rptr.window->hwnd, TRUE);
            GetWindowRect(rptr.window->hwnd, &res);
            rptr.st->x = res.left; rptr.st->y = res.top;
            WriteSettings(rptr.st);

            rptr.window_popup->isactive = 0;
            PostQuitMessage(0);
        }   break;

        /// -------------------
        case WM_CLOSE: {
            if (rptr.window_wc->isactive) PostMessage(rptr.window_wc->hwnd, WM_CLOSE, 0, 0);
            if (rptr.window_pb->isactive) PostMessage(rptr.window_pb->hwnd, WM_CLOSE, 0, 0);
            if (rptr.window_im->isactive) PostMessage(rptr.window_im->hwnd, WM_CLOSE, 0, 0);
            if (rptr.window_pw->isactive) PostMessage(rptr.window_pw->hwnd, WM_CLOSE, 0, 0);
            if (rptr.window_mwt->isactive) PostMessage(rptr.window_mwt->hwnd, WM_CLOSE, 0, 0);

            DestroyWindow(hwnd);
        }   break;

        /// -------------------
        case WM_ACTIVATE: {
            GetCursorPos(&p);
            GetWindowRect(rptr.window_popup->hwnd, &rect);
            GetWindowRect(rptr.window_wc->hwnd, &wcrect);
            GetWindowRect(rptr.window_pb->hwnd, &pbrect);
            GetWindowRect(rptr.window_im->hwnd, &imrect);
            GetWindowRect(rptr.window_mwt->hwnd, &mwtrect);

            if (wparam != WA_INACTIVE ||
                IsChild(rptr.window_popup->hwnd, (HWND)lparam) ||
                (HWND)lparam == hwnd ||
                PtInRect(&rect, p) ||
                (PtInRect(&wcrect, p) && rptr.window_wc->isactive) ||
                (PtInRect(&pbrect, p) && rptr.window_pb->isactive) ||
                (PtInRect(&imrect, p) && rptr.window_im->isactive) ||
                (PtInRect(&mwtrect, p) && rptr.window_mwt->isactive)) { break; }

            PostMessage(rptr.window_popup->hwnd, WM_CLOSE, 0, 0);
        }   break;

        /// -------------------
        case WM_LBUTTONDOWN: {
            rptr.rd->render_thread = 1; pthread_create(&thread_t, 0, RenderThread, &rptr);

            SetFocus(rptr.window_popup->hwnd);
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

            rptr.rd->render_thread = 0; pthread_join(thread_t, NULL);
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

                    LoadWindow(rptr.window_pb, rptr.st, NULL, rptr.rd, "window_pb", popup_left, popup_top, PB_WIDTH, PB_HEIGHT, POPUP_MENU_MINOR);

                    if (IsWindow(rptr.window_im->hwnd))
                        PostMessage(rptr.window_im->hwnd, WM_CLOSE, 0, 0);
                    if (IsWindow(rptr.window_mwt->hwnd))
                        PostMessage(rptr.window_mwt->hwnd, WM_CLOSE, 0, 0);
                    if (IsWindow(rptr.window_pw->hwnd))
                        PostMessage(rptr.window_pw->hwnd, WM_CLOSE, 0, 0);
                    if (IsWindow(rptr.window_wc->hwnd))
                        PostMessage(rptr.window_wc->hwnd, WM_CLOSE, 0, 0);

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
                    /// LINE
                    LoadButton(rptr.label_speed, rptr.window_pb,
                               4, y += 38, PB_WIDTH - 10, 25, 15,
                               " ", 4, "Segoe UI");

                    LoadTrackBar(rptr.trackbar_speed, rptr.window_pb,
                         13, y += 27, PB_WIDTH - 26, 24, 0,
                         1, 36, rptr.st->speed, 5);
                    /// LINE
                    LoadButton(rptr.btn_frame_updates, rptr.window_pb,
                               4, y += 38, PB_WIDTH - 10, 25, 15,
                               " ", 6, "Segoe UI");
                }
            }
            else PostMessage(rptr.window_pb->hwnd, WM_USER, 1, 0);

            if (_IsButtonHovered(rptr.label_interaction, &p, 1)) {
                if (rptr.window_im->isactive == 0) {
                    GetWindowRect(rptr.window_popup->hwnd, &rect);

                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);
                    int offset = 84;
                    int popup_left = rect.left + POPUP_WIDTH;
                    int popup_top = rect.top + offset;
                    if (popup_left + IM_WIDTH > res.right) popup_left = rect.left - IM_WIDTH;
                    if (popup_top + IM_HEIGHT > res.bottom) popup_top = res.bottom - IM_HEIGHT;

                    LoadWindow(rptr.window_im, rptr.st, NULL, rptr.rd, "window_im", popup_left, popup_top, IM_WIDTH, IM_HEIGHT, POPUP_MENU_MINOR);

                    if (IsWindow(rptr.window_mwt->hwnd))
                        PostMessage(rptr.window_mwt->hwnd, WM_CLOSE, 0, 0);
                    if (IsWindow(rptr.window_pb->hwnd))
                        PostMessage(rptr.window_pb->hwnd, WM_CLOSE, 0, 0);
                    if (IsWindow(rptr.window_pw->hwnd))
                        PostMessage(rptr.window_pw->hwnd, WM_CLOSE, 0, 0);
                    if (IsWindow(rptr.window_wc->hwnd))
                        PostMessage(rptr.window_wc->hwnd, WM_CLOSE, 0, 0);

                    HRGN rgn = CreateRoundRectRgn(0, 0, IM_WIDTH, IM_HEIGHT, 5, 5);
                    SetWindowRgn(rptr.window_im->hwnd, rgn, TRUE);

                    int y = 5;

                    LoadButton(rptr.btn_disable_moving, rptr.window_im,
                               5, y, IM_WIDTH - 10, 25, 15,
                               " ", 1, "Segoe UI");
                    LoadButton(rptr.btn_hide_hover, rptr.window_im,
                               5, y += 25, IM_WIDTH - 10, 25, 15,
                               " ", 2, "Segoe UI");
                    LoadButton(rptr.btn_click_through, rptr.window_im,
                               5, y += 25, IM_WIDTH - 10, 25, 15,
                               " ", 3, "Segoe UI");
                    LoadButton(rptr.btn_ignore_input, rptr.window_im,
                               5, y += 25, IM_WIDTH - 10, 25, 15,
                               " ", 4, "Segoe UI");
                }
            }
            else PostMessage(rptr.window_im->hwnd, WM_USER, 1, 0);

            if (_IsButtonHovered(rptr.label_window_scale, &p, 1)) {
                if (rptr.window_wc->isactive == 0) {
                    GetWindowRect(rptr.window_popup->hwnd, &rect);

                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);
                    int offset = 109;
                    int popup_left = rect.left + POPUP_WIDTH;
                    int popup_top = rect.top + offset;
                    if (popup_left + WC_WIDTH > res.right) popup_left = rect.left - WC_WIDTH;
                    if (popup_top + WC_HEIGHT > res.bottom) popup_top = res.bottom - WC_HEIGHT;

                    LoadWindow(rptr.window_wc, rptr.st, NULL, rptr.rd, "window_wc", popup_left, popup_top, WC_WIDTH, WC_HEIGHT, POPUP_MENU_MINOR);

                    if (IsWindow(rptr.window_im->hwnd))
                        PostMessage(rptr.window_im->hwnd, WM_CLOSE, 0, 0);
                    if (IsWindow(rptr.window_mwt->hwnd))
                        PostMessage(rptr.window_mwt->hwnd, WM_CLOSE, 0, 0);
                    if (IsWindow(rptr.window_pb->hwnd))
                        PostMessage(rptr.window_pb->hwnd, WM_CLOSE, 0, 0);
                    if (IsWindow(rptr.window_pw->hwnd))
                        PostMessage(rptr.window_pw->hwnd, WM_CLOSE, 0, 0);

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

//                  LINE
//                    LoadButton(rptr.label_transparency, rptr.window_wc,
//                               4, y += 28, WC_WIDTH - 10, 25, 15,
//                               " ", 5, "Segoe UI");

//                    LoadTrackBar(rptr.trackbar_transparency, rptr.window_wc,
//                         13, y += 27, WC_WIDTH - 27, 24, 0,
//                         1, 100, (int)((float)rptr.st->transparency / 2.55), 6);

                    /// LINE

                    LoadButton(rptr.btn_pin_top, rptr.window_wc,
                               4, y += 38, WC_WIDTH - 10, 25, 15,
                               " ", 7, "Segoe UI");

                    LoadButton(rptr.btn_taskbar, rptr.window_wc,
                               4, y += 25, WC_WIDTH - 10, 25, 15,
                               " ", 8, "Segoe UI");
                }
            }
            else PostMessage(rptr.window_wc->hwnd, WM_USER, 1, 0);

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

                    LoadWindow(rptr.window_mwt, rptr.st, NULL, rptr.rd, "window_mwt", popup_left, popup_top, MWT_WIDTH, MWT_HEIGHT, POPUP_MENU_MINOR);

                    if (IsWindow(rptr.window_im->hwnd))
                        PostMessage(rptr.window_im->hwnd, WM_CLOSE, 0, 0);
                    if (IsWindow(rptr.window_pb->hwnd))
                        PostMessage(rptr.window_pb->hwnd, WM_CLOSE, 0, 0);
                    if (IsWindow(rptr.window_pw->hwnd))
                        PostMessage(rptr.window_pw->hwnd, WM_CLOSE, 0, 0);
                    if (IsWindow(rptr.window_wc->hwnd))
                        PostMessage(rptr.window_wc->hwnd, WM_CLOSE, 0, 0);

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

            // Lines
            rect.left = 10; rect.top = 34; rect.right = POPUP_WIDTH - 10; rect.bottom = rect.top + 1;
            brush = CreateSolidBrush(RGB(210, 210, 210));
            FillRect(hdc, &rect, brush);
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
                _InvalidateButton(item, rptr.btn_title, APP_NAME_VER, 35, L"", 0, 0);

            else if (item->hwndItem == rptr.btn_openfile->hwnd)
                _InvalidateButton(item, rptr.btn_openfile, "Open file...", 35, L"", 0, 0);

            else if (item->hwndItem == rptr.label_playback->hwnd)
                _InvalidateButton(item, rptr.label_playback, "Playback", 35, L"", 0, 0x2);

            else if (item->hwndItem == rptr.label_interaction->hwnd)
                _InvalidateButton(item, rptr.label_interaction, "Interaction", 35, L"", 0, 0x2);

            else if (item->hwndItem == rptr.label_window_scale->hwnd)
                _InvalidateButton(item, rptr.label_window_scale, "Window", 35, L"", 0, 0x2);

            else if (item->hwndItem == rptr.label_move_window->hwnd)
                _InvalidateButton(item, rptr.label_move_window, "Move window to", 35, L"", 0, 0x2);

            else if (item->hwndItem == rptr.btn_close_window->hwnd)
                _InvalidateButton(item, rptr.btn_close_window, "Close window", 35, L"", 0, 0);

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
                    pthread_create(&thread_t, 0, ShowLink, NULL);
                    pthread_detach(thread_t);
                    printf("Браузер открыт!\n");
                }   break;

                /// Open file...
                case 2: {
                    if ((HWND)lparam == rptr.btn_openfile->hwnd) {
                        PostMessage(rptr.window->hwnd, WM_COMMAND, 1, 0);
                        ReleaseWindow(rptr.window_popup);
                    }
                }   break;

                /// Close window
                case 8: {
                    PostMessage(rptr.window->hwnd, WM_CLOSE, 0, 0);
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
            }
        }   break;

        /// -------------------
        case WM_TIMER: {
            GetCursorPos(&p);

            _IsButtonHovered(rptr.btn_title, &p, 0);
            _IsButtonHovered(rptr.btn_openfile, &p, 0);
            _IsButtonHovered(rptr.btn_close_window, &p, 0);
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
    static TRACKMOUSEEVENT tme;
    static BOOL tracking = FALSE;

    switch (msg) {
        /// -------------------
        case WM_CREATE: {
            pthread_create(&thread_t, 0, ShowLowerPopupThread, hwnd);
            pthread_detach(thread_t);
            SetTimer(hwnd, 1, 100, NULL);
        }   break;

        /// -------------------
        case WM_DESTROY: {
            PostQuitMessage(0);
        }   break;

        /// -------------------
        case WM_CLOSE: {
            DestroyWindow(hwnd);
            SetActiveWindow(rptr.window_popup->hwnd);
            rptr.window_pb->isactive = 0;
        }   break;

        /// -------------------
        case WM_PAINT: {
            hdc = BeginPaint(rptr.window_pb->hwnd, &ps);

            GetClientRect(rptr.window_pb->hwnd, &rect);
            brush = CreateSolidBrush(RGB(250, 250, 250));
            FillRect(hdc, &rect, brush);

            // Lines
            rect.left = 10; rect.top = 90; rect.right = PB_WIDTH - 10; rect.bottom = rect.top + 1; // 57
            brush = CreateSolidBrush(RGB(210, 210, 210));
            FillRect(hdc, &rect, brush);

            rect.left = 10; rect.top = 155; rect.right = PB_WIDTH - 10; rect.bottom = rect.top + 1; // 57
            brush = CreateSolidBrush(RGB(210, 210, 210));
            FillRect(hdc, &rect, brush);

            DeleteObject(brush);

            GetClientRect(rptr.window_pb->hwnd, &rect);
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
            SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_ARROW));

            GetCursorPos(&p);

//            _IsButtonHovered(rptr.label_frames, &p, 0);
            _IsButtonHovered(rptr.btn_prev_frame, &p, 0);
            _IsButtonHovered(rptr.btn_play, &p, 0);
            _IsButtonHovered(rptr.btn_next_frame, &p, 0);
            _IsButtonHovered(rptr.btn_frame_updates, &p, 0);
//            _IsButtonHovered(rptr.label_speed, &p, 0);
//            _IsButtonHovered(rptr.btn_slow_rewind, &p, 0);
        }   return 0;

        /// -------------------
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT)lparam;

            if (item->hwndItem == rptr.label_frames->hwnd) {
                sprintf(rptr.st->str_frame, "Frame (%d/%d)", rptr.rd->frame + 1, rptr.dt->count);
                _InvalidateButton(item, rptr.label_frames, rptr.st->str_frame, 35, L"", 0, 0);
            }
            if (item->hwndItem == rptr.btn_prev_frame->hwnd) {
                _InvalidateButton(item, rptr.btn_prev_frame, "", 30, L"", 0, 0x4);
            }
            if (item->hwndItem == rptr.btn_play->hwnd) {
                _InvalidateButton(item, rptr.btn_play, "", 28, L"", 0, 0x10);
            }
            if (item->hwndItem == rptr.btn_next_frame->hwnd) {
                _InvalidateButton(item, rptr.btn_next_frame, "", 21, L"", 0, 0x8);
            }
            if (item->hwndItem == rptr.label_speed->hwnd) {
                setlocale(LC_NUMERIC, "C");

                if (rptr.st->speed == 16 || rptr.st->speed == 36)
                    sprintf(rptr.st->str_speed, "Speed (%.0fx)", (float)rptr.st->speed * 0.05 + 0.2);
                else if (rptr.st->speed % 2)
                    sprintf(rptr.st->str_speed, "Speed (%.2fx)", (float)rptr.st->speed * 0.05 + 0.2);
                else
                    sprintf(rptr.st->str_speed, "Speed (%.1fx)", (float)rptr.st->speed * 0.05 + 0.2);
                _InvalidateButton(item, rptr.label_speed, rptr.st->str_speed, 35, L"", 0, 0);

                setlocale(LC_ALL, "Russian");
            }
            if (item->hwndItem == rptr.btn_frame_updates->hwnd) {
                _InvalidateButton(item, rptr.btn_frame_updates, "Show frame updates", 35, L"GIF, PNG", 147, (uint8_t)rptr.st->sfu);
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
        }   break;

        /// -------------------
        case WM_MOUSEMOVE: {
            if (!tracking) {
                tme.cbSize = sizeof(tme);
                tme.hwndTrack = rptr.window_pb->hwnd;
                tme.dwFlags = TME_LEAVE;

                if (_TrackMouseEvent(&tme))
                    tracking = TRUE;
            }
        }   break;

        /// -------------------
        case WM_MOUSELEAVE: {
            tracking = FALSE;

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
        case WM_NCMOUSEMOVE: {
            tracking = FALSE;
        }   break;

        /// -------------------
        case WM_TIMER: {
            InvalidateRect(rptr.label_frames->hwnd, NULL, TRUE);
            SendMessage(rptr.trackbar_frames->hwnd, TBM_SETPOS, TRUE, rptr.rd->frame + 1);
            switch (wparam) {
                case 1:
                    GetCursorPos(&p);
                    GetWindowRect(rptr.window_pb->hwnd, &wrect);
                    GetWindowRect(rptr.label_playback->hwnd, &brect);
                    GetWindowRect(rptr.window_popup->hwnd, &prect);

                    if (prect.left > wrect.left) brect.left -= 5;
                    else brect.right += 5;

                    if (!PtInRect(&wrect, p) && !PtInRect(&brect, p) &&
                        GetCapture() != rptr.trackbar_frames->hwnd &&
                        GetCapture() != rptr.trackbar_speed->hwnd) {
                        PostMessage(rptr.window_pb->hwnd, WM_CLOSE, 0, 0);
                        ReleaseHover(&rptr, rptr.label_playback->hwnd);
                    }
                    break;
            }
        }

        /// -------------------
        case WM_HSCROLL: {
            if ((HWND)lparam == rptr.trackbar_frames->hwnd) {
                SetFocus(rptr.window_popup->hwnd);
                rptr.rd->frame = SendMessage(rptr.trackbar_frames->hwnd, TBM_GETPOS, 0, 0) - 1;
                rptr.rd->framed_trackbar = 1; ShowFrame(rptr.window, rptr.dt, rptr.rd, rptr.st, 1); rptr.rd->framed_trackbar = 0;
            }
            if ((HWND)lparam == rptr.trackbar_speed->hwnd) {
                SetFocus(rptr.window_popup->hwnd);
                rptr.st->speed = SendMessage(rptr.trackbar_speed->hwnd, TBM_GETPOS, 0, 0);

                printf("frame: %d [%f]\n", GetCurrentFrame(rptr.dt, rptr.rd, rptr.st), rptr.dt->lengths[GetCurrentFrame(rptr.dt, rptr.rd, rptr.st)]);
                rptr.rd->start_time = GetTime() - (double)rptr.dt->lengths[rptr.rd->frame] / ((double)rptr.st->speed * 0.05 + 0.2);
                ShowFrame(rptr.window, rptr.dt, rptr.rd, rptr.st, 1);

                InvalidateRect(rptr.label_speed->hwnd, NULL, TRUE);
            }
            WriteSettings(rptr.st);
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
                    rptr.rd->start_time = GetTime() - (double)rptr.dt->lengths[rptr.rd->frame] / ((double)rptr.st->speed * 0.05 + 0.2);
                    ShowFrame(rptr.window, rptr.dt, rptr.rd, rptr.st, 1);
                }   break;

                case 3:
                case 327683: {
                    if (rptr.rd->change_frames) rptr.rd->change_frames = 0;
                    else rptr.rd->change_frames = 1;

                    InvalidateRect(rptr.label_frames->hwnd, NULL, TRUE);
                    SendMessage(rptr.trackbar_frames->hwnd, TBM_SETPOS, TRUE, rptr.rd->frame + 1);
                    rptr.rd->start_time = GetTime() - (double)rptr.dt->lengths[rptr.rd->frame] / ((double)rptr.st->speed * 0.05 + 0.2);

                    SetFocus(rptr.window_popup->hwnd);
                }   break;

                case 4:
                case 327684: {
                    if (++rptr.rd->frame > rptr.dt->count - 1) rptr.rd->frame = 0;
                    rptr.rd->start_time = GetTime() - (double)rptr.dt->lengths[rptr.rd->frame] / ((double)rptr.st->speed * 0.05 + 0.2);
                    ShowFrame(rptr.window, rptr.dt, rptr.rd, rptr.st, 1);
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
    static TRACKMOUSEEVENT tme;
    static BOOL tracking = FALSE;

    switch (msg) {
        /// -------------------
        case WM_CREATE: {
            pthread_create(&thread_t, 0, ShowLowerPopupThread, hwnd);
            pthread_detach(thread_t);
            SetTimer(hwnd, 1, 100, NULL);
        }   break;

        /// -------------------
        case WM_DESTROY: {
            PostQuitMessage(0);
        }   break;

        /// -------------------
        case WM_CLOSE: {
            DestroyWindow(hwnd);
            SetActiveWindow(rptr.window_popup->hwnd);
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
        case WM_SETCURSOR: {
            SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_ARROW));

            GetCursorPos(&p);

            if (!rptr.st->click_through)
                _IsButtonHovered(rptr.btn_disable_moving, &p, 0);
            _IsButtonHovered(rptr.btn_hide_hover, &p, 0);
            _IsButtonHovered(rptr.btn_click_through, &p, 0);
            if (!rptr.st->click_through)
                _IsButtonHovered(rptr.btn_ignore_input, &p, 0);
        }   return 0;

        /// -------------------
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT)lparam;

            if (item->hwndItem == rptr.btn_disable_moving->hwnd) {
                _InvalidateButton(item, rptr.btn_disable_moving, "Disable moving", 35, L"", 0,
                                  (rptr.st->click_through ? 0x20 : 0) | rptr.st->disable_moving | rptr.st->click_through);
            }
            if (item->hwndItem == rptr.btn_hide_hover->hwnd) {
                _InvalidateButton(item, rptr.btn_hide_hover, "Hide on hover", 35, L"", 0, rptr.st->hide_on_hover);
            }
            if (item->hwndItem == rptr.btn_click_through->hwnd) {
                _InvalidateButton(item, rptr.btn_click_through, "Click-through", 35, L"Ctrl + RMB", 114, rptr.st->click_through);
            }
            if (item->hwndItem == rptr.btn_ignore_input->hwnd) {
                _InvalidateButton(item, rptr.btn_ignore_input, "Ignore all input", 35,
                                  (rptr.st->click_through) ? L"" : L"ESC to disable",
                                  (rptr.st->click_through) ? 0 : 120, (rptr.st->click_through) ? 0x20 : 0);
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
            if (!tracking) {
                tme.cbSize = sizeof(tme);
                tme.hwndTrack = rptr.window_im->hwnd;
                tme.dwFlags = TME_LEAVE;

                if (_TrackMouseEvent(&tme))
                    tracking = TRUE;
            }
        }   break;

        /// -------------------
        case WM_MOUSELEAVE: {
            tracking = FALSE;

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
        case WM_NCMOUSEMOVE: {
            tracking = FALSE;
        }   break;

        /// -------------------
        case WM_TIMER: {
            switch (wparam) {
                case 1:
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
                    break;
            }
        }

        /// -------------------
        case WM_UPDATE_ALPHA: {
            uint8_t alpha = (uint8_t)lparam;
            if (alpha > 0) SetLayeredWindowAttributes(hwnd, 0x0, alpha, LWA_ALPHA);
        }   break;

        /// -------------------
        case WM_COMMAND: {
            switch (wparam) {
                /// -------------------
                case 1: {
                    if (rptr.st->click_through) break;
                    if (rptr.st->disable_moving)
                        rptr.st->disable_moving = 0;
                    else
                        rptr.st->disable_moving = 1;

                    PostMessage(rptr.window_popup->hwnd, WM_CLOSE, 0, 0);
                }   break;
                /// -------------------
                case 2:
                    if (rptr.st->hide_on_hover)
                        rptr.st->hide_on_hover = 0;
                    else
                        rptr.st->hide_on_hover = 1;

                    PostMessage(rptr.window_popup->hwnd, WM_CLOSE, 0, 0);
                    break;

                /// -------------------
                case 3:
                    LONG_PTR style = GetWindowLong(rptr.window->hwnd, GWL_EXSTYLE);

                    if (rptr.st->click_through) {
                        style &= ~WS_EX_TRANSPARENT;
                        rptr.st->click_through = 0;
                    }
                    else {
                        style |= WS_EX_TRANSPARENT;
                        rptr.st->click_through = 1;
                    }

                    SetWindowLong(rptr.window->hwnd, GWL_EXSTYLE, style);
                    PostMessage(rptr.window_popup->hwnd, WM_CLOSE, 0, 0);
                    break;

                /// -------------------
                case 4: {
                    if (rptr.st->click_through) break;
                    SetWindowLongPtr(rptr.window->hwnd, GWLP_WNDPROC, (LONG_PTR)EscapeWindowProc);
                    PostMessage(rptr.window_popup->hwnd, WM_CLOSE, 0, 0);
                    rptr.st->ignore_input = 1;
                    WriteSettings(rptr.st);
                }   break;
            }
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
    static TRACKMOUSEEVENT tme;
    static BOOL tracking = FALSE;
    static uint8_t ts = 0;

    switch (msg) {
        /// -------------------
        case WM_CREATE: {
            pthread_create(&thread_t, 0, ShowLowerPopupThread, hwnd);
            pthread_detach(thread_t);
            SetTimer(hwnd, 1, 100, NULL);
        }   break;

        /// -------------------
        case WM_DESTROY: {
            PostQuitMessage(0);
        }   break;

        /// -------------------
        case WM_CLOSE: {
            DestroyWindow(hwnd);
            SetActiveWindow(rptr.window_popup->hwnd);
            rptr.window_wc->isactive = 0;
        }   break;

        /// -------------------
        case WM_PAINT: {
            hdc = BeginPaint(rptr.window_wc->hwnd, &ps);

            GetClientRect(rptr.window_wc->hwnd, &rect);
            brush = CreateSolidBrush(RGB(250, 250, 250));
            FillRect(hdc, &rect, brush);

            // Lines
            rect.left = 10; rect.top = 65; rect.right = PB_WIDTH - 10; rect.bottom = rect.top + 1;
            brush = CreateSolidBrush(RGB(210, 210, 210));
            FillRect(hdc, &rect, brush);

            DeleteObject(brush);

            pen = CreatePen(PS_SOLID, 3, RGB(238, 238, 238));
            prev_pen = (HPEN)SelectObject(hdc, pen);
            prev_brush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

            GetClientRect(rptr.window_wc->hwnd, &rect);
            RoundRect(hdc, 0, 0, rect.right - 1, rect.bottom - 1, 10, 10);
            SelectObject(hdc, prev_pen);
            SelectObject(hdc, prev_brush);
            DeleteObject(pen);

            EndPaint(rptr.window_wc->hwnd, &ps);
        }   break;

        /// -------------------
        case WM_CTLCOLORSTATIC: {
            brush = CreateSolidBrush(RGB(250, 250, 250));

            if (GetDlgCtrlID((HWND)lparam) == 4 || GetDlgCtrlID((HWND)lparam) == 6) {
                SetBkMode((HDC)wparam, TRANSPARENT);
                return (LRESULT)brush;
            }
        }   break;

        /// -------------------
        case WM_SETCURSOR: {
            SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_ARROW));

            GetCursorPos(&p);

            _IsButtonHovered(rptr.btn_add_scale, &p, 0);
            _IsButtonHovered(rptr.btn_subtract_scale, &p, 0);
//            _IsButtonHovered(rptr.label_transparency, &p, 0);
            _IsButtonHovered(rptr.btn_pin_top, &p, 0);
            _IsButtonHovered(rptr.btn_taskbar, &p, 0);
        }   return 0;

        /// -------------------
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT)lparam;

            if (item->hwndItem == rptr.label_scale->hwnd) {
                sprintf(rptr.st->str_size, "Scale (%.0f%%)", rptr.st->trackbar_size * 100);
                _InvalidateButton(item, rptr.label_scale, rptr.st->str_size, 35, L"", 0, 0);
            }

            if (item->hwndItem == rptr.btn_add_scale->hwnd)
                _InvalidateButton(item, rptr.btn_add_scale, "+", 12, L"", 0, 0);

            if (item->hwndItem == rptr.btn_subtract_scale->hwnd)
                _InvalidateButton(item, rptr.btn_subtract_scale, "-", 12, L"", 0, 0);

//            if (item->hwndItem == rptr.label_transparency->hwnd) {
//                sprintf(rptr.st->str_transparency, "Transparency (%u%%)", (uint8_t)((float)rptr.st->transparency / 2.55));
//                _InvalidateButton(item, rptr.label_transparency, rptr.st->str_transparency, 35, 0);
//            }

            if (item->hwndItem == rptr.btn_pin_top->hwnd)
                _InvalidateButton(item, rptr.btn_pin_top, "Always on top", 35, L"", 0, (uint8_t)rptr.st->topmost);

            if (item->hwndItem == rptr.btn_taskbar->hwnd)
                _InvalidateButton(item, rptr.btn_taskbar, "Show taskbar icon", 35, L"", 0, (uint8_t)rptr.st->taskbar);
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
        }   break;

        /// -------------------
        case WM_MOUSEMOVE: {
            if (!tracking) {
                tme.cbSize = sizeof(tme);
                tme.hwndTrack = rptr.window_wc->hwnd;
                tme.dwFlags = TME_LEAVE;

                if (_TrackMouseEvent(&tme))
                    tracking = TRUE;
            }
        }   break;

        /// -------------------
        case WM_MOUSELEAVE: {
            tracking = FALSE;

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
        case WM_NCMOUSEMOVE: {
            tracking = FALSE;
        }   break;

        /// -------------------
        case WM_TIMER: {
            switch (wparam) {
                case 1:
                    GetCursorPos(&p);
                    GetWindowRect(rptr.window_wc->hwnd, &wrect);
                    GetWindowRect(rptr.label_window_scale->hwnd, &brect);
                    GetWindowRect(rptr.window_popup->hwnd, &prect);

                    if (prect.left > wrect.left) brect.left -= 5;
                    else brect.right += 5;

                    if (!PtInRect(&wrect, p) && !PtInRect(&brect, p) &&
//                        GetCapture() != rptr.trackbar_transparency->hwnd &&
                        GetCapture() != rptr.trackbar_scale->hwnd) {
                        PostMessage(rptr.window_wc->hwnd, WM_CLOSE, 0, 0);
                        ReleaseHover(&rptr, rptr.label_window_scale->hwnd);
                    }
                    break;
            }
        }

        /// -------------------
        case WM_UPDATE_ALPHA: {
            uint8_t alpha = (uint8_t)lparam;
            if (alpha > 0) SetLayeredWindowAttributes(hwnd, 0x0, alpha, LWA_ALPHA);
        }   break;

        /// -------------------
        case WM_HSCROLL: {
            if ((HWND)lparam == rptr.trackbar_scale->hwnd) {
                rptr.st->pos = SendMessage(rptr.trackbar_scale->hwnd, TBM_GETPOS, 0, 0);

                if (rptr.st->pos == 100)
                    _ChangeTexFilt(rptr.window, rptr.dt, GL_NEAREST);
                else
                    _ChangeTexFilt(rptr.window, rptr.dt, GL_LINEAR);

                _ChangeScaleTrackBar(rptr.window, rptr.window_wc,
                                     rptr.trackbar_scale, rptr.label_scale,
                                     rptr.st, rptr.dt, rptr.rd,
                                     rptr.st->pos);
            }
//            if ((HWND)lparam == rptr.trackbar_transparency->hwnd) {
//                rptr.st->transparency = (uint8_t)(((float)SendMessage(rptr.trackbar_transparency->hwnd, TBM_GETPOS, 0, 0)) * 2.55 + 0.555555555);
//                SetLayeredWindowAttributes(rptr.window->hwnd, 0x0, rptr.st->transparency, LWA_ALPHA);
//
//                InvalidateRect(rptr.label_transparency->hwnd, NULL, TRUE);
//            }
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

                        InvalidateRect(rptr.label_frames->hwnd, NULL, TRUE);
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

                        InvalidateRect(rptr.label_frames->hwnd, NULL, TRUE);
                        PostMessage(rptr.trackbar_scale->hwnd, TBM_SETPOS, TRUE, rptr.st->pos);
                    }
                }   break;

                case 7: {
                     if (rptr.st->topmost) rptr.st->topmost = 0;
                     else rptr.st->topmost = 1;

                     InvalidateRect(rptr.btn_pin_top->hwnd, NULL, TRUE);
                }   break;

                case 8: {
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
    static TRACKMOUSEEVENT tme;
    static BOOL tracking = FALSE;

    switch (msg)
    {
        /// -------------------
        case WM_CREATE: {
            pthread_create(&thread_t, 0, ShowLowerPopupThread, hwnd);
            pthread_detach(thread_t);
            SetTimer(hwnd, 1, 100, NULL);
        }   break;

        /// -------------------
        case WM_DESTROY: {
            PostQuitMessage(0);
        }   break;

        /// -------------------
        case WM_CLOSE: {
            DestroyWindow(hwnd);
            SetActiveWindow(rptr.window_popup->hwnd);
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
            SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_ARROW));

            GetCursorPos(&p);

            _IsButtonHovered(rptr.btn_move_topleft, &p, 0);
            _IsButtonHovered(rptr.btn_move_topright, &p, 0);
            _IsButtonHovered(rptr.btn_move_center, &p, 0);
            _IsButtonHovered(rptr.btn_move_left, &p, 0);
            _IsButtonHovered(rptr.btn_move_right, &p, 0);
        }   return 0;

        /// -------------------
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT)lparam;

            if (item->hwndItem == rptr.btn_move_topleft->hwnd)
                _InvalidateButton(item, rptr.btn_move_topleft, "Top left corner", 35, L"", 0, 0);

            if (item->hwndItem == rptr.btn_move_topright->hwnd)
                _InvalidateButton(item, rptr.btn_move_topright, "Top right corner", 35, L"", 0, 0);

            if (item->hwndItem == rptr.btn_move_center->hwnd)
                _InvalidateButton(item, rptr.btn_move_center, "Center", 35, L"", 0, 0);

            if (item->hwndItem == rptr.btn_move_left->hwnd)
                _InvalidateButton(item, rptr.btn_move_left, "Bottom left corner", 35, L"", 0, 0);

            if (item->hwndItem == rptr.btn_move_right->hwnd)
                _InvalidateButton(item, rptr.btn_move_right, "Bottom right corner", 35, L"", 0, 0);
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
//            GetCursorPos(&p);
//            GetWindowRect(rptr.window_mwt->hwnd, &rect);
//
//            if (wparam != WA_INACTIVE ||
//                IsChild(rptr.window_mwt->hwnd, (HWND)lparam) ||
//                (HWND)lparam == hwnd ||
//                PtInRect(&rect, p)) break;
//
//            PostMessage(rptr.window_mwt->hwnd, WM_CLOSE, 0, 0);
        }   break;

        /// -------------------
        case WM_MOUSEMOVE: {
            if (!tracking) {
                tme.cbSize = sizeof(tme);
                tme.hwndTrack = rptr.window_mwt->hwnd;
                tme.dwFlags = TME_LEAVE;

                if (_TrackMouseEvent(&tme))
                    tracking = TRUE;
            }
        }   break;

        /// -------------------
        case WM_MOUSELEAVE: {
            tracking = FALSE;

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
        case WM_NCMOUSEMOVE: {
            tracking = FALSE;
        }   break;

        /// -------------------
        case WM_TIMER: {
            switch (wparam) {
                case 1:
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
                    break;
            }
        }

        /** Commands **/

        /// -------------------
        case WM_UPDATE_ALPHA: {
            uint8_t alpha = (uint8_t)lparam;
            if (alpha > 0) SetLayeredWindowAttributes(hwnd, 0x0, alpha, LWA_ALPHA);
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
                                 SWP_NOSIZE | SWP_NOREDRAW);

                    GetWindowRect(rptr.window->hwnd, &res);
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
                    rptr.rd->pos = POS_LTC;
                }   break;

                /// -------------------
                case 2: {
                    GetWindowRect(rptr.window->hwnd, &res);
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
                    rptr.rd->pos = POS_RTC;

                    SetWindowPos(rptr.window->hwnd,
                                 NULL,
                                 GetSystemMetrics(SM_CXSCREEN) - rptr.dt->width * 2,
                                 0,
                                 0,
                                 0,
                                 SWP_NOSIZE | SWP_NOREDRAW);
                }   break;

                /// -------------------
                case 3: {
                    GetWindowRect(rptr.window->hwnd, &res);
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
                    rptr.rd->pos = POS_C;

                    SetWindowPos(rptr.window->hwnd,
                                 NULL,
                                 (rect.right - rect.left - rptr.dt->width * 2) / 2,
                                 (rect.bottom - rect.top - rptr.dt->height * 2) / 2,
                                 0,
                                 0,
                                 SWP_NOSIZE | SWP_NOREDRAW);
                }   break;

                /// -------------------
                case 4: {
                    GetWindowRect(rptr.window->hwnd, &res);
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
                    rptr.rd->pos = POS_LLC;

                    SetWindowPos(rptr.window->hwnd,
                                 NULL,
                                 0,
                                 rect.bottom - rect.top - (rptr.dt->height) * 2,
                                 0,
                                 0,
                                 SWP_NOSIZE | SWP_NOREDRAW);
                }   break;

                /// -------------------
                case 5: {
                    GetWindowRect(rptr.window->hwnd, &res);
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
                    rptr.rd->pos = POS_RLC;

                    SetWindowPos(rptr.window->hwnd,
                                 NULL,
                                 rect.right - rect.left - (rptr.dt->width) * 2,
                                 rect.bottom - rect.top - (rptr.dt->height) * 2,
                                 0,
                                 0,
                                 SWP_NOSIZE | SWP_NOREDRAW);
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
    rptr->btn_click_through->hovered = 0;
    rptr->btn_disable_moving->hovered = 0;
    rptr->btn_hide_hover->hovered = 0;
    rptr->btn_ignore_input->hovered = 0;
    rptr->btn_pin_top->hovered = 0;
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
                       const char* text, int left, LPCWSTR graytext, int gleft, uint8_t flag)
{
    static HDC hdc = NULL;
    static HFONT font = NULL;
    static RECT rect;
    rect = item->rcItem;
    hdc = item->hDC;

    SetBkMode(hdc, TRANSPARENT);
    HBRUSH hBrush = CreateSolidBrush(
        (button->hovered) ? BTN_HOVERED : BTN_HOVERED_NOT
    );
    FillRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);
    rect.left = left;
    if (flag & 0x20)
        SetTextColor(hdc, BTN_COLOR_TEXT_DEACTIVATED);
    else
        SetTextColor(hdc, BTN_COLOR_TEXT);
    DrawText(hdc, text, -1, &rect, DT_VCENTER | DT_SINGLELINE);

    if (gleft) {
        int size = 16;
        font = CreateFont(16, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY, DEFAULT_QUALITY, "Segoe UI");

        int y = rect.top + (rect.bottom - rect.top - size) / 2 + 1;

        SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, (flag & 0x20) ? BTN_COLOR_MARLETT_DEACTIVATED : BTN_COLOR_MARLETT);
        TextOutW(hdc, gleft, y, graytext, (int)wcslen(graytext));

        DeleteObject(font);
    }

    int size = 20;
    font = CreateFont(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH, "Marlett");

    // activated
    if (flag & 0x1) {
        int x = rect.left - 27;
        int y = rect.top + (rect.bottom - rect.top - size) / 2;

        SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, (flag & 0x20) ? BTN_COLOR_MARLETT_DEACTIVATED : BTN_COLOR_MARLETT);
        TextOutW(hdc, x, y, L"a", 1);

        DeleteObject(font);
    }
    // arrow
    if (flag & 0x2) {
        int x = rect.left + 176;
        int y = rect.top + (rect.bottom - rect.top - size) / 2;

        SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, (flag & 0x20) ? BTN_COLOR_MARLETT_DEACTIVATED : BTN_COLOR_MARLETT);
        TextOutW(hdc, x, y, L"8", 1);
        SetTextColor(hdc, BTN_HOVERED_NOT);
        TextOutW(hdc, x - 2, y, L"8", 1);

        DeleteObject(font);
    }
    // slow_left_rewind
    if (flag & 0x4) {
        size = 15;
        font = CreateFont(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH, "Marlett");
        int x = rect.left;
        int y = rect.top + (rect.bottom - rect.top - size) / 2 - 1;

        SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, (flag & 0x20) ? BTN_COLOR_MARLETT_DEACTIVATED : BTN_COLOR_MARLETT);

        TextOutW(hdc, x, y, L"g", 1);
        SetTextColor(hdc, (button->hovered) ? BTN_HOVERED : BTN_HOVERED_NOT);
        TextOutW(hdc, x + 2, y, L"g", 1);
        SetTextColor(hdc, (flag & 0x20) ? BTN_COLOR_MARLETT_DEACTIVATED : BTN_COLOR_MARLETT);

        size = 20;
        font = CreateFont(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH, "Marlett");

        SelectObject(hdc, font);
        y = rect.top + (rect.bottom - rect.top - size) / 2;

        TextOutW(hdc, x + 2, y, L"3", 1);

        DeleteObject(font);
    }
    // slow_right_rewind
    if (flag & 0x8) {
        size = 15;
        font = CreateFont(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH, "Marlett");
        int x = rect.left;
        int y = rect.top + (rect.bottom - rect.top - size) / 2 - 1;

        SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, (flag & 0x20) ? BTN_COLOR_MARLETT_DEACTIVATED : BTN_COLOR_MARLETT);

        TextOutW(hdc, x + 6, y, L"g", 1);
        SetTextColor(hdc, (button->hovered) ? BTN_HOVERED : BTN_HOVERED_NOT);
        TextOutW(hdc, x + 4, y, L"g", 1);
        SetTextColor(hdc, (flag & 0x20) ? BTN_COLOR_MARLETT_DEACTIVATED : BTN_COLOR_MARLETT);

        size = 20;
        font = CreateFont(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH, "Marlett");

        SelectObject(hdc, font);
        y = rect.top + (rect.bottom - rect.top - size) / 2;

        TextOutW(hdc, x, y, L"4", 1);

        DeleteObject(font);
    }
    // pause
    if (flag & 0x10) {
        int x = rect.left;
        int y = rect.top + (rect.bottom - rect.top - size) / 2;

        size = 18;
        font = CreateFont(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH, "Marlett");

        SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, (flag & 0x20) ? BTN_COLOR_MARLETT_DEACTIVATED : BTN_COLOR_MARLETT);
        TextOutW(hdc, x, y, L"g", 1);
        SetTextColor(hdc, (button->hovered) ? BTN_HOVERED : BTN_HOVERED_NOT);
        TextOutW(hdc, x + 3, y, L"g", 1);
        SetTextColor(hdc, (flag & 0x20) ? BTN_COLOR_MARLETT_DEACTIVATED : BTN_COLOR_MARLETT);
        TextOutW(hdc, x + 6, y, L"g", 1);
        SetTextColor(hdc, (button->hovered) ? BTN_HOVERED : BTN_HOVERED_NOT);
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
    return (n * size < 10.0) ? 10.0 : n * size;
}

void _ChangeScaleTrackBar(Window* window, Window* window_popup,
                          Trackbar* scale_trackbar, Button* scale_button,
                          Settings* st, Data* dt,
                          Render* rd, int pos)
{
    SetFocus(window_popup->hwnd);

    int settedpos = pos;
    if (dt->width * ((float)pos / 100) < WINDOW_MIN_SIZE)
        settedpos = (WINDOW_MIN_SIZE / (float)dt->width * 100);
    else if (dt->height * ((float)pos / 100) < WINDOW_MIN_SIZE)
        settedpos = (WINDOW_MIN_SIZE / (float)dt->height * 100);

    if (settedpos < 1) settedpos = 1;
    else if (settedpos > 200) settedpos = 200;

    st->trackbar_size = (float)settedpos / 100;

    sprintf(st->str_size, "Scale (%.0f%%)", st->trackbar_size * 100);
    SetWindowText(scale_button->hwnd, st->str_size);

    if (rd->change_frames)
    {
        rd->change_frames = 0; ShowFrame(window, dt, rd, st, 1); rd->change_frames = 1;
    }
    else
        ShowFrame(window, dt, rd, st, 1);
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

void* ShowLink()
{
    system("start https://github.com/repa214/GIFDesk/releases");
    return NULL;
}

/**
        OpenGL Proccessing
**/

void ShowFrame(Window* restrict window, Data* restrict dt, Render* restrict rd, Settings* restrict st, uint8_t skip)
{
    int cframe = rd->frame;
    if (rd->change_frames)
        rd->frame = GetCurrentFrame(dt, rd, st);
    if (cframe == rd->frame && !skip)
        return;

    wglMakeCurrent(window->hdc, window->hrc);

    RECT rect; GetWindowRect(window->hwnd, &rect);

//    printf("\33[2K\r");
//    printf("Frame: %d | [%ld, %ld] | [%f, %f] [%f, %f]", rd->frame + 1,
//           rect.right - rect.left,
//           rect.bottom - rect.top,
//           dt->frame_points[rd->frame * 4],
//           dt->frame_points[(rd->frame * 4) + 1],
//           dt->frame_points[(rd->frame * 4) + 2],
//           dt->frame_points[(rd->frame * 4) + 3]);

    if (rptr.window_popup->isactive)
        switch (rd->pos) {
            case POS_LTC:
                glViewport(0,
                           dt->height * 2 - dt->height * st->trackbar_size,
                           dt->width * st->trackbar_size,
                           dt->height * st->trackbar_size);
                break;
            case POS_LLC:
                glViewport(0,
                           0,
                           dt->width * st->trackbar_size,
                           dt->height * st->trackbar_size);
                break;
            case POS_RTC:
                glViewport(dt->width * 2 - dt->width * st->trackbar_size,
                           dt->height * 2 - dt->height * st->trackbar_size,
                           dt->width * st->trackbar_size,
                           dt->height * st->trackbar_size);
                break;
            case POS_RLC:
                glViewport(dt->width * 2 - dt->width * st->trackbar_size,
                           0,
                           dt->width * st->trackbar_size,
                           dt->height * st->trackbar_size);
                break;
            case POS_C:
                glViewport((dt->width * 2 - dt->width * st->trackbar_size) / 2,
                           (dt->height * 2 - dt->height * st->trackbar_size) / 2,
                           (dt->width * st->trackbar_size),
                           (dt->height * st->trackbar_size));
                break;
        }
    else
        glViewport(0, 0,
                   dt->width * st->trackbar_size,
                   dt->height * st->trackbar_size);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, dt->textures[rd->frame]);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glPushMatrix();

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(2, GL_FLOAT, 0, vertex);
        glTexCoordPointer(2, GL_FLOAT, 0, texCoord);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);

        if (st->sfu)
        {
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

    glPopMatrix();

    SwapBuffers(window->hdc);

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

void* RenderThread(void* arg)
{
    RenderPtr* rptr = (RenderPtr*)arg;

    MSG msg;
    while (rptr->rd->render_thread) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!IsWindow(rptr->window->hwnd)) continue;

        if (!rptr->rd->loading /** && ChangeFrame(rptr->dt, rptr->rd, rptr->st) **/ )
//        if (!rptr->rd->loading && ChangeFrame(rptr->dt, rptr->rd, rptr->st))
            ShowFrame(rptr->window, rptr->dt, rptr->rd, rptr->st, 0);
        Sleep(1);
    }

    return 0;
}

/**
        Time Proccessing
**/

int GetCurrentFrame(Data* dt, Render* rd, Settings* st)
{
    rd->current_time = GetTime();
    int b = 0;

    while ((float)((int)((rd->current_time - rd->start_time) * 100) %
           (int)((dt->lengths[dt->count - 1] / ((float)st->speed * 0.05 + 0.2)) * 100)) / 100
           > (dt->lengths[b] / ((float)st->speed * 0.05 + 0.2)))
        b++;

    return b;
}

double GetTime()
{
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / freq.QuadPart;
}

uint8_t ChangeFrame(Data* dt, Render* rd, Settings* st)
{
    rd->current_time = GetTime();
    float delay = dt->delays[rd->frame] / ((float)st->speed * 0.05 + 0.2);

    if (rd->current_time - 0.002 > rd->start_time + delay + rd->inaccuracy)
    {
        rd->inaccuracy += delay - (rd->current_time - rd->start_time);

        if (rd->inaccuracy < -delay * 2)
            rd->inaccuracy = 0;

//        printf("%f | %f | %f | %f\n", rd->start_time, rd->current_time, delay, rd->inaccuracy);

        rd->start_time = rd->current_time;
        return 1;
    }
    else return 0;
}

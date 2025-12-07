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
              Window* window, Window* popup_window, Window* debug_window,

              Window* scale_trackbar, Window* frames_trackbar,

              Window* title_button, Window* openfile_button, Window* scale_button,
              Window* addscale_button, Window* decscale_button, Window* pause_button,
              Window* sfp_button, Window* sti_button, Window* aot_button, Window* mwt_button,
              Window* tlc_button, Window* trc_button, Window* cnr_button, Window* blc_button,
              Window* brc_button, Window* language_button, Window* exit_button, Window* mwt_window)
{
    rptr->window = window;
    rptr->popup_window = popup_window;
    rptr->debug_window = debug_window;
    rptr->mwt_window = mwt_window;               rptr->mwt_window->hovered = 0;

    /** trackbars **/
    rptr->scale_trackbar = scale_trackbar;
    rptr->frames_trackbar = frames_trackbar;

    /** buttons **/
    rptr->title_button = title_button;           rptr->title_button->hovered = 0;
    rptr->openfile_button = openfile_button;     rptr->openfile_button->hovered = 0;
    rptr->scale_button = scale_button;           rptr->scale_button->hovered = 0;
    rptr->addscale_button = addscale_button;     rptr->addscale_button->hovered = 0;
    rptr->decscale_button = decscale_button;     rptr->decscale_button->hovered = 0;
    rptr->pause_button = pause_button;           rptr->pause_button->hovered = 0;
    rptr->sfp_button = sfp_button;               rptr->sfp_button->hovered = 0;
    rptr->sti_button = sti_button;               rptr->sti_button->hovered = 0;
    rptr->aot_button = aot_button;               rptr->aot_button->hovered = 0;
    rptr->mwt_button = mwt_button;               rptr->mwt_button->hovered = 0;
    rptr->tlc_button = tlc_button;               rptr->tlc_button->hovered = 0;
    rptr->trc_button = trc_button;               rptr->trc_button->hovered = 0;
    rptr->cnr_button = cnr_button;               rptr->cnr_button->hovered = 0;
    rptr->blc_button = blc_button;               rptr->blc_button->hovered = 0;
    rptr->brc_button = brc_button;               rptr->brc_button->hovered = 0;
    rptr->language_button = language_button;     rptr->language_button->hovered = 0;
    rptr->exit_button = exit_button;             rptr->exit_button->hovered = 0;

    rptr->dt = dt;
    rptr->rd = rd;
    rptr->st = st;
}

/// Main Loop

void Loop(RenderPtr* rptr)
{
    ShowSettings(rptr->st);

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
        if (rptr->rd->loading) { Sleep(10); continue; }

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!IsWindow(rptr->window->hwnd)) continue;

        if (ChangeFrame(rptr->dt, rptr->rd)) ShowFrame(rptr->window, rptr->dt, rptr->rd, rptr->st);
        Sleep(1);
    }

}

/**
        Windows Proccessing
**/

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static POINT p;
    static RECT rect;
    static HRGN hrgn = NULL;

    switch (msg)
    {
        case WM_CLOSE: {
            if (IsWindow(rptr.window->hwnd))
                DestroyWindow(rptr.window->hwnd);
        }   break;

        case WM_DROPFILES: {
            DragAcceptFiles(hwnd, FALSE);

            _LoadDropFile((HDROP)wparam, rptr.window, rptr.st, rptr.dt, rptr.rd);

            DragAcceptFiles(hwnd, TRUE);
        }   break;

        case WM_LBUTTONDOWN: {
            thread = CreateThread(
                        NULL,
                        0,
                        RenderThread,
                        &rptr,
                        0,
                        NULL
                    );

            if (!IsWindow(rptr.popup_window->hwnd)) SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

            rptr.rd->render_thread = 0;
            WaitForSingleObject(thread, INFINITE); CloseHandle(thread);
            rptr.rd->render_thread = 1;
        }   break;

        case WM_RBUTTONDOWN: {
            ReleaseWindow(rptr.popup_window);

            rptr.title_button->hovered = 0;
            rptr.openfile_button->hovered = 0;
            rptr.scale_button->hovered = 0;
            rptr.addscale_button->hovered = 0;
            rptr.decscale_button->hovered = 0;
            rptr.pause_button->hovered = 0;
            rptr.sfp_button->hovered = 0;
            rptr.sti_button->hovered = 0;
            rptr.aot_button->hovered = 0;
            rptr.mwt_button->hovered = 0;
            rptr.tlc_button->hovered = 0;
            rptr.trc_button->hovered = 0;
            rptr.cnr_button->hovered = 0;
            rptr.blc_button->hovered = 0;
            rptr.brc_button->hovered = 0;
            rptr.language_button->hovered = 0;
            rptr.exit_button->hovered = 0;

            GetCursorPos(&p);
            SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

            if (p.x + POPUP_WIDTH > rect.right - rect.left) { p.x = rect.right - rect.left - POPUP_WIDTH; }
            if (p.y + POPUP_HEIGHT > rect.bottom - rect.top) { p.y = rect.bottom - rect.top - POPUP_HEIGHT; }


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

            LoadWindow(rptr.popup_window, rptr.st, NULL,
                       "PopupWindow", p.x + 5, p.y + 5,
                       1, 1, POPUP_MENU);

            /** Buttons **/

            int y = 5;
            LoadButton(rptr.title_button, rptr.popup_window,
                       5, y, POPUP_WIDTH - 10, 25, 15,
                       " ", (int)NULL);

            LoadButton(rptr.openfile_button, rptr.popup_window,
                       5, y += 34, POPUP_WIDTH - 10, 25, 15,
                       " ", 2);

            LoadButton(rptr.scale_button, rptr.popup_window,
                       5, y += 25, 160, 25, 15,
                       " ", 0);

            LoadButton(rptr.addscale_button, rptr.popup_window,
                       185, y, 30, 25, 15,
                       " ", 4);

            LoadButton(rptr.decscale_button, rptr.popup_window,
                       215, y, 30, 25, 15,
                       " ", 5);

            /** Scale trackbar **/

            LoadTrackBar(rptr.scale_trackbar, rptr.popup_window,
                         10, y += 27, POPUP_WIDTH - 20, 24, 0,
                         1, 200, (int)(rptr.st->trackbar_size * 100), 13);

            if (rptr.dt->count > 1)
            {
                /** Buttons **/

                LoadButton(rptr.pause_button, rptr.popup_window,
                           5, y += 28, POPUP_WIDTH - 10, 25, 15,
                           " ", 6);

                /** Frames trackbar **/

                LoadTrackBar(rptr.frames_trackbar, rptr.popup_window,
                             10, y += 27, POPUP_WIDTH -20, 24, 1,
                             1, rptr.dt->count, rptr.rd->frame - 1, 14);
            }

            /** Buttons **/

            LoadButton(rptr.sfp_button, rptr.popup_window,
                       5, y += 28, POPUP_WIDTH - 10, 25, 15,
                       " ", 7);

            LoadButton(rptr.sti_button, rptr.popup_window,
                       5, y += 25, POPUP_WIDTH - 10, 25, 15,
                       " ", 8);

            LoadButton(rptr.aot_button, rptr.popup_window,
                       5, y += 25, POPUP_WIDTH - 10, 25, 15,
                       " ", 9);

            LoadButton(rptr.mwt_button, rptr.popup_window,
                       5, y += 25, POPUP_WIDTH - 10, 25, 15,
                       " ", 10);

            LoadButton(rptr.exit_button, rptr.popup_window,
                       5, y += 25, POPUP_WIDTH - 10, 25, 15,
                       " ", 12);

            SetWindowPos(rptr.popup_window->hwnd,
                         NULL,
                         0,
                         0,
                         POPUP_WIDTH,
                         y += 32,
                         SWP_NOMOVE);

            hrgn = CreateRoundRectRgn(0, 0,
                                      POPUP_WIDTH, y,
                                      10, 10);

            SetWindowRgn(rptr.popup_window->hwnd, hrgn, TRUE);

            rptr.rd->render_thread = 0;
            WaitForSingleObject(thread, INFINITE); CloseHandle(thread);
            rptr.rd->render_thread = 1;
        }

        case WM_COMMAND: {
            switch (wparam)
            {
                case 1:
                {
                    thread = CreateThread(
                        NULL,
                        0,
                        _LoadSettings,
                        &rptr,
                        0,
                        NULL
                    );

                    CloseHandle(thread);
                }   break;
            }
        }   break;

        case WM_USER: {
            if (wparam == 2)
            {
                SetWindowPos(rptr.window->hwnd,
                             HWND_NOTOPMOST,
                             0,
                             0,
                             0,
                             0,
                             SWP_NOMOVE | SWP_NOSIZE);
            }
            else if (wparam == 3)
            {
                SetWindowPos(rptr.window->hwnd,
                             (rptr.st->topmost) ? HWND_TOPMOST : HWND_NOTOPMOST,
                             0,
                             0,
                             _GetCollisionSize(rptr.dt->width, rptr.st->size),
                             _GetCollisionSize(rptr.dt->height, rptr.st->size),
                             SWP_NOMOVE);
            }
        }

        case WM_KEYDOWN: {
            if (wparam == VK_SPACE)
            {
                if (rptr.rd->change_frames) rptr.rd->change_frames = 0;
                else rptr.rd->change_frames = 1;
            }
            if (wparam == VK_ESCAPE)
            {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
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
    static RECT rect;
    static RECT res;
    static HBRUSH brush = NULL;
    static PAINTSTRUCT ps;
    static HDC hdc = NULL;
    static HPEN pen = NULL;
    static HPEN prev_pen = NULL;
    static HBRUSH prev_brush = NULL;
    static uint8_t ts = 0;

    switch (msg)
    {
        case WM_CREATE: {
            pthread_create(&thread_t, 0, ShowThread, hwnd);
        }   break;

        case WM_DESTROY: {
            rptr.rd->render_thread = 0;
//            pthread_join(thread, NULL);
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

            PostQuitMessage(0);
        }   break;

        case WM_CLOSE: {
            DestroyWindow(hwnd);
            break;
        }

        case WM_ACTIVATE: {
            GetCursorPos(&p);
            GetWindowRect(rptr.popup_window->hwnd, &rect);

            if (wparam != WA_INACTIVE ||
                IsChild(rptr.popup_window->hwnd, (HWND)lparam) ||
                (HWND)lparam == hwnd ||
                PtInRect(&rect, p)) break;

            PostMessage(rptr.popup_window->hwnd, WM_CLOSE, 0, 0);

            if (rptr.mwt_window->isactive)
                PostMessage(rptr.mwt_window->hwnd, WM_CLOSE, 0, 0);

            return DefWindowProc(hwnd, msg, wparam, lparam);
        }

        case WM_LBUTTONDOWN: {
            thread = CreateThread(
                        NULL,
                        0,
                        RenderThread,
                        &rptr,
                        0,
                        NULL
                    );

            SetFocus(rptr.popup_window->hwnd);
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

            rptr.rd->render_thread = 0;
//            pthread_join(thread, NULL);
            WaitForSingleObject(thread, INFINITE);
            rptr.rd->render_thread = 1;
        }   break;

        case WM_RBUTTONDOWN: {
            ReleaseWindow(rptr.popup_window);

            SendMessage(rptr.window->hwnd, WM_RBUTTONDOWN, HTCAPTION, 0);
        }   break;

        case WM_SETCURSOR: {
            GetCursorPos(&p);

            _IsButtonHovered(rptr.title_button, &p, 0);
            _IsButtonHovered(rptr.openfile_button, &p, 0);
            // _IsButtonHovered(rptr.scale_button, &p, 0);
            _IsButtonHovered(rptr.addscale_button, &p, 0);
            _IsButtonHovered(rptr.decscale_button, &p, 0);
            _IsButtonHovered(rptr.pause_button, &p, 0);
            _IsButtonHovered(rptr.sfp_button, &p, 0);
             _IsButtonHovered(rptr.sti_button, &p, 0);
            _IsButtonHovered(rptr.aot_button, &p, 0);
//            _IsButtonHovered(rptr.language_button, &p, 0);
            _IsButtonHovered(rptr.exit_button, &p, 0);

            if (_IsButtonHovered(rptr.mwt_button, &p, 1)) {
                if (rptr.mwt_window->isactive == 0)
                {
                    GetWindowRect(rptr.popup_window->hwnd, &rect);

                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);
                    int popup_left = rect.left + POPUP_WIDTH;
                    int popup_top = rect.top + 224;
                    if (rptr.dt->count == 1) popup_top -= 55;
                    if (popup_left + MWT_WIDTH + 5 > res.right) popup_left = rect.left - POPUP_WIDTH + 24;
                    if (popup_top + MWT_HEIGHT + 5 > res.bottom) popup_top = res.bottom - 229;

                    LoadWindow(rptr.mwt_window, rptr.st, NULL, "FrameInfo", popup_left, popup_top, MWT_WIDTH, MWT_HEIGHT, 0, 0, 0, 0);

                    HRGN hrgn = CreateRoundRectRgn(0, 0, MWT_WIDTH, MWT_HEIGHT, 5, 5);

                    SetWindowRgn(rptr.mwt_window->hwnd, hrgn, TRUE);

                    rptr.tlc_button->hovered = 0;
                    rptr.trc_button->hovered = 0;
                    rptr.cnr_button->hovered = 0;
                    rptr.blc_button->hovered = 0;
                    rptr.brc_button->hovered = 0;

                    int y = 5;
                    LoadButton(rptr.tlc_button, rptr.mwt_window,
                               5, y, MWT_WIDTH - 10, 25, 15,
                               " ", 1);

                    LoadButton(rptr.trc_button, rptr.mwt_window,
                               5, y += 25, MWT_WIDTH - 10, 25, 15,
                               " ", 2);

                    LoadButton(rptr.cnr_button, rptr.mwt_window,
                               5, y += 25, MWT_WIDTH - 10, 25, 15,
                               " ", 3);

                    LoadButton(rptr.blc_button, rptr.mwt_window,
                               5, y += 25, MWT_WIDTH - 10, 25, 15,
                               " ", 4);

                    LoadButton(rptr.brc_button, rptr.mwt_window,
                               5, y += 25, MWT_WIDTH - 10, 25, 15,
                               " ", 5);
                }
            }
            else PostMessage(rptr.mwt_window->hwnd, WM_USER, 1, 0);

        }   break;

        case WM_PAINT: {
            hdc = BeginPaint(rptr.popup_window->hwnd, &ps);

            GetClientRect(rptr.popup_window->hwnd, &rect);

            rect.left = 0; rect.top = 0; rect.right = POPUP_WIDTH; rect.bottom = POPUP_HEIGHT;
            HBRUSH brush = CreateSolidBrush(RGB(250, 250, 250));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            rect.left = 10; rect.top = 34; rect.right = POPUP_WIDTH - 10; rect.bottom = 35;
            brush = CreateSolidBrush(RGB(210, 210, 210));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            GetClientRect(rptr.popup_window->hwnd, &rect);
            pen = CreatePen(PS_SOLID, 3, RGB(238, 238, 238));
            prev_pen = (HPEN)SelectObject(hdc, pen);
            prev_brush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

            RoundRect(hdc, 0, 0, rect.right - 1, rect.bottom - 1, 10, 10);
            SelectObject(hdc, prev_pen);
            SelectObject(hdc, prev_brush);
            DeleteObject(pen);

            EndPaint(rptr.popup_window->hwnd, &ps);

        }   break;

        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT)lparam;

            if (item->hwndItem == rptr.title_button->hwnd)
                _InvalidateButton(item, rptr.title_button, APP_NAME, 35, 0, 0);

            else if (item->hwndItem == rptr.openfile_button->hwnd)
                _InvalidateButton(item, rptr.openfile_button, "Open file...", 35, 0, 0);

            else if (item->hwndItem == rptr.scale_button->hwnd) {
                sprintf(rptr.st->str_size, "Scale (%.0f%%)", rptr.st->trackbar_size * 100);
                _InvalidateButton(item, rptr.scale_button, rptr.st->str_size, 35, 0, 0);
            }
            else if (item->hwndItem == rptr.addscale_button->hwnd)
                _InvalidateButton(item, rptr.addscale_button, "+", 10, 0, 0);

            else if (item->hwndItem == rptr.decscale_button->hwnd)
                _InvalidateButton(item, rptr.decscale_button, "-", 10, 0, 0);

            else if (item->hwndItem == rptr.pause_button->hwnd)
                _InvalidateButton(item, rptr.pause_button, "Pause", 35, 0, 0);

            else if (item->hwndItem == rptr.sfp_button->hwnd)
                _InvalidateButton(item, rptr.sfp_button, "Show frame updates", 35, rptr.st->sfu, 0);

            else if (item->hwndItem == rptr.sti_button->hwnd)
                _InvalidateButton(item, rptr.sti_button, "Show taskbar icon", 35, rptr.st->taskbar, 0);

            else if (item->hwndItem == rptr.aot_button->hwnd)
                _InvalidateButton(item, rptr.aot_button, "Always on top", 35, rptr.st->topmost, 0);

            else if (item->hwndItem == rptr.mwt_button->hwnd)
                _InvalidateButton(item, rptr.mwt_button, "Move window to", 35, 0, 1);

            else if (item->hwndItem == rptr.exit_button->hwnd)
                _InvalidateButton(item, rptr.exit_button, "Close", 35, 0, 0);

        }   break;

        case WM_CTLCOLORSTATIC: {
            HWND hwnd = (HWND)lparam;
            brush = CreateSolidBrush(RGB(250, 250, 250));

            if (GetDlgCtrlID(hwnd) == 13 || GetDlgCtrlID(hwnd) == 14) {
                SetBkMode((HDC)wparam, TRANSPARENT);
                return (LRESULT)brush;
            }
        }   break;

        case WM_HSCROLL: {
            if ((HWND)lparam == rptr.scale_trackbar->hwnd) {
                rptr.st->pos = SendMessage(rptr.scale_trackbar->hwnd, TBM_GETPOS, 0, 0);

                _ChangeScaleTrackBar(rptr.window, rptr.popup_window,
                                     rptr.scale_trackbar, rptr.scale_button,
                                     rptr.st, rptr.dt, rptr.rd,
                                     rptr.st->pos);
            }
            else {
                SetFocus(rptr.popup_window->hwnd);
                rptr.rd->frame = SendMessage(rptr.frames_trackbar->hwnd, TBM_GETPOS, 0, 0) - 1;
                rptr.rd->framed_trackbar = 1; ShowFrame(rptr.window, rptr.dt, rptr.rd, rptr.st); rptr.rd->framed_trackbar = 0;
            }
        }   break;

        case WM_UPDATE_ALPHA: {
            uint8_t alpha = (uint8_t)lparam;
            SetLayeredWindowAttributes(hwnd, 0x0, alpha, LWA_ALPHA);
        }   break;

        case WM_COMMAND: {
            if ( (wparam == 4 || wparam == 327684) && lparam == (LPARAM)rptr.addscale_button->hwnd) {
                rptr.st->pos = SendMessage(rptr.scale_trackbar->hwnd, TBM_GETPOS, 0, 0) + 1;

                _ChangeScaleTrackBar(rptr.window, rptr.popup_window,
                                     rptr.scale_trackbar, rptr.scale_button,
                                     rptr.st, rptr.dt, rptr.rd,
                                     rptr.st->pos);

                PostMessage(rptr.scale_trackbar->hwnd, TBM_SETPOS, TRUE, rptr.st->pos);
            }

            else if ( (wparam == 5 || wparam == 327685) && lparam == (LPARAM)rptr.decscale_button->hwnd) {
                rptr.st->pos = SendMessage(rptr.scale_trackbar->hwnd, TBM_GETPOS, 0, 0) - 1;

                _ChangeScaleTrackBar(rptr.window, rptr.popup_window,
                                     rptr.scale_trackbar, rptr.scale_button,
                                     rptr.st, rptr.dt, rptr.rd,
                                     rptr.st->pos);

                PostMessage(rptr.scale_trackbar->hwnd, TBM_SETPOS, TRUE, rptr.st->pos);
            }

            else if ( wparam == 2 && (HWND)lparam == rptr.openfile_button->hwnd) {
                PostMessage(rptr.window->hwnd, WM_COMMAND, 1, 0);
                ReleaseWindow(rptr.popup_window);
            }

            else if ( (wparam == 8 || wparam == 327688) && (HWND)lparam == rptr.sti_button->hwnd) {
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

                if (ts) {
                    ShowWindow(rptr.window->hwnd, SW_HIDE);
                    ShowWindow(rptr.window->hwnd, SW_SHOW);
                    ts = 1;
                }
                SetFocus(rptr.popup_window->hwnd);

                WriteSettings(rptr.st);
            }

            switch (wparam) {
                case 6: {
                    if (rptr.rd->change_frames) rptr.rd->change_frames = 0;
                    else rptr.rd->change_frames = 1;

                    SetFocus(rptr.popup_window->hwnd);
                }   break;
                case 7: {
                    if (rptr.st->sfu)
                        rptr.st->sfu = 0;
                    else
                        rptr.st->sfu = 1;

                    SetFocus(rptr.popup_window->hwnd);
                    InvalidateRect(rptr.sfp_button->hwnd, NULL, TRUE);
                }   break;
                case 9: {
                    if (rptr.st->topmost)
                        rptr.st->topmost = 0;
                    else
                        rptr.st->topmost = 1;

                    SetWindowPos(rptr.window->hwnd, NULL,
                             0, 0, 0, 0,
                             SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

                    SetFocus(rptr.popup_window->hwnd);
                    InvalidateRect(rptr.aot_button->hwnd, NULL, TRUE);
                    WriteSettings(rptr.st);
                }   break;

                case 12: {
                    PostMessage(rptr.window->hwnd, WM_CLOSE, 0, 0);
                }   break;

                case 16: {
                    SetLayeredWindowAttributes(rptr.popup_window->hwnd, 0x0, (uint8_t)lparam, LWA_ALPHA);
                }   break;
            }
        }   break;

        case WM_KEYDOWN: {
            if (wparam == VK_SPACE) {
                if (rptr.rd->change_frames) rptr.rd->change_frames = 0;
                else rptr.rd->change_frames = 1;
            }
            else if (wparam == VK_ESCAPE)
                PostMessage(rptr.popup_window->hwnd, WM_CLOSE, 0, 0);
            else if (wparam == VK_LEFT || wparam == VK_UP) {
                if (rptr.rd->change_frames) {
                    PostMessage(rptr.popup_window->hwnd, WM_COMMAND, 327685, (LPARAM)rptr.decscale_button->hwnd);
                }
                else {
                    SetFocus(rptr.popup_window->hwnd);
                    int pos = SendMessage(rptr.frames_trackbar->hwnd, TBM_GETPOS, 0, 0) - 1;
                    if (pos - 1 < 0)
                        rptr.rd->frame = rptr.dt->count - 1;
                    else
                        rptr.rd->frame = pos - 1;
                    PostMessage(rptr.frames_trackbar->hwnd, TBM_SETPOS, TRUE, pos);
                    rptr.rd->framed_trackbar = 1; ShowFrame(rptr.window, rptr.dt, rptr.rd, rptr.st); rptr.rd->framed_trackbar = 0;
                }
            }
            else if (wparam == VK_RIGHT || wparam == VK_DOWN) {
                if (rptr.rd->change_frames) {
                    PostMessage(rptr.popup_window->hwnd, WM_COMMAND, 327684, (LPARAM)rptr.addscale_button->hwnd);
                }
                else {
                    SetFocus(rptr.popup_window->hwnd);
                    int pos = SendMessage(rptr.frames_trackbar->hwnd, TBM_GETPOS, 0, 0);

                    if (pos + 1 > rptr.dt->count) { rptr.rd->frame = 0; pos = 1; }
                    else { rptr.rd->frame = pos; pos++; }

                    SendMessage(rptr.frames_trackbar->hwnd, TBM_SETPOS, TRUE, pos);
                    rptr.rd->framed_trackbar = 1; ShowFrame(rptr.window, rptr.dt, rptr.rd, rptr.st); rptr.rd->framed_trackbar = 0;
                }
            }
            else
                PostMessage(rptr.popup_window->hwnd, WM_CLOSE, 0, 0);

        }   break;

        default: break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK MWTProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static RECT rect, res;
    static POINT p;
    static HBRUSH brush = NULL;
    static HBRUSH prev_brush = NULL;
    static HDC hdc = NULL;
    static PAINTSTRUCT ps;
    static HPEN pen = NULL;
    static HPEN prev_pen = NULL;

    switch (msg)
    {
        case WM_CREATE: {
            pthread_create(&thread_t, 0, ShowThread, hwnd);
        }   break;

        case WM_DESTROY: {
            PostQuitMessage(0);
        }   break;

        case WM_CLOSE: {
            DestroyWindow(hwnd);
            SetFocus(rptr.popup_window->hwnd);
            rptr.mwt_window->isactive = 0;
            break;
        }

        case WM_PAINT: {
            hdc = BeginPaint(rptr.mwt_window->hwnd, &ps);

            GetClientRect(rptr.mwt_window->hwnd, &rect);
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

            EndPaint(rptr.mwt_window->hwnd, &ps);
        }   break;

        case WM_SETCURSOR: {
            GetCursorPos(&p);

            _IsButtonHovered(rptr.mwt_window, &p, 0);
            _IsButtonHovered(rptr.mwt_button, &p, 0);
            _IsButtonHovered(rptr.tlc_button, &p, 0);
            _IsButtonHovered(rptr.trc_button, &p, 0);
            _IsButtonHovered(rptr.cnr_button, &p, 0);
            _IsButtonHovered(rptr.blc_button, &p, 0);
            _IsButtonHovered(rptr.brc_button, &p, 0);
        }   break;

        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT)lparam;

            if (item->hwndItem == rptr.tlc_button->hwnd)
                _InvalidateButton(item, rptr.tlc_button, "Top left corner", 35, 0, 0);

            if (item->hwndItem == rptr.trc_button->hwnd)
                _InvalidateButton(item, rptr.trc_button, "Top right corner", 35, 0, 0);

            if (item->hwndItem == rptr.cnr_button->hwnd)
                _InvalidateButton(item, rptr.cnr_button, "Center", 35, 0, 0);

            if (item->hwndItem == rptr.blc_button->hwnd)
                _InvalidateButton(item, rptr.blc_button, "Bottom left corner", 35, 0, 0);

            if (item->hwndItem == rptr.brc_button->hwnd)
                _InvalidateButton(item, rptr.brc_button, "Bottom right corner", 35, 0, 0);
        }   break;

        /** Checks whether window should exist **/

        case WM_USER: {
            GetCursorPos(&p);

            RECT wrect; GetWindowRect(rptr.mwt_window->hwnd, &wrect);
            wrect.left += 1; wrect.top += 1; wrect.right -= 1; wrect.bottom -= 1;

            RECT brect; GetWindowRect(rptr.mwt_button->hwnd, &brect);
            brect.left += 1; brect.top += 1; brect.right -= 1; brect.bottom -= 1;

            if (!PtInRect(&wrect, p) && !PtInRect(&brect, p)) {
                PostMessage(rptr.mwt_window->hwnd, WM_CLOSE, 0, 0);
            }
        }

        case WM_ACTIVATE: {
            if (wparam == WA_INACTIVE)
                PostMessage(rptr.mwt_window->hwnd, WM_CLOSE, 0, 0);
        }   break;

        case WM_MOUSEMOVE: {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = hwnd;
            tme.dwFlags = TME_LEAVE;

            _TrackMouseEvent(&tme);
        }   break;

        case WM_MOUSELEAVE: {
            GetCursorPos(&p);

            RECT wrect; GetWindowRect(rptr.mwt_window->hwnd, &wrect);
            wrect.left += 1; wrect.top += 1; wrect.right -= 1; wrect.bottom -= 1;

            RECT brect; GetWindowRect(rptr.mwt_button->hwnd, &brect);
            brect.left += 1; brect.top += 1; brect.right -= 1; brect.bottom -= 1;

            if (!PtInRect(&wrect, p) && !PtInRect(&brect, p)) {
                PostMessage(rptr.mwt_window->hwnd, WM_CLOSE, 0, 0);
            }
            else {
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.hwndTrack = hwnd;
                tme.dwFlags = TME_LEAVE;
            }
        }   break;

        /** Commands **/

        case WM_UPDATE_ALPHA: {
            uint8_t alpha = (uint8_t)lparam;
            SetLayeredWindowAttributes(hwnd, 0x0, alpha, LWA_ALPHA);
        }   break;

        case WM_COMMAND: {
            switch (wparam)
            {
                case 1:
                {
                    SetWindowPos(rptr.window->hwnd,
                                 NULL,
                                 0,
                                 0,
                                 0,
                                 0,
                                 SWP_NOSIZE);
                }   break;
                case 2:
                {
                    SetWindowPos(rptr.window->hwnd,
                                 NULL,
                                 GetSystemMetrics(SM_CXSCREEN) - (rptr.dt->width) * rptr.st->trackbar_size,
                                 0,
                                 0,
                                 0,
                                 SWP_NOSIZE);
                }   break;
                case 3:
                {
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);

                    SetWindowPos(rptr.window->hwnd,
                                 NULL,
                                 (res.right - res.left - rptr.dt->width * rptr.st->trackbar_size) / 2,
                                 (res.bottom - res.top - rptr.dt->height * rptr.st->trackbar_size) / 2,
                                 0,
                                 0,
                                 SWP_NOSIZE);
                }   break;
                case 4:
                {
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);

                    SetWindowPos(rptr.window->hwnd,
                                 NULL,
                                 0,
                                 res.bottom - res.top - (rptr.dt->height) * rptr.st->trackbar_size,
                                 0,
                                 0,
                                 SWP_NOSIZE);
                }   break;
                case 5:
                {
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
        case WM_INITDIALOG: {
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
        }   break;

        case WM_CTLCOLORDLG: {
            return (LRESULT)CreateSolidBrush(RGB(255, 255, 255));
        }   break;

        case WM_CTLCOLORSTATIC: {
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
        }   break;

        case WM_COMMAND: {
            switch (LOWORD(wparam))
            {
                case ID_SELECT:
                {
                    EndDialog(hwnd, 1);
                    return TRUE;
                }
                case ID_EXIT:
                {
                    EndDialog(hwnd, 0);
                    return TRUE;
                }   break;
            }
        }   break;
    }
    return FALSE;
}

void _InvalidateButton(LPDRAWITEMSTRUCT item, Window* window,
                       const char* text, int left, int activated, int arrow)
{
    static HDC hdc = NULL;
    static HFONT hFont = NULL;
    static HFONT hOldFont = NULL;
    static COLORREF oldColor;
    static RECT rect;
    static HPEN pen = NULL;
    static HPEN prev_pen = NULL;
    rect = item->rcItem;
    hdc = item->hDC;

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(30, 30, 30));
    HBRUSH hBrush = CreateSolidBrush(
        (window->hovered) ? RGB(242, 242, 242) : RGB(250, 250, 250)
    );
    FillRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);
    rect.left = left;
    DrawText(hdc, text, -1, &rect, DT_VCENTER | DT_SINGLELINE);

    if (activated) {
        int size = 20;
        int x = rect.left - 27;
        int y = rect.top + (rect.bottom - rect.top - size) / 2;

        hFont = CreateFont(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH, "Marlett");
        hOldFont = (HFONT)SelectObject(hdc, hFont);
        oldColor = SetTextColor(hdc, RGB(90, 90, 90));
        int oldBkMode = SetBkMode(hdc, TRANSPARENT);

        TextOutW(hdc, x, y, L"a", 1);

        SetBkMode(hdc, oldBkMode);
        SetTextColor(hdc, oldColor);
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
    }
    if (arrow) {
        pen = CreatePen(PS_SOLID, 2, RGB(110, 110, 110));
        prev_pen = SelectObject(item->hDC, pen);

        rect = item->rcItem;
        int size = 8;
        int x = rect.right - 20;
        int y = rect.top + (rect.bottom - rect.top) / 2 - size / 2;

        MoveToEx(item->hDC, x, y, NULL);
        LineTo(item->hDC, x + size * 0.75, y + size / 2);
        LineTo(item->hDC, x, y + size);

        SelectObject(item->hDC, prev_pen);
        DeleteObject(pen);
    }
}

int _IsButtonHovered(Window* window, POINT* p, int arrowed)
{
    static RECT rect; GetWindowRect(window->hwnd, &rect);
    if (arrowed) { rect.left -= 5; rect.right += 5; }
    if (PtInRect(&rect, *p) && !window->hovered) { window->hovered = 1; InvalidateRect(window->hwnd, NULL, TRUE); }
    else if (!PtInRect(&rect, *p) && window->hovered) { window->hovered = 0; InvalidateRect(window->hwnd, NULL, TRUE); }
    return window->hovered;
}

int _GetCollisionSize(int n, float size)
{
    return (n * size < 10.0) ? 10.0 : n * size + size;
}

void _ChangeScaleTrackBar(Window* window, Window* popup_window,
                          Window* scale_trackbar, Window* scale_button,
                          Settings* st, Data* dt,
                          Render* rd, int pos)
{
    SetFocus(popup_window->hwnd);

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

void* ShowThread(void* arg)
{
    Sleep(10);
    for (uint16_t i = 20; i <= 255; i += 20) {
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
        PostMessage(rptr.frames_trackbar->hwnd, TBM_SETPOS, TRUE, rd->frame + 1);

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

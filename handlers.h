#ifndef HANDLERS_H_INCLUDED
#define HANDLERS_H_INCLUDED

#include "types.h"
#include "render.h"
#include "flags.h"
#include "gifdesk.h"


/** String functions **/

int _ContainsOnlyStr(const char *str, const char *allowed) {
    while (*str != '\0') {
        if (strchr(allowed, *str) == NULL)
            return 0;
        str++;
    }
    return 1;
}

int _CountChr(char sym, char* str) {
    int cnt = 0;
    for (char *ptr = strchr(str, sym); ptr; ptr = strchr(++ptr, sym)) cnt++;
    return cnt;
}

/** Params **/

SINLINE int _SetMinimalWindowSize(LPARAM lparam, float scale) {
    LPMINMAXINFO lpmmi = (LPMINMAXINFO)lparam;
    lpmmi->ptMinTrackSize.x = ScaleForDPI(MGR_MINWINDOWSIZEX, scale);
    lpmmi->ptMinTrackSize.y = ScaleForDPI(MGR_MINWINDOWSIZEY, scale);

    return 0;
}

SINLINE void _SetSizableWindow(Manager* manager, GIFDesk* gfk, uint8_t pos) {
    static RECT res;
    GIFDesk* prev_gfk = &manager->gfk[manager->gfk_current];

    if (manager->gfk_current >= 0) {
        GetWindowRect(prev_gfk->window, &res);

        if (prev_gfk->width * prev_gfk->size < 10)
            prev_gfk->size = (10 / (float)prev_gfk->width);
        if (prev_gfk->height * prev_gfk->size < 10)
            prev_gfk->size = (10 / (float)prev_gfk->height);

        float rl_ = (float)res.left,            rt_ = (float)res.top,
              dw_ = (float)prev_gfk->width,     dh_ = (float)prev_gfk->height,
              dwh = (float)prev_gfk->npotwidth, dhh = (float)prev_gfk->npotheight,
              sts = prev_gfk->size;

        int coordw_LLC = (int) (rl_ + 0.5555),
            coordh_LLC = (int) (rt_ + (dhh * 2.0f - dhh * sts) + 0.55555),
            coordw_RTC = (int) (rl_ + (dwh * 2.0f - dwh * sts) + 0.55555),
            coordh_RTC = (int) (rt_ + 0.55555),

            coordw_C   = (int) ((rl_ + (dwh * 2.0f - dwh * sts) / 2.0f) + 0.55555),
            coordh_C   = (int) ((rt_ + (dhh * 2.0f - dhh * sts) / 2.0f) + 0.55555),

            coordw     = (int) (dw_ * sts + 0.55555),
            coordh     = (int) (dh_ * sts + 0.55555);

        switch (prev_gfk->pos) {
            case POS_LTC:
                SetWindowPos(prev_gfk->window,
                             (prev_gfk->flags & SETTINGS_AOT) ? HWND_TOPMOST : HWND_TOP,
                             0, 0, coordw, coordh, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOACTIVATE);
                break;
            case POS_LLC:
                SetWindowPos(prev_gfk->window,
                             (prev_gfk->flags & SETTINGS_AOT) ? HWND_TOPMOST : HWND_TOP,
                             coordw_LLC, coordh_LLC, coordw, coordh, SWP_NOREDRAW | SWP_NOACTIVATE);
                break;
            case POS_RTC:
                SetWindowPos(prev_gfk->window,
                             (prev_gfk->flags & SETTINGS_AOT) ? HWND_TOPMOST : HWND_TOP,
                             coordw_RTC, coordh_RTC, coordw, coordh, SWP_NOREDRAW | SWP_NOACTIVATE);
                break;
            case POS_RLC:
                SetWindowPos(prev_gfk->window,
                             (prev_gfk->flags & SETTINGS_AOT) ? HWND_TOPMOST : HWND_TOP,
                             coordw_RTC, coordh_LLC, coordw, coordh, SWP_NOREDRAW | SWP_NOACTIVATE);
                break;
            case POS_C:
                SetWindowPos(prev_gfk->window,
                             (prev_gfk->flags & SETTINGS_AOT) ? HWND_TOPMOST : HWND_TOP,
                             coordw_C, coordh_C, coordw, coordh, SWP_NOREDRAW | SWP_NOACTIVATE);
                break;
            default:
                break;
        }
        prev_gfk->pos = POS_NULL;
        GetWindowRect(prev_gfk->window, &res);
        prev_gfk->x = (uint16_t)res.left;
        prev_gfk->y = (uint16_t)res.top;
        WriteSettings(manager);
    }

    if (!gfk) return;

    HMONITOR mtr = MonitorFromWindow(gfk->window, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mtf = {sizeof(MONITORINFO)};
    GetMonitorInfo(mtr, &mtf);

    GetWindowRect(gfk->window, &res);

    if (pos) gfk->pos = pos;
    else {
        float reslc = (float)res.left + (float)(res.right - res.left) / 2.0f,
              restc = (float)res.top + (float)(res.bottom - res.top) / 2.0f,
              rectr2 = (float)mtf.rcWork.right / 2.0f,
              rectb2 = (float)mtf.rcWork.bottom / 2.0f,
              rectr3 = (float)mtf.rcWork.right / 3.0f,
              rectb3 = (float)mtf.rcWork.bottom / 3.0f,
              rectr23 = (float)mtf.rcWork.right / 3.0f * 2.0f,
              rectb23 = (float)mtf.rcWork.bottom / 3.0f * 2.0f;

        uint8_t pos = 0;
        if (reslc < rectr2) pos |= 0x1;
        if (restc < rectb2) pos |= 0x2;
        if (reslc < rectr3) pos |= 0x4;
        if (restc < rectb3) pos |= 0x8;
        if (reslc < rectr23) pos |= 0x10;
        if (restc < rectb23) pos |= 0x20;

        switch (pos) {
            case 0x3F:
            case 0x3B:
            case 0x37:
                gfk->pos = POS_LTC;
                break;
            case 0x35:
            case 0x15:
            case 0x11:
                gfk->pos = POS_LLC;
                break;
            case 0x3A:
            case 0x2A:
            case 0x22:
                gfk->pos = POS_RTC;
                break;
            case 0x20:
            case 0x10:
            case 0x0:
                gfk->pos = POS_RLC;
                break;
            default:
                gfk->pos = POS_C;
                break;
        }
        if (reslc > mtf.rcWork.right) gfk->pos = POS_LTC;
        else if (restc > mtf.rcWork.bottom) gfk->pos = POS_LTC;
    }

    float rl_ = (float)res.left,           rt_ = (float)res.top,
          dwh = (float)gfk->npotwidth,     dhh = (float)gfk->npotheight,
          sts = gfk->size;

    int coordw_LLC  = (int) (rl_ + 0.5),
        coordh_LLC  = (int) (rt_ - (dhh * 2.0f - dhh * sts) + 0.5),
        coordw_RTC  = (int) (rl_ - (dwh * 2.0f - dwh * sts) + 0.5),
        coordh_RTC  = (int) (rt_ + 0.5),

        coordw_C    = (int) ((rl_ - (dwh * 2.0f - dwh * sts) / 2.0f) + 0.5),
        coordh_C    = (int) ((rt_ - (dhh * 2.0f - dhh * sts) / 2.0f) + 0.5),

        coordw      = (int) (dwh * 2.0f + 0.5),
        coordh      = (int) (dhh * 2.0f + 0.5);

    switch ((pos) ? pos : gfk->pos) {
        case POS_LTC:
            SetWindowPos(gfk->window, HWND_TOPMOST,
                         0, 0, coordw, coordh, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOACTIVATE);
            break;
        case POS_LLC:
            SetWindowPos(gfk->window, HWND_TOPMOST,
                         coordw_LLC, coordh_LLC,
                         coordw, coordh, SWP_NOREDRAW | SWP_NOACTIVATE);
            break;
        case POS_C:
            SetWindowPos(gfk->window, HWND_TOPMOST,
                         coordw_C, coordh_C,
                         coordw, coordh, SWP_NOREDRAW | SWP_NOACTIVATE);

            break;
        case POS_RTC:
            SetWindowPos(gfk->window, HWND_TOPMOST,
                         coordw_RTC, coordh_RTC,
                         coordw, coordh, SWP_NOREDRAW | SWP_NOACTIVATE);
            break;
        case POS_RLC:
            SetWindowPos(gfk->window, HWND_TOPMOST,
                         coordw_RTC, coordh_LLC,
                         coordw, coordh, SWP_NOREDRAW | SWP_NOACTIVATE);
            break;
        default:
            break;
    }
}

/** Drawing functions **/

SINLINE int _InvalidateManagerBkg(HWND hwnd) {
    PAINTSTRUCT paintst;

    BeginPaint(hwnd, &paintst);

    EndPaint(hwnd, &paintst);

    return 0;
}

SINLINE void _InvalidateManagerRect(Manager* manager, LPDRAWITEMSTRUCT lpdis) {
    static COLORREF ref = NULL;
    static Object* obj;
    static RECT rect;
    static UINT state;
    static HBRUSH brush = NULL;
    static HDC hdc;

    hdc = lpdis->hDC;
    obj = &(manager->objects[GetDlgCtrlID(lpdis->hwndItem) - 1]);
    state = lpdis->itemState;
    rect = lpdis->rcItem;

    if (state & ODS_DISABLED) {
        ref = obj->colormod[0].disabled;
    }
    else if (state & ODS_SELECTED) {
        ref = obj->colormod[0].pressed;
    }
    else if (obj->flags & OBJ_HOVERED) {
        ref = obj->colormod[0].hovered;
    }
    else {
        ref = obj->colormod[0].enabled;
    }

    /** Filling Clip Rgn **/

    brush = (HBRUSH)CreateSolidBrush(ref);
    FillRect(hdc, &rect, brush);

    DeleteObject(brush);
}

SINLINE void _InvalidateManagerStatic(Manager* manager, LPDRAWITEMSTRUCT lpdis) {
    static COLORREF ref = NULL;
    static Object* obj;
    static RECT rect;
    static UINT state;
    static HBRUSH brush = NULL;
    static HDC hdc;
    static TCHAR text[512];

    hdc = lpdis->hDC;
    obj = &(manager->objects[GetDlgCtrlID(lpdis->hwndItem) - 1]);
    state = lpdis->itemState;
    rect = lpdis->rcItem;

    /** Selecting Button Background **/

    if (state & ODS_DISABLED) {
        SetTextColor(hdc, obj->colormod[1].disabled);
        ref = obj->colormod[0].disabled;
    }
    else if (state & ODS_SELECTED) {
        SetTextColor(hdc, obj->colormod[1].pressed);
        ref = obj->colormod[0].pressed;
    }
    else if (obj->flags & OBJ_HOVERED) {
        SetTextColor(hdc, obj->colormod[1].hovered);
        ref = obj->colormod[0].hovered;
    }
    else {
        SetTextColor(hdc, obj->colormod[1].enabled);
        ref = obj->colormod[0].enabled;
    }

    /** Filling Clip Rgn **/

    brush = (HBRUSH)CreateSolidBrush(ref);
    FillRect(hdc, &rect, brush);

    /** Drawing text **/

    SetBkMode(hdc, TRANSPARENT);
    GetWindowText(lpdis->hwndItem, text, _countof(text));
    rect.left += ScaleForDPI(obj->tol, manager->scale_dpi);
    rect.top += ScaleForDPI(obj->tot, manager->scale_dpi);
    rect.right += ScaleForDPI(obj->tor, manager->scale_dpi);
    rect.bottom += ScaleForDPI(obj->tob, manager->scale_dpi);
    DrawText(hdc, text, -1, &rect,
             (obj->dt_format) ?
             (obj->dt_format) :
             (DT_CENTER | DT_VCENTER | DT_SINGLELINE));

    DeleteObject(brush);
}

SINLINE void _InvalidateManagerButton(Manager* manager, LPDRAWITEMSTRUCT lpdis) {
    static COLORREF ref = NULL;
    static Object* obj;
    static RECT rect;
    static UINT state;
    static HBRUSH brush = NULL;
    static HDC hdc;
    static TCHAR text[512];

    hdc = lpdis->hDC;
    obj = &(manager->objects[GetDlgCtrlID(lpdis->hwndItem) - 1]);
    state = lpdis->itemState;
    rect = lpdis->rcItem;

    /** Selecting Button Background **/

    if (state & ODS_DISABLED) {
        SetTextColor(hdc, obj->colormod[1].disabled);
        ref = obj->colormod[0].disabled;
    }
    else if (state & ODS_SELECTED || obj->flags & OBJ_CHECKED) {
        SetTextColor(hdc, obj->colormod[1].pressed);
        ref = obj->colormod[0].pressed;
        DEFBTNBTNUPDEL((void*)obj);
    }
    else if (obj->flags & OBJ_HOVERED || obj->flags & OBJ_CHOVERED) {
        SetTextColor(hdc, obj->colormod[1].hovered);
        ref = obj->colormod[0].hovered;
    }
    else {
        SetTextColor(hdc, obj->colormod[1].enabled);
        ref = obj->colormod[0].enabled;
    }

    /** Filling Clip Rgn **/

    brush = (HBRUSH)CreateSolidBrush(ref);
    FillRect(hdc, &rect, brush);

    /** Drawing text **/

    SetBkMode(hdc, TRANSPARENT);
    GetWindowText(lpdis->hwndItem, text, _countof(text));

    rect.left += ScaleForDPI(obj->tol, manager->scale_dpi);
    rect.top += ScaleForDPI(obj->tot, manager->scale_dpi);
    rect.right += ScaleForDPI(obj->tor, manager->scale_dpi);
    rect.bottom += ScaleForDPI(obj->tob, manager->scale_dpi);

    DrawText(hdc, text, -1, &rect, obj->dt_format);

    DeleteObject(brush);
}

SINLINE void _InvalidateManagerSwitch(Manager* manager, LPDRAWITEMSTRUCT lpdis) {
    static COLORREF ref = NULL, sref = NULL, dref = NULL;
    static Object* obj;
    static RECT rect;
    static UINT state;
    static HDC hdc;
    static int ox, oy, cx, cy;

    static HBRUSH brush = NULL;
    static HRGN srgn = NULL;

    hdc = lpdis->hDC;
    obj = &(manager->objects[GetDlgCtrlID(lpdis->hwndItem) - 1]);
    state = lpdis->itemState;
    rect = lpdis->rcItem;

    /** Selecting Button Background **/

    if (state & ODS_DISABLED) {
        ref = obj->colormod[0].disabled;
        sref = obj->colormod[1].disabled;
        dref = obj->colormod[2].disabled;
    }
    else if (state & ODS_SELECTED) {
        ref = obj->colormod[0].pressed;
        sref = obj->colormod[1].pressed;
        dref = obj->colormod[2].pressed;
    }
    else if (obj->flags & OBJ_HOVERED) {
        ref = obj->colormod[0].hovered;
        sref = obj->colormod[1].hovered;
        dref = obj->colormod[2].hovered;
    }
    else {
        ref = obj->colormod[0].enabled;
        sref = obj->colormod[1].enabled;
        dref = obj->colormod[2].enabled;
    }

    /** Filling Clip Rgn **/

    brush = (HBRUSH)CreateSolidBrush(ref);
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);


    /** Filling switch **/

    cx = obj->swsize;
    cy = obj->swsize / 2;
    ox = (rect.right - rect.left - cx) / 2;
    oy = (rect.bottom - rect.top - cy) / 2;

    srgn = CreateRoundRectRgn(ox + 1, oy + 1, ox + cx - 1, oy + cy - 1, obj->swsize / 3, obj->swsize / 3);

    brush = (HBRUSH)CreateSolidBrush(sref);
    FillRgn(hdc, srgn, brush);
    DeleteObject(brush);

    brush = (HBRUSH)CreateSolidBrush(dref);
    SelectObject(hdc, (HPEN)GetStockObject(NULL_PEN));
    SelectObject(hdc, brush);
    if (obj->flags & OBJ_BOOL)
        Ellipse(hdc, cx / 2, 0, cx, cx / 2);
    else
        Ellipse(hdc, 0, 0, cx / 2, cx / 2);

    DeleteObject(brush);
    DeleteObject(srgn);
}

SINLINE void _InvalidateManagerScrollBar(Manager* manager, LPDRAWITEMSTRUCT lpdis) {
    static COLORREF ref = NULL, sref = NULL;
    static Object* obj;
    static RECT rect, orect, brect, trect;
    static UINT state;
    static HDC hdc;

    static HBRUSH brush = NULL;
    static HRGN rgn = NULL;

    hdc = lpdis->hDC;
    obj = &(manager->objects[GetDlgCtrlID(lpdis->hwndItem) - 1]);
    state = lpdis->itemState;
    rect = lpdis->rcItem;

    /** Selecting Rectangle Background **/

    if (state & ODS_DISABLED) {
        ref = obj->colormod[0].disabled;
        sref = obj->colormod[1].disabled;
    }
    else if ((state & ODS_SELECTED) || manager->casb > -1) {
        ref = obj->colormod[0].pressed;
        sref = obj->colormod[1].pressed;
        DEFSCRBTNUPDEL((void*)manager);
    }
    else if (obj->flags & OBJ_HOVERED) {
        ref = obj->colormod[0].hovered;
        sref = obj->colormod[1].hovered;
    }
    else {
        ref = obj->colormod[0].enabled;
        sref = obj->colormod[1].enabled;
    }

    /** Filling Clip Rgn **/

    brush = (HBRUSH)CreateSolidBrush(ref);
    FillRect(hdc, &rect, brush);

    /** Calculating slider pos **/
    DeleteObject(brush);
    brush = (HBRUSH)CreateSolidBrush(sref);
    GetClientRect(manager->window, &orect);

    RECT slider_rect = rect;

    if (((float)orect.bottom / (float)obj->stm_rect.bottom) < 1) {
        for (int n = 0; n < 4; n++) {
            GetWindowRect(manager->objects[obj->stm_object_ids[n]].window, &brect);
            MapWindowPoints(HWND_DESKTOP, manager->window, (POINT*)&brect, 2);
            switch (n) {
                case 0: trect.left = brect.left; break;
                case 1: trect.top = brect.top; break;
                case 2: trect.right = brect.right; break;
                case 3: trect.bottom = brect.bottom; break;
            }
        }

        if (trect.top < obj->stm_rect.top) {
            slider_rect.top = (float)rect.bottom * (1.0 - (float)trect.bottom / (float)obj->stm_rect.bottom);
        }

        slider_rect.bottom *= ((float)orect.bottom / (float)obj->stm_rect.bottom);
        slider_rect.bottom += slider_rect.top;

        if (obj->rgnw || obj->rgnh) {
            rgn = CreateRoundRectRgn(slider_rect.left, slider_rect.top, slider_rect.right, slider_rect.bottom, obj->rgnw, obj->rgnh);
            SelectClipRgn(hdc, rgn);
        }
        if (rgn)
            FillRgn(hdc, rgn, brush);
        else
            FillRect(hdc, &slider_rect, brush);
    }

    DeleteObject(brush);
    DeleteObject(rgn);
}

SINLINE void _InvalidateManagerListBox(Manager* manager, LPDRAWITEMSTRUCT lpdis) {
    static Manager* lbsh;
    static COLORREF ref = NULL, arw = NULL;
    static Object* obj;
    static RECT rect;
    static UINT state;
    static HBRUSH brush = NULL;
    static HDC hdc;
    static TCHAR text[512];
    static LOGFONT lgf;

    hdc = lpdis->hDC;
    obj = &(manager->objects[GetDlgCtrlID(lpdis->hwndItem) - 1]);
    lbsh = ((Manager *)obj->lbsh);
    state = lpdis->itemState;
    rect = lpdis->rcItem;
    GetObject(obj->marlett, sizeof(LOGFONT), &lgf);

    /** Selecting Button Background **/

    if (state & ODS_DISABLED) {
        SetTextColor(hdc, obj->colormod[2].disabled);
        ref = obj->colormod[0].disabled;
        arw = obj->colormod[1].disabled;
    }
    else if (state & ODS_SELECTED) {
        SetTextColor(hdc, obj->colormod[2].pressed);
        ref = obj->colormod[0].pressed;
        arw = obj->colormod[1].pressed;
    }
    else if (obj->flags & OBJ_HOVERED || IsWindowVisible(lbsh->window)) {
        SetTextColor(hdc, obj->colormod[2].hovered);
        ref = obj->colormod[0].hovered;
        arw = obj->colormod[1].hovered;
    }
    else {
        SetTextColor(hdc, obj->colormod[2].enabled);
        ref = obj->colormod[0].enabled;
        arw = obj->colormod[1].enabled;
    }

    /** Filling Clip Rgn **/

    brush = (HBRUSH)CreateSolidBrush(ref);
    FillRect(hdc, &rect, brush);

    /** Drawing text **/

    SetBkMode(hdc, TRANSPARENT);
    GetWindowText(lpdis->hwndItem, text, _countof(text));
    rect.left += ScaleForDPI(8, manager->scale_dpi);
    DrawText(hdc, text, -1, &rect, obj->dt_format);
    rect.left -= ScaleForDPI(8, manager->scale_dpi);

    /** Drawing arrow **/

    SetTextColor(hdc, arw);

    SelectObject(hdc, obj->marlett);
    rect.right -= ScaleForDPI(5, manager->scale_dpi);
    DrawText(hdc, "u", -1, &rect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    SetTextColor(hdc, ref);
    rect.top -= ScaleForDPI(3, manager->scale_dpi);
    DrawText(hdc, "u", -1, &rect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

    DeleteObject(brush);
}

SINLINE void _InvalidateManagerScrollLinear(Manager* manager, LPDRAWITEMSTRUCT lpdis) {
    static COLORREF ref = NULL, sref = NULL, arw = NULL;
    static Object* obj;
    static RECT rect;
    static UINT state;
    static HBRUSH brush = NULL;
    static HDC hdc;
    static HRGN rgn = NULL;

    hdc = lpdis->hDC;
    obj = &(manager->objects[GetDlgCtrlID(lpdis->hwndItem) - 1]);
    state = lpdis->itemState;
    rect = lpdis->rcItem;

    /** Selecting Rectangle Background **/

    if (state & ODS_DISABLED) {
        ref = obj->colormod[0].disabled;
        sref = obj->colormod[1].disabled;
        arw = obj->colormod[2].disabled;
    }
    else if ((state & ODS_SELECTED) || manager->casb > -1) {
        ref = obj->colormod[0].pressed;
        sref = obj->colormod[1].pressed;
        arw = obj->colormod[2].pressed;
        DEFSCLBTNUPDEL((void*)manager);
    }
    else if (obj->flags & OBJ_HOVERED) {
        ref = obj->colormod[0].hovered;
        sref = obj->colormod[1].hovered;
        arw = obj->colormod[2].hovered;
    }
    else {
        ref = obj->colormod[0].enabled;
        sref = obj->colormod[1].enabled;
        arw = obj->colormod[2].enabled;
    }

    /** Filling Clip Rgn **/

    brush = (HBRUSH)CreateSolidBrush(ref);
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);

    /** Filling Line Rgn **/

    rect.left += rect.bottom / 4;
    rect.right -= rect.bottom / 4;
    rect.top = rect.bottom / 2 - (rect.bottom / 10);
    rect.bottom = rect.top + (rect.bottom / 2 - rect.top) * 2;
    brush = (HBRUSH)CreateSolidBrush(sref);
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);

    /** Filling Arrow Rgn **/

    rect = lpdis->rcItem;
    brush = (HBRUSH)CreateSolidBrush(arw);
    rect.left = ((float)rect.right - (float)rect.bottom / 2) *
                (((float)obj->scr_current - (float)obj->scr_min) / ((float)obj->scr_max - (float)obj->scr_min));
    rect.right = rect.left + rect.bottom / 2;
    FillRect(hdc, &rect, brush);


    DeleteObject(brush);
    DeleteObject(rgn);
}

SINLINE int _InvalidateManagerItem(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    Manager* manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lparam;
    Object* obj = &(manager->objects[GetDlgCtrlID(((LPDRAWITEMSTRUCT)lparam)->hwndItem) - 1]);

    switch (obj->type) {
        case OBJ_RECT:
            _InvalidateManagerRect(manager, lpdis);
            break;
        case OBJ_STATIC:
            _InvalidateManagerStatic(manager, lpdis);
            break;
        case OBJ_BUTTON:
            _InvalidateManagerButton(manager, lpdis);
            break;
        case OBJ_SWITCH:
            _InvalidateManagerSwitch(manager, lpdis);
            break;
        case OBJ_SCROLLBAR:
            _InvalidateManagerScrollBar(manager, lpdis);
            break;
        case OBJ_LISTBOX:
            _InvalidateManagerListBox(manager, lpdis);
            break;
        case OBJ_SCRLINEAR:
            _InvalidateManagerScrollLinear(manager, lpdis);
            break;
        default:
            return DefWindowProc(hwnd, WM_DRAWITEM, wparam, lparam);
    }

    return 1;
}

SINLINE int _InvalidateResizedItems(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    /// hwnd - main_window
    Manager* manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    RECT rect, trect, brect;
    Object* obj;
    int ox, oy, cx, cy, dx, dy; ((void)dx);
    trect.top = 0;

    /** Setting preview for current GIFDesk window **/

    switch (wparam) {
        case SIZE_MINIMIZED:
            if (manager->gfk_count > 0)
                _SetSizableWindow(manager, NULL, POS_NULL);
            break;
        default:
            if (manager->gfk_current >= 0)
                _SetSizableWindow(manager, &manager->gfk[manager->gfk_current], POS_NULL);

            dx = LOWORD(lparam) - manager->width_buff;
            dy = HIWORD(lparam) - manager->height_buff;
            GetWindowRect(manager->window, &rect);
            if (rect.left < 0 || rect.top < 0 || rect.right < 0 || rect.bottom < 0) {
                dx = 0; dy = 0;
            }

            /** Setting align object **/
            for (int i = 0; i < manager->objects_count; i++) {
                if (!&manager->objects[i]) continue;

                obj = &manager->objects[i];
                if (!obj->window || !obj->align)
                    continue;

                GetWindowRect(obj->window, &rect);
                MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);

                ox = rect.left;
                oy = rect.top;
                cx = rect.right - rect.left;
                cy = rect.bottom - rect.top;

                switch (obj->align) {
                    case (OBJ_ALIGNR | OBJ_ALIGNB):
                        ox = LOWORD(lparam) - obj->aor - cx;
                        oy = HIWORD(lparam) - obj->aob - cy;
                        break;
                    case OBJ_ALIGNR:
                        ox = LOWORD(lparam) - obj->aor - cx;
                        break;
                    case OBJ_ALIGNB:
                        oy = HIWORD(lparam) - obj->aob - cy;
                        break;
                    case (OBJ_ALIGNW | OBJ_ALIGNH):
                        ox = obj->aol;
                        oy = obj->aot;
                        cx = LOWORD(lparam) - obj->aol - obj->aor;
                        cy = HIWORD(lparam) - obj->aot - obj->aob;
                        break;
                    case OBJ_ALIGNW:
                        ox = obj->aol;
                        cx = LOWORD(lparam) - obj->aol - obj->aor;
                        break;
                    case OBJ_ALIGNH:
                        oy = obj->aot;
                        cy = HIWORD(lparam) - obj->aot - obj->aob;
                        break;

                    case (OBJ_ALIGNR | OBJ_ALIGNH):
                        ox = LOWORD(lparam) - obj->aor - cx;
                        oy = obj->aot;
                        cy = HIWORD(lparam) - obj->aot - obj->aob;
                        break;
                    case (OBJ_ALIGNB | OBJ_ALIGNW):
                        oy = HIWORD(lparam) - obj->aob - cy;
                        ox = obj->aol;
                        cx = LOWORD(lparam) - obj->aol - obj->aor;
                        break;
                    default:
                        break;
                }
                SetWindowPos(obj->window, NULL, ox, oy, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);

                if (obj->rgnw || obj->rgnh) {
                    if (obj->rgn) { DeleteObject(obj->rgn); obj->rgn = NULL; }
                    obj->rgn = CreateRoundRectRgn(0, 0, cx, cy, obj->rgnw, obj->rgnh);

                    SetWindowRgn(obj->window, obj->rgn, TRUE);
                }
            }

            /** Setting scrolled objects **/
            for (int i = 0; i < manager->objects_count; i++) {
                if (!&manager->objects[i]) continue;
                obj = &manager->objects[i];
                if (obj->type != OBJ_SCROLLBAR) continue;

                for (int n = 0; n < 4; n++) {
                    GetWindowRect(manager->objects[obj->stm_object_ids[n]].window, &brect);
                    MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&brect, 2);
                    switch (n) {
                        case 0: trect.left = brect.left; break;
                        case 1: trect.top = brect.top; break;
                        case 2: trect.right = brect.right; break;
                        case 3: trect.bottom = brect.bottom;
                    }
                }
                for (int j = 0; j < obj->stm_count; j++) {
                    if (obj->type != OBJ_SCROLLBAR ||
                        !(obj->tab & manager->tab) ||
                        obj->stm[j] == -1) break;

                    GetWindowRect(manager->objects[obj->stm[j]].window, &rect);
                    MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);

                    if (trect.top < obj->stm_rect.top && dy > 0) {
                        rect.top += dy;
                        if (!j) obj->stm_offset -= dy;
                    }
                    SetWindowPos(manager->objects[obj->stm[j]].window, NULL,
                                 rect.left,
                                 rect.top,
                                 0,
                                 0,
                                 SWP_NOSIZE | SWP_NOREDRAW | SWP_NOZORDER | SWP_NOACTIVATE);
                }
            }
            break;
    }

    manager->width_buff = LOWORD(lparam);
    manager->height_buff = HIWORD(lparam);
    return 0;
}

SINLINE LRESULT _InvalidateManagerTextBox(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    HDC hdc = (HDC)wparam;
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkColor(hdc, MGR_COLOR_TEXTBOX_DIV);

    return (LRESULT)CreateSolidBrush(MGR_COLOR_TEXTBOX_DIV);
}

/** Handlers **/

SINLINE int _HandleManagerCursor(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    Manager* manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    POINT pnt; RECT rect;
    GetCursorPos(&pnt);

    for (int i = 0; i < manager->objects_count; i++) {
        if (!&manager->objects[i]) continue;

        if (!manager->objects[i].window) continue;

        GetWindowRect(manager->objects[i].window, &rect);
        if (PtInRect(&rect, pnt) && !(manager->objects[i].flags & OBJ_HOVERED)) {
            manager->objects[i].flags |= OBJ_HOVERED;
            InvalidateRect(manager->objects[i].window, NULL, FALSE);
        }
        else if (!PtInRect(&rect, pnt) && (manager->objects[i].flags & OBJ_HOVERED)) {
            manager->objects[i].flags &= ~OBJ_HOVERED;
            InvalidateRect(manager->objects[i].window, NULL, FALSE);
        }
    }

    return DefWindowProc(hwnd, WM_SETCURSOR, wparam, lparam);
}

SINLINE int _HandleManagerWheel(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    static Manager* manager;
    static Object* obj;
    static RECT rect, wrect, trect, brect;
    static POINT pnt;
    int delta = 0;

    manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    GetClientRect(manager->window, &wrect);
    for (int i = 0; i < manager->objects_count; i++) {
        if (!&manager->objects[i]) continue;

        obj = &manager->objects[i];
        if (obj->type != OBJ_SCROLLBAR ||
            !(obj->tab & manager->tab)) continue;

        GetCursorPos(&pnt);
        MapWindowPoints(HWND_DESKTOP, manager->window, &pnt, 1);
        if (!PtInRect(&obj->stm_mouse_rect, pnt)) continue;

        for (int n = 0; n < 4; n++) {
            GetWindowRect(manager->objects[obj->stm_object_ids[n]].window, &brect);
            MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&brect, 2);
            switch (n) {
                case 0:
                    trect.left = brect.left;
                    break;
                case 1:
                    trect.top = brect.top;
                    break;
                case 2:
                    trect.right = brect.right;
                    break;
                case 3:
                    trect.bottom = brect.bottom;
                    break;
            }
        }
        for (int j = 0; j < obj->stm_count; j++) {
            if (!obj->stm[j]) break;
            GetWindowRect(manager->objects[obj->stm[j]].window, &rect);
            MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);

            for (int m = 5; m <= 120; m += 1) {
                delta = (int)((float)GET_WHEEL_DELTA_WPARAM(wparam) / m);
                if (trect.top + delta <= obj->stm_rect.top &&
                    trect.bottom + delta >= wrect.bottom) {
                    rect.top += delta;
                    rect.bottom += delta;
                    if (!j) obj->stm_offset -= delta;
                    break;
                }
            }
            SetWindowPos(manager->objects[obj->stm[j]].window, NULL,
                         rect.left, rect.top,
                         0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

            InvalidateRect(obj->window, NULL, TRUE);
        }
    }
    return DefWindowProc(hwnd, WM_MOUSEWHEEL, wparam, lparam);
}

SINLINE int _HandleSCRLNR(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    Manager* manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    static TCHAR text[64];
    Object* obj;
    static GIFDesk* gfk;

    if (manager->casb >= 0)
        obj = &manager->objects[manager->casb];
    else
        obj = &manager->objects[lparam];

    switch (obj->id) {
        case MANAGER_SCROLLLINEAR_FRAMES:
            gfk = &manager->gfk[manager->gfk_current];
            gfk->schange_frames = 0;
            sprintf(text, "%d/%d", obj->scr_current, gfk->count);
            SetWindowText(manager->objects[obj->scr_info].window, text);
            break;
        case MANAGER_SCROLLLINEAR_SPEED:
            gfk = &manager->gfk[manager->gfk_current];
            if (obj->scr_current % 20 == 0)
                sprintf(text, "%.0fx", (float)obj->scr_current * 0.05);
            else if (obj->scr_current % 2 == 0)
                sprintf(text, "%.1fx", (float)obj->scr_current * 0.05);
            else
                sprintf(text, "%.2fx", (float)obj->scr_current * 0.05);
            SetWindowText(manager->objects[obj->scr_info].window, text);
            break;
        case MANAGER_SCROLLLINEAR_SCALE:
            gfk = &manager->gfk[manager->gfk_current];
            sprintf(text, "%d%%", obj->scr_current);
            SetWindowText(manager->objects[obj->scr_info].window, text);
            break;
        case MANAGER_SCROLLLINEAR_SPEED_ST2:
            if (obj->scr_current % 20 == 0)
                sprintf(text, "%.0fx", (float)obj->scr_current * 0.05);
            else if (obj->scr_current % 2 == 0)
                sprintf(text, "%.1fx", (float)obj->scr_current * 0.05);
            else
                sprintf(text, "%.2fx", (float)obj->scr_current * 0.05);
            SetWindowText(manager->objects[obj->scr_info].window, text);
            break;
        case MANAGER_SCROLLLINEAR_SCALE_ST2:
            sprintf(text, "%d%%", obj->scr_current);

            SetWindowText(manager->objects[obj->scr_info].window, text);
            break;
    }
    UpdateWindow(obj->window);

    return 0;
}

SINLINE int _HandleManagerMouseMove(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    static RECT rect, brect, trect, wrect;
    static POINT pnt;
    trect.bottom = 0;
    trect.top = 0;
    Manager* manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (manager->casb < 0) return DefWindowProc(hwnd, WM_MOUSEMOVE, wparam, lparam);

    Object* obj = &manager->objects[manager->casb];

    if (!manager->cpx_buff && !manager->cpy_buff) {
        manager->cpx_buff = GET_X_LPARAM(lparam);
        manager->cpy_buff = GET_Y_LPARAM(lparam);
        return DefWindowProc(hwnd, WM_MOUSEMOVE, wparam, lparam);
    }
    if (obj->type == OBJ_SCRLINEAR) {
        GetClientRect(obj->window, &rect);

        GetCursorPos(&pnt);
        MapWindowPoints(HWND_DESKTOP, obj->window, (POINT*)&pnt, 1);

        if (pnt.x < 0) pnt.x = 0;
        if (pnt.x > rect.right) pnt.x = rect.right;

        int pos = (int)(((float)pnt.x / (float)rect.right) * ((float)obj->scr_max - (float)obj->scr_min) + 0.5555) + obj->scr_min;
        _SetScrollLinearCurrent(obj, (int)(((float)pnt.x / (float)rect.right) * ((float)obj->scr_max - (float)obj->scr_min) + 0.5555) + obj->scr_min);
        SendMessage(manager->window, WM_SCRLNR, (WPARAM)pos, (LPARAM)obj->window);
    }
    else {
        int dy = GET_Y_LPARAM(lparam) - manager->cpy_buff;

        GetClientRect(manager->window, &wrect);
        for (int n = 0; n < 4; n++) {
            GetWindowRect(manager->objects[obj->stm_object_ids[n]].window, &brect);
            MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&brect, 2);
            switch (n) {
                case 0: trect.left = brect.left; break;
                case 1: trect.top = brect.top; break;
                case 2: trect.right = brect.right; break;
                case 3: trect.bottom = brect.bottom; break;
            }
        }
        GetWindowRect(obj->window, &rect);

        dy = (int)((float)dy * (1.25 / ((float)wrect.bottom / (float)obj->stm_rect.bottom)) + 0.5);

        for (int i = 0; i < obj->stm_count; i++) {
            if (!obj->stm[i]) break;
            if (!dy) continue;

            GetWindowRect(manager->objects[obj->stm[i]].window, &rect);
            MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);

            if (dy > 0) {
               for (int j = dy; j > 0; j--) {
                    if (trect.top - j <= obj->stm_rect.top &&
                        trect.bottom - j >= wrect.bottom) {
                        rect.top -= j;
                        rect.bottom -= j;
                        break;
                    }
                }
            }
            else {
                for (int j = dy; j < 0; j++) {
                    if (trect.top - j <= obj->stm_rect.top &&
                        trect.bottom - j >= wrect.bottom) {
                        rect.top -= j;
                        rect.bottom -= j;
                        break;
                    }
                }
            }

            SetWindowPos(manager->objects[obj->stm[i]].window, NULL,
                         rect.left, rect.top,
                         0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

            InvalidateRect(obj->window, NULL, FALSE);
        }
    }

    manager->cpx_buff = GET_X_LPARAM(lparam);
    manager->cpy_buff = GET_Y_LPARAM(lparam);
    return DefWindowProc(hwnd, WM_MOUSEMOVE, wparam, lparam);
}

SINLINE int _HandleManagerTimer(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    static Manager* manager;
    static POINT pnt; GetCursorPos(&pnt);
    static RECT rect;
    static TCHAR text[16];
    Object* obj;
    manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (wparam) {
        case 1:
            SetCapture(manager->window);

            for (int i = 0; i < manager->objects_count; i++) {
                if (!&manager->objects[i]) continue;

                if (manager->objects[i].type != OBJ_SCROLLBAR) continue;

                GetWindowRect(manager->objects[i].window, &rect);

                if (PtInRect(&rect, pnt)) { manager->casb = i; }
                else continue;
            }
            KillTimer(hwnd, 1);
            break;
        case 2:
            SetCapture(manager->window);

            for (int i = 0; i < manager->objects_count; i++) {
                if (!&manager->objects[i]) continue;
                if (manager->objects[i].type != OBJ_SCRLINEAR) continue;
                GetWindowRect(manager->objects[i].window, &rect);
                if (PtInRect(&rect, pnt) && IsWindowVisible(manager->objects[i].window)) {
                    manager->casb = i;
                }
                else continue;
            }

            KillTimer(hwnd, 2);
            break;
        case 3:
            if (manager->gfk_current < 0) break;
            for (int i = 0; i < manager->objects_count; i++) {
                if (!&manager->objects[i]) continue;

                obj = &manager->objects[i];
                if (obj->id == MANAGER_TEXTBOX_FRAMES) {
                    sprintf(text, "%d/%d", manager->gfk[manager->gfk_current].render_frame + 1,
                            manager->gfk[manager->gfk_current].count);
                    if (manager->gfk[manager->gfk_current].change_frames) SetWindowText(obj->window, text);
                    UpdateWindow(obj->window);
                }
                else if (obj->id == MANAGER_SCROLLLINEAR_FRAMES) {
                    _SetScrollLinearCurrent(obj, manager->gfk[manager->gfk_current].render_frame + 1);
                    UpdateWindow(obj->window);
                }
            }
            break;
    }
    return 0;
}

SINLINE int _HandleLBSActivate(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    Manager* lbsh = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    Object* ref = (Object *)lbsh->parent;

    if (wparam || !IsWindowVisible(lbsh->window)) return DefWindowProc(hwnd, message, wparam, lparam);

    ShowWindow(lbsh->window, SW_HIDE);
    ref->flags |= OBJ_LBSDS;
    InvalidateRect(ref->window, NULL, TRUE);

    return DefWindowProc(hwnd, message, wparam, lparam);
}

SINLINE void _UpdateTabItems(Manager* manager) {
    for (int i = 0; i < manager->objects_count; i++) {
        if (!&manager->objects[i]) continue;

        if (!(manager->tab & manager->objects[i].tab)) ShowWindow(manager->objects[i].window, SW_HIDE);
        else ShowWindow(manager->objects[i].window, SW_SHOW);
    }
    SetWindowPos(manager->window, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOCOPYBITS);
}

SINLINE int _HandleButtonUp(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    static POINT pnt;
    static RECT rect;
    GIFDesk* gfk;
    Manager* manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    GetCursorPos(&pnt);
    MapWindowPoints(HWND_DESKTOP, hwnd, &pnt, 1);
    GetWindowRect(manager->objects[2].window, &rect);
    MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *)&rect, 2);

    if (manager->tab & MGR_MMSF &&
        (!(pnt.x > ScaleForDPI(230, manager->scale_dpi) &&
        pnt.y > ScaleForDPI(20, manager->scale_dpi)) ||
        PtInRect(&rect, pnt)) &&
        manager->casb < 0
        ) {
        _SetSizableWindow(manager, NULL, POS_NULL);

        manager->tab = MGR_MMWF;
        manager->gfk[manager->gfk_current].sfu = 0;
        manager->gfk_current = -1;
        manager->context_id = -1;
        manager->casb = -1;

        _UpdateTabItems(manager);
        for (int i = 0; i < manager->objects_count; i++) {
            if (!&manager->objects[i]) continue;

            if (manager->objects[i].id == MANAGER_BTN_FILE) {
                _SetManagerObjectColori(&manager->objects[i], 0, MGR_COLOR_BUTTON_BACKGROUND_DEFAULT);
                InvalidateRect(manager->objects[i].window, NULL, TRUE);
            }
        }
        for (uint16_t i = 0; i < manager->gfk_count; i++) {
            if (!&manager->gfk[i]) continue;
            SetWindowPos(manager->gfk[i].window,
                         (manager->gfk[i].flags & SETTINGS_AOT) ? HWND_TOPMOST : HWND_NOTOPMOST,
                         0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
        ReleaseCapture();
    }

    if (manager->gfk_current > -1) {
        gfk = &manager->gfk[manager->gfk_current];
        gfk->schange_frames = 1;
        gfk->start_time = GetTime() - (double)gfk->lengths[gfk->render_frame] / ((double)gfk->speed * 0.05);
    }

    WriteSettings(manager);
    ReleaseCapture();
    manager->context_id = -1;
    manager->casb = -1;
    return DefWindowProc(hwnd, WM_LBUTTONUP, wparam, lparam);
}

SINLINE int _HandleContextMenu(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    static Manager* manager;
    static POINT pnt;
    static int index;
    static Object* obj;
    static HMENU menu, submenu;
    manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    index = GetDlgCtrlID((HWND)wparam) - 1;
    obj = &manager->objects[index];
    manager->context_id = index;

    switch (obj->id) {
        case MANAGER_BTN_FILE:
            GetCursorPos(&pnt);

            menu = CreatePopupMenu();
            submenu = CreatePopupMenu();
            AppendMenu(menu, MF_STRING, POPUP_CONFIGUREFILE, "Configure file...");
            AppendMenu(menu, MF_STRING | MF_POPUP, (UINT_PTR)submenu, "Move window to...");
            AppendMenu(menu, MF_SEPARATOR, 0, "");
            AppendMenu(menu, MF_STRING, POPUP_DELETEFILE, "Close file");

            AppendMenu(submenu, MF_STRING, POPUP_MOVEWINDOW_LTC, "Top left corner");
            AppendMenu(submenu, MF_STRING, POPUP_MOVEWINDOW_RTC, "Top right corner");
            AppendMenu(submenu, MF_STRING, POPUP_MOVEWINDOW_C, "Center");
            AppendMenu(submenu, MF_STRING, POPUP_MOVEWINDOW_LLC, "Bottom left corner");
            AppendMenu(submenu, MF_STRING, POPUP_MOVEWINDOW_RLC, "Bottom right corner");

            TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_HORNEGANIMATION,
                           pnt.x, pnt.y, 0, hwnd, NULL);

            DestroyMenu(menu);
            break;
        default:
            break;
    }

    return DefWindowProc(hwnd, WM_LBUTTONUP, wparam, lparam);
}

SINLINE int _HandleDropFiles(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    Manager* manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    /** Drags multiple file **/
//    int cnt = (int)DragQueryFile((HDROP)wparam, 0xFFFFFFFF, NULL, 0);
//    for (int i = 0; i < cnt; i++) {
//        DragQueryFile((HDROP)wparam, i, manager->buff_filepath, MAX_PATH);
//
//        GIFDeskNew((LPVOID)manager);
//    }
//    DragFinish((HDROP)wparam);

    /** Drags only one file **/
    if (DragQueryFile((HDROP)wparam, 0xFFFFFFFF, NULL, 0) > 1) {
        manager->error = MANAGER_WARN_QUERY_TOO_MUCH;
        ManagerHandleError(manager);
        return 0;
    }

    DragQueryFile((HDROP)wparam, 0, manager->buff_filepath, MAX_PATH);
    DragFinish((HDROP)wparam);

    if (!CreateThread(NULL, 0, GIFDeskNew, (LPVOID)manager, 0, NULL)) {
        manager->error = MANAGER_WARN_CREATE_LOOP;
        ManagerHandleError(manager);
        return 0;
    }
    return 1;
}

SINLINE int _HandleKeyDown(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    Manager* manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    GIFDesk* gfk = NULL;

    switch (wparam) {
        case VK_DELETE:
            if (manager->gfk_current == -1) return 0;

            gfk = &manager->gfk[manager->gfk_current];

            manager->context_id = gfk->obj_index;
            SendMessage(manager->window, WM_COMMAND, POPUP_DELETEFILE, 0);
            return 0;
        case VK_SPACE:
            if (manager->gfk_current == -1) return 0;

            gfk = &manager->gfk[manager->gfk_current];
            gfk->start_time = GetTime() - (double)gfk->lengths[gfk->render_frame] / ((double)gfk->speed * 0.05);
            gfk->change_frames ^= 1;
            return 0;
        default:
            return 0;
    }
}

SINLINE int _HandleNotify(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    static Manager* manager;
    static POINT pnt;

    switch (lparam) {
        case WM_LBUTTONUP:
            manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            ShowWindow(manager->window, SW_SHOW);
            break;
        case WM_RBUTTONUP:
            manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            GetCursorPos(&pnt);

            HMENU menu = CreatePopupMenu();
            AppendMenu(menu, MF_STRING, POPUP_OPENGIFDESK, "Open GIFDesk");
            AppendMenu(menu, MF_SEPARATOR, 0, "");
            AppendMenu(menu, MF_STRING, POPUP_QUIT, "Quit");

            TrackPopupMenu(menu, TPM_RIGHTBUTTON, pnt.x, pnt.y, 0, manager->window_tray, NULL);
            DestroyMenu(menu);
            break;
    }
    return 0;
}

/** Commands/API **/

SINLINE int _HandleManagerCommand(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    static Manager* manager;
    static RECT rect, wrect;
    static HMENU menu;
    Object* obj, *scr_obj;
    int index = 0, id = 0, stm_last = 0, stm_delta = 0, stm_current = 0;
    static GIFDesk* gfk;
    static TCHAR text[64], buff[16];
    static DWORD style;
    static HKEY key = NULL;
    static uint32_t value;
    static char* ptr_speed;

    manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    index = GetDlgCtrlID((HWND)lparam) - 1;

    if (lparam != 0 && IsWindow((HWND)lparam)) {
        index = GetDlgCtrlID((HWND)lparam) - 1;
    }

    if (index < 0 || (unsigned int)index >= manager->objects_count) {
        id = LOWORD(wparam);
    }
    else {
        if (manager->objects[index].btndown != NULL && LOWORD(wparam) - 1 == index)
            manager->objects[index].btndown((void *)&manager->objects[index]);

        id = manager->objects[index].id;
    }
    if (!id) id = (int)wparam;

    switch (id) {

        /** BUTTON_ITEMS **/

        case MANAGER_BTN_NEWFILE: {
            if (manager->is_loading) break;
            manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            GetWindowRect(hwnd, &rect);

            menu = CreatePopupMenu();
            AppendMenu(menu, MF_STRING, POPUP_OPENFILE, "Open file...");
            if (manager->gfk_count > 0)
                AppendMenu(menu, MF_STRING, POPUP_CLOSEFILES, "Close all files");

            TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_HORNEGANIMATION,
                           rect.left + ScaleForDPI(8, manager->scale_dpi),
                           rect.top + ScaleForDPI(50, manager->scale_dpi), 0, hwnd, NULL);

            DestroyMenu(menu);
            break;
        }
        case MANAGER_BTN_SETTINGS: {
            if (manager->is_loading) break;
            _HandleButtonUp(hwnd, 0, 0);

            manager->tab = MGR_ST1;
            _UpdateTabItems(manager);

            DragAcceptFiles(manager->window, FALSE);
            for (int i = 0; i < manager->objects_count; i++)
                if (manager->objects[i].id == MANAGER_BTN_GENERAL)
                    PostMessage(manager->window, WM_COMMAND, manager->objects[i].index, (LPARAM)manager->objects[i].window);
            break;
        }

        case MANAGER_BTN_BACK: {
            manager->tab = (manager->gfk_count) ? MGR_MMWF : MGR_MMWnF;
            _UpdateTabItems(manager);

            DragAcceptFiles(manager->window, TRUE);
            break;
        }

        case MANAGER_BTN_FILE: {
            if (manager->is_loading) break;
            if (!IsWindowVisible(manager->window))
                ShowWindow(manager->window, SW_SHOW);

            obj = &manager->objects[index];
            gfk = &manager->gfk[obj->gfk_index];

            _SetSizableWindow(manager, gfk, POS_NULL);

            if (manager->gfk_current > - 1) manager->gfk[manager->gfk_current].sfu = 0;
            manager->gfk_current = gfk->index;
            manager->context_id = gfk->obj_index;
            gfk->sfu = 1;

            SetWindowPos(gfk->window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOREDRAW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

            for (int i = 0; i < manager->objects_count; i++) {
                if (!&manager->objects[i]) continue;

                if (manager->objects[i].id == MANAGER_BTN_FILE) {
                    _SetManagerObjectColori(&manager->objects[i], 0, MGR_COLOR_BUTTON_BACKGROUND_DEFAULT);
                    InvalidateRect(manager->objects[i].window, NULL, TRUE);
                }

                switch (manager->objects[i].id) {
                    case MANAGER_BTN_PFRAME:
                    case MANAGER_BTN_MFRAME:
                    case MANAGER_BTN_PSCALE:
                    case MANAGER_BTN_MSCALE:
                        manager->objects[i].gfk_index = gfk->index;
                        continue;

                    case MANAGER_BTN_FRAMEPAUSE:
                        if (gfk->change_frames)
                            _SetManagerObjectColor(&manager->objects[i],
                                                   MGR_COLOR_BUTTON_BACKGROUND_DIV);
                        else
                            _SetManagerObjectColor(&manager->objects[i],
                                                   MGR_COLOR_BUTTON_BACKGROUND_DIV_T);
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        manager->objects[i].gfk_index = gfk->index;
                        continue;
                    case MANAGER_SCROLLLINEAR_FRAMES:
                        manager->objects[i].scr_max = gfk->count;
                        _SetScrollLinearCurrent(&manager->objects[i], gfk->render_frame + 1);
                        manager->objects[i].gfk_index = gfk->index;
                        continue;
                    case MANAGER_SCROLLLINEAR_SPEED:
                        _SetScrollLinearCurrent(&manager->objects[i], gfk->speed);
                        manager->objects[i].gfk_index = gfk->index;
                        continue;
                    case MANAGER_SCROLLLINEAR_SCALE:
                        _SetScrollLinearCurrent(&manager->objects[i], (int)(gfk->size * 100));
                        manager->objects[i].gfk_index = gfk->index;
                        continue;

                    case MANAGER_TEXTBOX_FRAMES:
                        sprintf(text, "%d/%d", gfk->render_frame + 1, gfk->count);
                        SetWindowText(manager->objects[i].window, text);
                        continue;
                    case MANAGER_TEXTBOX_SPEED:
                        if (gfk->speed % 20 == 0)
                            sprintf(text, "%.0fx", (float)gfk->speed * 0.05);
                        else if (gfk->speed % 2 == 0)
                            sprintf(text, "%.1fx", (float)gfk->speed * 0.05);
                        else
                            sprintf(text, "%.2fx", (float)gfk->speed * 0.05);
                        SetWindowText(manager->objects[i].window, text);
                        continue;
                    case MANAGER_TEXTBOX_SCALE:
                        sprintf(text, "%d%%", (int)(gfk->size * 100));
                        SetWindowText(manager->objects[i].window, text);
                        continue;

                    case MANAGER_SWITCH_AOT:
                        manager->objects[i].gfk_index = gfk->index;
                        if (gfk->flags & SETTINGS_AOT) {
                            manager->objects[i].flags |= OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                        }
                        else {
                            manager->objects[i].flags &= ~OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                        }
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        continue;
                    case MANAGER_SWITCH_DM:
                        manager->objects[i].gfk_index = gfk->index;
                        if (gfk->flags & SETTINGS_DM) {
                            manager->objects[i].flags |= OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                        }
                        else {
                            manager->objects[i].flags &= ~OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                        }

                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        continue;
                    case MANAGER_SWITCH_CT:
                        manager->objects[i].gfk_index = gfk->index;
                        if (gfk->flags & SETTINGS_CT) {
                            manager->objects[i].flags |= OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                        }
                        else {
                            manager->objects[i].flags &= ~OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                        }

                        for (int j = 0; j < manager->objects_count; j++)
                            switch (manager->objects[j].id) {
                                case MANAGER_SWITCH_DM:
                                    if (gfk->flags & SETTINGS_DM ||
                                        gfk->flags & SETTINGS_CT) {
                                        manager->objects[j].flags |= OBJ_BOOL;
                                        _SetManagerObjectColori(&manager->objects[j], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                                    }
                                    else {
                                        manager->objects[j].flags &= ~OBJ_BOOL;
                                        _SetManagerObjectColori(&manager->objects[j], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                                    }
                                case MANAGER_STATIC_DM:
                                    EnableWindow(manager->objects[j].window, (gfk->flags & SETTINGS_CT) ? FALSE : TRUE);
                                    InvalidateRect(manager->objects[j].window, NULL, TRUE);
                                    break;
                            }
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        continue;
                    case MANAGER_SWITCH_HH:
                        manager->objects[i].gfk_index = gfk->index;
                        if (gfk->flags & SETTINGS_HH) {
                            manager->objects[i].flags |= OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                        }
                        else {
                            manager->objects[i].flags &= ~OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                        }
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        continue;
                    case MANAGER_SWITCH_STI:
                        manager->objects[i].gfk_index = gfk->index;
                        if (gfk->flags & SETTINGS_STI) {
                            manager->objects[i].flags |= OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                        }
                        else {
                            manager->objects[i].flags &= ~OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                        }
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        continue;
                    default:
                        continue;
                }
            }
            _SetManagerObjectColori(obj, 0, MGR_COLOR_BUTTON_BACKGROUND_DEFAULT_S);
            InvalidateRect(obj->window, NULL, TRUE);

            manager->tab = MGR_MMSF;
            _UpdateTabItems(manager);
            break;
        }

        case MANAGER_BTN_GENERAL: {
            if (manager->is_loading) break;
            obj = &manager->objects[index];
            for (int i = 0; i < manager->objects_count; i++) {
                if (!&manager->objects[i]) continue;

                switch (manager->objects[i].id) {
                    case MANAGER_BTN_TEMPLATE:
                    case MANAGER_BTN_PLAYBACK:
                        _SetManagerObjectColori(&manager->objects[i], 0, MGR_COLOR_BUTTON_BACKGROUND_DEFAULT);
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        break;
                    case MANAGER_SWITCH_LSB: {
                        RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &key);
                        if (!RegQueryValueEx(key, "GIFDeskUService", NULL, NULL, NULL, NULL)) {
                            manager->objects[i].flags |= OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                        }
                        else {
                            manager->objects[i].flags &= ~OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                        }
                        RegCloseKey(key);
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        break;
                    }
                    case MANAGER_SWITCH_MSTC: {
                        if (manager->flags & 0x1) {
                            manager->objects[i].flags |= OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                        }
                        else {
                            manager->objects[i].flags &= ~OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                        }
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        break;
                    }
                }
            }
            _SetManagerObjectColori(obj, 0, MGR_COLOR_BUTTON_BACKGROUND_DEFAULT_S);

            manager->tab = MGR_ST1;
            _UpdateTabItems(manager);
            break;
        }

        case MANAGER_BTN_TEMPLATE: {
            if (manager->is_loading) break;
            obj = &manager->objects[index];

            for (int i = 0; i < manager->objects_count; i++) {
                if (!&manager->objects[i]) continue;

                switch (manager->objects[i].id) {
                    case MANAGER_BTN_GENERAL:
                    case MANAGER_BTN_PLAYBACK:
                        _SetManagerObjectColori(&manager->objects[i], 0, MGR_COLOR_BUTTON_BACKGROUND_DEFAULT);
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        break;
                    case MANAGER_SCROLLLINEAR_SPEED_ST2:
                        _SetScrollLinearCurrent(&manager->objects[i], manager->speed);
                        break;
                    case MANAGER_SCROLLLINEAR_SCALE_ST2:
                        _SetScrollLinearCurrent(&manager->objects[i], (int)(manager->size * 100));
                        break;

                    case MANAGER_TEXTBOX_SPEED_ST2:
                        if (manager->speed % 20 == 0)
                            sprintf(text, "%.0fx", (float)manager->speed * 0.05);
                        else if (manager->speed % 2 == 0)
                            sprintf(text, "%.1fx", (float)manager->speed * 0.05);
                        else
                            sprintf(text, "%.2fx", (float)manager->speed * 0.05);
                        SetWindowText(manager->objects[i].window, text);
                        break;
                    case MANAGER_TEXTBOX_SCALE_ST2:
                        sprintf(text, "%d%%", (int)(manager->size * 100));
                        SetWindowText(manager->objects[i].window, text);
                        break;

                    case MANAGER_SWITCH_AOT_ST2:
                        if (manager->flags & SETTINGS_AOT) {
                            manager->objects[i].flags |= OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                        }
                        else {
                            manager->objects[i].flags &= ~OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                        }
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        continue;
                    case MANAGER_SWITCH_DM_ST2:
                        if (manager->flags & SETTINGS_DM) {
                            manager->objects[i].flags |= OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                        }
                        else {
                            manager->objects[i].flags &= ~OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                        }

                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        continue;
                    case MANAGER_SWITCH_CT_ST2:
                        if (manager->flags & SETTINGS_CT) {
                            manager->objects[i].flags |= OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                        }
                        else {
                            manager->objects[i].flags &= ~OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                        }
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);

                        for (int j = 0; j < manager->objects_count; j++)
                            switch (manager->objects[j].id) {
                                case MANAGER_SWITCH_DM_ST2:
                                    if (manager->flags & SETTINGS_DM ||
                                        manager->flags & SETTINGS_CT) {
                                        manager->objects[j].flags |= OBJ_BOOL;
                                        _SetManagerObjectColori(&manager->objects[j], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                                    }
                                    else {
                                        manager->objects[j].flags &= ~OBJ_BOOL;
                                        _SetManagerObjectColori(&manager->objects[j], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                                    }
                                case MANAGER_STATIC_DM_ST2:
                                    EnableWindow(manager->objects[j].window, (manager->flags & SETTINGS_CT) ? FALSE : TRUE);
                                    InvalidateRect(manager->objects[j].window, NULL, TRUE);
                                    break;
                            }
                        continue;
                    case MANAGER_SWITCH_HH_ST2:
                        if (manager->flags & SETTINGS_HH) {
                            manager->objects[i].flags |= OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                        }
                        else {
                            manager->objects[i].flags &= ~OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                        }
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        continue;
                    case MANAGER_SWITCH_STI_ST2:
                        if (manager->flags & SETTINGS_STI) {
                            manager->objects[i].flags |= OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                        }
                        else {
                            manager->objects[i].flags &= ~OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                        }
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        continue;
                }
            }
            _SetManagerObjectColori(obj, 0, MGR_COLOR_BUTTON_BACKGROUND_DEFAULT_S);

            manager->tab = MGR_ST2;
            _UpdateTabItems(manager);
            break;
        }

        case MANAGER_BTN_PLAYBACK: {
            if (manager->is_loading) break;
            obj = &manager->objects[index];
            for (int i = 0; i < manager->objects_count; i++) {
                if (!&manager->objects[i]) continue;

                switch (manager->objects[i].id) {
                    case MANAGER_BTN_TEMPLATE:
                    case MANAGER_BTN_GENERAL:
                        _SetManagerObjectColori(&manager->objects[i], 0, MGR_COLOR_BUTTON_BACKGROUND_DEFAULT);
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        break;
                    case MANAGER_SWITCH_SP: {
                        if (manager->flags & SETTINGS_SP) {
                            manager->objects[i].flags |= OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                        }
                        else {
                            manager->objects[i].flags &= ~OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                        }
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        break;
                    }
                }
            }
            _SetManagerObjectColori(obj, 0, MGR_COLOR_BUTTON_BACKGROUND_DEFAULT_S);

            manager->tab = MGR_ST3;
            _UpdateTabItems(manager);
            break;
        }

        case MANAGER_BTN_FRAMEPAUSE: {
            obj = &manager->objects[index];
            gfk = &manager->gfk[obj->gfk_index];

            gfk->start_time = GetTime() - (double)gfk->lengths[gfk->render_frame] / ((double)gfk->speed * 0.05);
            if (gfk->change_frames) {
                gfk->change_frames = 0;
                gfk->schange_frames = 1;
            }
            else {
                gfk->change_frames = 1;
                gfk->schange_frames = 1;
            }
            if (gfk->change_frames)
                _SetManagerObjectColor(obj, MGR_COLOR_BUTTON_BACKGROUND_DIV, MGR_COLOR_BUTTON_TEXT_DEFAULT);
            else
                _SetManagerObjectColor(obj, MGR_COLOR_BUTTON_BACKGROUND_DIV_T, MGR_COLOR_BUTTON_TEXT_DEFAULT);
            break;
        }

        case MANAGER_BTN_PFRAME:
        case MANAGER_BTN_PSPEED:
        case MANAGER_BTN_PSCALE:
        case MANAGER_BTN_PSPEED_ST2:
        case MANAGER_BTN_PSCALE_ST2: {
            obj = &manager->objects[index];
            scr_obj = &manager->objects[obj->scr_index];

            _SetScrollLinearCurrent(scr_obj, scr_obj->scr_current + 1);
            SendMessage(manager->window, WM_SCRLNR, (WPARAM)scr_obj->scr_current, (LPARAM)scr_obj->index);
            WriteSettings(manager);
            break;
        }

        case MANAGER_BTN_MFRAME:
        case MANAGER_BTN_MSPEED:
        case MANAGER_BTN_MSCALE:
        case MANAGER_BTN_MSPEED_ST2:
        case MANAGER_BTN_MSCALE_ST2: {
            obj = &manager->objects[index];
            scr_obj = &manager->objects[obj->scr_index];

            _SetScrollLinearCurrent(scr_obj, scr_obj->scr_current - 1);
            SendMessage(manager->window, WM_SCRLNR, (WPARAM)scr_obj->scr_current, (LPARAM)scr_obj->index);
            WriteSettings(manager);
            break;
        }

        case MANAGER_SWITCH_LSB: {
            RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &key);
            if (RegQueryValueEx(key, "GIFDeskUService", NULL, NULL, NULL, NULL)) {
                RegCloseKey(key);
                char param[512];
                char filepath[MAX_PATH];
                GetModuleFileName(NULL, filepath, sizeof(filepath));
                sprintf(param, "\"%s\" -s", filepath);

                RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                               0, "", REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &key, 0);

                if (!key) break;
                if (RegSetValueEx(key, "GIFDeskUService", 0, REG_SZ, (LPBYTE)param, strlen(param))) {
                    manager->error = MANAGER_WARN_REG;
                    ManagerHandleError(manager);
                }
                RegCloseKey(key);
            }
            else {
                RegCloseKey(key);
                RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                             0, KEY_SET_VALUE, &key);

                if (!key) { break; }
                if (RegDeleteValue(key, (LPCSTR)"GIFDeskUService")) {
                    manager->error = MANAGER_WARN_REG;
                    ManagerHandleError(manager);
                }
                RegCloseKey(key);
            }
            break;
        }

        case MANAGER_SWITCH_MSTC: {
            obj = &manager->objects[index];

            if (obj->flags & OBJ_BOOL) manager->flags |= SETTINGS_MSTC;
            else manager->flags &= ~SETTINGS_MSTC;
            WriteSettings(manager);
            break;
        }

        case MANAGER_SWITCH_AOT: {
            obj = &manager->objects[index];
            gfk = &manager->gfk[obj->gfk_index];

            if (obj->flags & OBJ_BOOL) gfk->flags |= SETTINGS_AOT;
            else gfk->flags &= ~SETTINGS_AOT;
            SetWindowPos(gfk->window, (gfk->flags & SETTINGS_AOT) ? HWND_TOPMOST : HWND_NOTOPMOST,
                         0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW);
            WriteSettings(manager);
            break;
        }
        case MANAGER_SWITCH_STI: {
            obj = &manager->objects[index];
            gfk = &manager->gfk[obj->gfk_index];

            if (obj->flags & OBJ_BOOL) gfk->flags |= SETTINGS_STI;
            else gfk->flags &= ~SETTINGS_STI;

            style = GetWindowLong(gfk->window, GWL_EXSTYLE);
            SetWindowLong(gfk->window, GWL_EXSTYLE,
                          (gfk->flags & SETTINGS_STI) ? (style | WS_EX_APPWINDOW) & ~WS_EX_TOOLWINDOW :
                                                        (style | WS_EX_TOOLWINDOW) & ~WS_EX_APPWINDOW);
            PostMessage(gfk->window, WM_LBUTTONUP, 0, 0);
            WriteSettings(manager);
            break;
        }
        case MANAGER_SWITCH_DM: {
            obj = &manager->objects[index];
            gfk = &manager->gfk[obj->gfk_index];

            if (obj->flags & OBJ_BOOL) gfk->flags |= SETTINGS_DM;
            else gfk->flags &= ~SETTINGS_DM;
            WriteSettings(manager);
            break;
        }
        case MANAGER_SWITCH_HH: {
            obj = &manager->objects[index];
            gfk = &manager->gfk[obj->gfk_index];

            if (obj->flags & OBJ_BOOL) gfk->flags |= SETTINGS_HH;
            else gfk->flags &= ~SETTINGS_HH;
            WriteSettings(manager);
            break;
        }
        case MANAGER_SWITCH_CT: {
            obj = &manager->objects[index];
            gfk = &manager->gfk[obj->gfk_index];

            if (obj->flags & OBJ_BOOL) gfk->flags |= SETTINGS_CT;
            else gfk->flags &= ~SETTINGS_CT;

            for (int i = 0; i < manager->objects_count; i++)
                switch (manager->objects[i].id) {
                    case MANAGER_SWITCH_DM:
                        if (gfk->flags & SETTINGS_DM ||
                            gfk->flags & SETTINGS_CT) {
                            manager->objects[i].flags |= OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                        }
                        else {
                            manager->objects[i].flags &= ~OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                        }
                    case MANAGER_STATIC_DM:
                        EnableWindow(manager->objects[i].window, (gfk->flags & SETTINGS_CT) ? FALSE : TRUE);
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        break;
                }

            style = GetWindowLong(gfk->window, GWL_EXSTYLE);
            SetWindowLong(gfk->window, GWL_EXSTYLE,
                          (gfk->flags & SETTINGS_CT) ? (style | WS_EX_TRANSPARENT) :
                                                               (style & ~WS_EX_TRANSPARENT) );
            SetTimer(gfk->window, 2, 20, NULL);
            WriteSettings(manager);
            break;
        }

        case MANAGER_SWITCH_AOT_ST2: {
            obj = &manager->objects[index];
            if (obj->flags & OBJ_BOOL) manager->flags |= SETTINGS_AOT;
            else manager->flags &= ~SETTINGS_AOT;

            WriteSettings(manager);
            break;
        }
        case MANAGER_SWITCH_STI_ST2: {
            obj = &manager->objects[index];
            if (obj->flags & OBJ_BOOL) manager->flags |= SETTINGS_STI;
            else manager->flags &= ~SETTINGS_STI;

            WriteSettings(manager);
            break;
        }
        case MANAGER_SWITCH_DM_ST2: {
            obj = &manager->objects[index];
            if (obj->flags & OBJ_BOOL) manager->flags |= SETTINGS_DM;
            else manager->flags &= ~SETTINGS_DM;

            WriteSettings(manager);
            break;
        }
        case MANAGER_SWITCH_HH_ST2: {
            obj = &manager->objects[index];
            gfk = &manager->gfk[obj->gfk_index];
            if (obj->flags & OBJ_BOOL) manager->flags |= SETTINGS_HH;
            else {
                manager->flags &= ~SETTINGS_HH;
                SetLayeredWindowAttributes(gfk->window, 0x0, 0, LWA_COLORKEY);
            }

            WriteSettings(manager);
            break;
        }
        case MANAGER_SWITCH_CT_ST2: {
            obj = &manager->objects[index];
            if (obj->flags & OBJ_BOOL) manager->flags |= SETTINGS_CT;
            else manager->flags &= ~SETTINGS_CT;

            for (int i = 0; i < manager->objects_count; i++)
                switch (manager->objects[i].id) {
                    case MANAGER_SWITCH_DM_ST2:
                        if (manager->flags & SETTINGS_DM ||
                            manager->flags & SETTINGS_CT) {
                            manager->objects[i].flags |= OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
                        }
                        else {
                            manager->objects[i].flags &= ~OBJ_BOOL;
                            _SetManagerObjectColori(&manager->objects[i], 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
                        }
                    case MANAGER_STATIC_DM_ST2:
                        EnableWindow(manager->objects[i].window, (manager->flags & SETTINGS_CT) ? FALSE : TRUE);
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                        break;
                }

            WriteSettings(manager);
            break;
        }

        case MANAGER_SWITCH_SP: {
            obj = &manager->objects[index];
            if (obj->flags & OBJ_BOOL) manager->flags |= SETTINGS_SP;
            else manager->flags &= ~SETTINGS_SP;
            WriteSettings(manager);
            break;
        }

        case MANAGER_TEXTBOX_FRAMES: {
            if (!IsWindowVisible((HWND)lparam)) break;
            GetWindowText((HWND)lparam, text, 64);
            obj = &manager->objects[index];
            gfk = &manager->gfk[manager->gfk_current];
            sscanf(text, "%u", &value);
            sprintf(buff, "/%u", gfk->count);

            if (!value ||
                !_ContainsOnlyStr(text, "1234567890/") ||
                !strstr(text, buff) ||
                _CountChr('/', text) > 1) {
                gfk->render_frame = 0;
                sprintf(text, "%d/%d", gfk->render_frame + 1, gfk->count);
                SetWindowText((HWND)lparam, text);
                ShowFrame(manager, gfk->index, 1);

                MessageBeep(MB_ICONEXCLAMATION);
                break;
            }
            if (value > gfk->count) {
                gfk->render_frame = gfk->count - 1;
                sprintf(text, "%d/%d", gfk->render_frame + 1, gfk->count);
                SetWindowText((HWND)lparam, text);

                MessageBeep(MB_ICONEXCLAMATION);
            }
            gfk->render_frame = value - 1;

            ShowFrame(manager, gfk->index, 1);
            scr_obj = &manager->objects[obj->scr_index];
            _SetScrollLinearCurrent(scr_obj, value);
            break;
        }

        case MANAGER_TEXTBOX_SPEED: {
            if (!IsWindowVisible((HWND)lparam)) break;
            GetWindowText((HWND)lparam, text, 64);
            obj = &manager->objects[index];
            gfk = &manager->gfk[manager->gfk_current];
            value = (uint16_t)(strtof(text, &ptr_speed) * 20);

            if (!strchr(text, 'x') ||
                !_ContainsOnlyStr(text, "1234567890.x") ||
                _CountChr('.', text) > 1 ||
                _CountChr('x', text) > 1) {
                gfk->start_time = GetTime() - (double)gfk->lengths[gfk->render_frame] / ((double)gfk->speed * 0.05);

                if (gfk->speed % 20 == 0)
                    sprintf(text, "%.0fx", (float)gfk->speed * 0.05);
                else if (gfk->speed % 2 == 0)
                    sprintf(text, "%.1fx", (float)gfk->speed * 0.05);
                else
                    sprintf(text, "%.2fx", (float)gfk->speed * 0.05);
                SetWindowText((HWND)lparam, text);
                ShowFrame(manager, gfk->index, 1);

                MessageBeep(MB_ICONEXCLAMATION);
                break;
            }
            if (!value) value = 1;
            if (value > 2000) {
                value = 2000;

                if (value % 20 == 0)
                    sprintf(text, "%.0fx", (float)value * 0.05);
                else if (value % 2 == 0)
                    sprintf(text, "%.1fx", (float)value * 0.05);
                else
                    sprintf(text, "%.2fx", (float)value * 0.05);
                SetWindowText((HWND)lparam, text);

                MessageBeep(MB_ICONEXCLAMATION);
            }

            if (gfk->speed != value) {
                gfk->speed = value;
                gfk->start_time = GetTime() - (double)gfk->lengths[gfk->render_frame] / ((double)gfk->speed * 0.05);
                ShowFrame(manager, gfk->index, 1);
                scr_obj = &manager->objects[obj->scr_index];
                _SetScrollLinearCurrent(scr_obj, value);
                WriteSettings(manager);
            }
            break;
        }

        case MANAGER_TEXTBOX_SCALE: {
            if (!IsWindowVisible((HWND)lparam)) break;
            GetWindowText((HWND)lparam, text, 64);
            obj = &manager->objects[index];
            gfk = &manager->gfk[manager->gfk_current];
            sscanf(text, "%u", &value);

            if (!value ||
                !strchr(text, '%') ||
                !_ContainsOnlyStr(text, "1234567890%") ||
                _CountChr('%', text) > 1) {
                sprintf(text, "%u%%", (uint16_t)(gfk->size * 100));
                SetWindowText((HWND)lparam, text);
                ShowFrame(manager, gfk->index, 1);

                MessageBeep(MB_ICONEXCLAMATION);
                break;
            }
            if (value < 1) value = 1;
            if (value > 200) {
                value = 200;
                sprintf(text, "%u%%", value);

                SetWindowText((HWND)lparam, text);
                MessageBeep(MB_ICONEXCLAMATION);
            }
            gfk->size = (float)value / 100;

            if (gfk->size == 1)
                ChangeTexFilt(manager, gfk->index, GL_NEAREST);
            else
                ChangeTexFilt(manager, gfk->index, GL_LINEAR);
            ShowFrame(manager, gfk->index, 1);
            scr_obj = &manager->objects[obj->scr_index];
            _SetScrollLinearCurrent(scr_obj, value);
            WriteSettings(manager);
            break;
        }

        case MANAGER_TEXTBOX_SPEED_ST2: {
            if (!IsWindowVisible((HWND)lparam)) break;
            GetWindowText((HWND)lparam, text, 64);
            obj = &manager->objects[index];
            value = (uint16_t)(strtof(text, &ptr_speed) * 20);

            if (!strchr(text, 'x') ||
                !_ContainsOnlyStr(text, "1234567890.x") ||
                _CountChr('.', text) > 1 ||
                _CountChr('x', text) > 1) {
                if (manager->speed % 20 == 0)
                    sprintf(text, "%.0fx", (float)manager->speed * 0.05);
                else if (manager->speed % 2 == 0)
                    sprintf(text, "%.1fx", (float)manager->speed * 0.05);
                else
                    sprintf(text, "%.2fx", (float)manager->speed * 0.05);
                SetWindowText((HWND)lparam, text);
                ShowFrame(manager, gfk->index, 1);

                MessageBeep(MB_ICONEXCLAMATION);
                break;
            }
            if (!value) value = 1;
            if (value > 2000) {
                value = 2000;

                if (value % 20 == 0)
                    sprintf(text, "%.0fx", (float)value * 0.05);
                else if (value % 2 == 0)
                    sprintf(text, "%.1fx", (float)value * 0.05);
                else
                    sprintf(text, "%.2fx", (float)value * 0.05);
                SetWindowText((HWND)lparam, text);

                MessageBeep(MB_ICONEXCLAMATION);
            }
            manager->speed = value;

            ShowFrame(manager, gfk->index, 1);
            scr_obj = &manager->objects[obj->scr_index];
            _SetScrollLinearCurrent(scr_obj, value);
            WriteSettings(manager);

            break;
        }

        case MANAGER_TEXTBOX_SCALE_ST2: {
            if (!IsWindowVisible((HWND)lparam)) break;
            GetWindowText((HWND)lparam, text, 64);
            obj = &manager->objects[index];
            sscanf(text, "%u", &value);

            if (!value ||
                !strchr(text, '%') ||
                !_ContainsOnlyStr(text, "1234567890%") ||
                _CountChr('%', text) > 1) {
                sprintf(text, "%u%%", (uint16_t)(manager->size * 100));
                SetWindowText((HWND)lparam, text);
                ShowFrame(manager, gfk->index, 1);

                MessageBeep(MB_ICONEXCLAMATION);
                break;
            }
            if (value < 1) value = 1;
            if (value > 200) {
                value = 200;
                sprintf(text, "%u%%", value);

                SetWindowText((HWND)lparam, text);
                MessageBeep(MB_ICONEXCLAMATION);
            }
            manager->size = (float)value / 100;

            ShowFrame(manager, gfk->index, 1);
            scr_obj = &manager->objects[obj->scr_index];
            _SetScrollLinearCurrent(scr_obj, value);
            WriteSettings(manager);
            printf("--------------------------------\n");
            break;
        }

        /** THREAD_ITEMS **/

        case MANAGER_THREAD_CREATEOBJECT: {
            uint16_t gfk_index = (LPARAM)lparam;
            obj = _CreateManagerButton(manager, 20, 20 + 20 * (gfk_index), 199, 20, TRUE, MANAGER_BTN_FILE,
                                       FONT_MAIN_ID, manager->gfk[gfk_index].filename, FALSE);

            if (!obj) {
                manager->error = MANAGER_WARN_CREATE_OBJ;
                ManagerHandleError(manager);
                return 0;
            }

            _SetItemTab(obj, MGR_MMWF | MGR_MMSF);
            _SetTextFormat(obj, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            _SetTextAlign(obj, 3, 0, 0, 0);
            _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_FILES);

            obj->gfk_index = manager->gfk[gfk_index].index;
            manager->gfk[gfk_index].obj_index = obj->index;

            if (manager->tab != MGR_MMSF)
                manager->tab  = MGR_MMWF;

            _UpdateTabItems(manager);
            return 1;
        }

        case MANAGER_THREAD_CREATEWINDOW: {
            uint16_t gfk_index = (LPARAM)lparam;
            if (!GIFDeskWindow(manager, gfk_index)) return 0;
            else return 1;
        }

        /** POPUP_ITEMS **/

        case POPUP_OPENFILE: {
            if (!SetSettings(manager)) break;

            if (!CreateThread(NULL, 0, GIFDeskNew, (LPVOID)manager, 0, NULL)) {
                manager->error = MANAGER_WARN_CREATE_THREAD;
                ManagerHandleError(manager);
                return 0;
            }
            break;
        }

        case POPUP_CLOSEFILES: {
            for (int i = manager->gfk_count - 1; i >= 0; i--) {
                manager->context_id = manager->gfk[i].obj_index;
                SendMessage(manager->window, WM_COMMAND, POPUP_DELETEFILE, lparam);
            }
            break;
        }

        case POPUP_CONFIGUREFILE: {
            PostMessage(manager->window, WM_COMMAND, index + 1, (LPARAM)manager->objects[manager->context_id].window);
            manager->context_id = -1;
            break;
        }

        case POPUP_DELETEFILE: {
            if (manager->context_id < 0) break;

            obj = &manager->objects[manager->context_id];
            gfk = &manager->gfk[obj->gfk_index];

            uint16_t obj_index = obj->index;
            uint16_t gfk_index = gfk->index;

            /** Clearing GFK **/

            GIFDeskRelease(manager, gfk_index);

            for (uint16_t i = gfk_index; i < manager->gfk_count - 1; i++) {
                manager->gfk[i + 1].render_thread = 0;

                memmove(&manager->gfk[i], &manager->gfk[i + 1], sizeof(GIFDesk));
                manager->gfk[i].index = i;
            }

            manager->gfk_count--;
            if (manager->gfk_count) {
                GIFDesk* buff = realloc(manager->gfk, sizeof(GIFDesk) * manager->gfk_count);
                if (buff) manager->gfk = buff;
            }
            else {
                free(manager->gfk);
                manager->gfk = NULL;
            }

            for (uint16_t i = gfk_index; i < manager->gfk_count; i++) {
                SetWindowLongPtr(manager->gfk[i].window, GWLP_USERDATA, (LONG_PTR)&manager->gfk[i]);
                if (i >= gfk_index) {
                    manager->gfk[i].render_thread = 1;
                    GIFDeskLoop* args = malloc(sizeof(GIFDeskLoop));
                    args->manager = manager;
                    args->gfk = &manager->gfk[i];
                    args->gfk_index = manager->gfk[i].index;
                    CreateThread(NULL, 0, Loop, (LPVOID)args, 0, NULL);
                }
            }

            /** Clearing Object **/

            for (int i = 0; i < manager->objects_count; i++)
                if (manager->objects[i].gfk_index > gfk_index)
                    manager->objects[i].gfk_index--;

            _DeleteObject(obj);

            for (uint16_t i = obj_index; i < manager->objects_count - 1; i++) {
                memmove(&manager->objects[i], &manager->objects[i + 1], sizeof(Object));
                manager->objects[i].index = i;
                SetWindowLongPtr(manager->objects[i].window, GWLP_ID, (LONG_PTR)(i + 1));

                if (manager->objects[i].id != MANAGER_BTN_FILE &&
                    obj_index != i) continue;
                GetWindowRect(manager->objects[i].window, &rect);
                MapWindowPoints(HWND_DESKTOP, manager->window, (POINT*)&rect, 2);
                SetWindowPos(manager->objects[i].window, NULL,
                             rect.left, rect.top - ScaleForDPI(20, manager->scale_dpi),
                             0, 0,
                             SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
            }

            manager->objects_count--;
            if (manager->objects_count > 0) {
                Object* buff = realloc(manager->objects, sizeof(Object) * manager->objects_count);
                if (buff) manager->objects = buff;
            }
            else {
                free(manager->objects);
                manager->objects = NULL;
            }

            /** Clearing *stm **/

            for (int i = 0; i < manager->objects_count; i++) {
                if ( manager->objects[i].type != OBJ_SCROLLBAR ||
                    !manager->objects[i].stm_count) continue;

                BOOL is_obj = FALSE;
                for (uint16_t j = 0; j < manager->objects[i].stm_count; j++)
                    if (manager->objects[i].stm[j] == obj_index) is_obj = TRUE;
                if (!is_obj) continue;

                manager->objects[i].stm_count--;

                for (int j = 0; j < manager->objects[i].stm_count; j++) {
                    if (manager->objects[i].stm[j] >= obj_index)
                        manager->objects[i].stm[j] = --manager->objects[i].stm[j + 1];
                }

                int* stm_buff = malloc(sizeof(int) * (manager->objects[i].stm_count));
                memcpy(stm_buff, manager->objects[i].stm, sizeof(int) * (manager->objects[i].stm_count));

                free(manager->objects[i].stm); manager->objects[i].stm = NULL;
                memset(manager->objects[i].stm_object_ids, 0, sizeof(int) * 4);
                memset(&manager->objects[i].stm_rect, 0, sizeof(RECT));

                _SetScrollObjectsM(manager, i, manager->objects[i].stm_count, stm_buff);

                if (!manager->objects[i].stm_count) continue;

                stm_last = manager->objects[i].stm[manager->objects[i].stm_count - 1];
                GetWindowRect(manager->objects[stm_last].window, &rect);
                MapWindowPoints(HWND_DESKTOP, manager->window, (POINT*)&rect, 2);
                GetClientRect(manager->window, &wrect);

                /** Check overflow pos **/

                if (rect.bottom < wrect.bottom) {
                    stm_delta = wrect.bottom - rect.bottom;
                    if (stm_delta <= manager->objects[i].stm_offset) {
                        manager->objects[i].stm_offset -= stm_delta;
                        for (int j = 0; j < manager->objects[i].stm_count; j++) {
                            stm_current = manager->objects[i].stm[j];
                            GetWindowRect(manager->objects[stm_current].window, &rect);
                            MapWindowPoints(HWND_DESKTOP, manager->window, (POINT*)&rect, 2);

                            SetWindowPos(manager->objects[stm_current].window, NULL,
                                         rect.left, rect.top + stm_delta, 0, 0,
                                         SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
                        }
                    }
                }
            }

            if (manager->gfk_current == gfk_index) {
                manager->gfk_current = -1;
                for (int i = 0; i < manager->objects_count; i++) {
                    if (!&manager->objects[i]) continue;

                    if (manager->objects[i].id == MANAGER_BTN_FILE) {
                        _SetManagerObjectColori(&manager->objects[i], 0, MGR_COLOR_BUTTON_BACKGROUND_DEFAULT);
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                    }
                }
            }
            else if (manager->gfk_current > gfk_index) manager->gfk_current--;

            if (manager->casb == obj_index) manager->casb = -1;
            else if (manager->casb > obj_index) manager->casb--;

            if (manager->context_id == obj_index) {
                manager->context_id = -1;

                for (int i = 0; i < manager->objects_count; i++) {
                    if (!&manager->objects[i]) continue;

                    if (manager->objects[i].id == MANAGER_BTN_FILE) {
                        _SetManagerObjectColori(&manager->objects[i], 0, MGR_COLOR_BUTTON_BACKGROUND_DEFAULT);
                        InvalidateRect(manager->objects[i].window, NULL, TRUE);
                    }
                }
            }
            else if (manager->context_id > obj_index) manager->context_id--;

            if (manager->gfk_count && manager->gfk_current > -1 && manager->context_id > -1) manager->tab = MGR_MMSF;
            else if (manager->gfk_count) manager->tab = MGR_MMWF;
            else manager->tab = MGR_MMWnF;

            _UpdateTabItems(manager);
            InvalidateRect(manager->window, NULL, TRUE);
            if (!lparam) {
                WriteSettings(manager);
            }
            break;
        }

        case POPUP_MOVEWINDOW_LTC: { // Top left corner
            obj = &manager->objects[manager->context_id];
            gfk = &manager->gfk[obj->gfk_index];

            HMONITOR mtr = MonitorFromWindow(gfk->window, MONITOR_DEFAULTTONEAREST);
            MONITORINFO mtf = {sizeof(MONITORINFO)};
            GetMonitorInfo(mtr, &mtf);

            _SetSizableWindow(manager, gfk, POS_LTC);
            SetWindowPos(gfk->window, NULL,
                         mtf.rcWork.left,
                         mtf.rcWork.top,
                         0,
                         0,
                         SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
            if (manager->gfk_current != gfk->index)
                _SetSizableWindow(manager, NULL, POS_NULL);

            gfk->x = (int16_t)mtf.rcWork.left;
            gfk->y = (int16_t)mtf.rcWork.top;
            WriteSettings(manager);
            break;
        }
        case POPUP_MOVEWINDOW_RTC: { // Top right corner
            obj = &manager->objects[manager->context_id];
            gfk = &manager->gfk[obj->gfk_index];

            MONITORINFO mtf = {sizeof(MONITORINFO)};
            HMONITOR mtr = MonitorFromWindow(gfk->window, MONITOR_DEFAULTTONEAREST);
            GetMonitorInfo(mtr, &mtf);
            GetClientRect(gfk->window, &rect);

            if (manager->gfk_current == gfk->index)
                _SetSizableWindow(manager, gfk, POS_RTC);

            SetWindowPos(gfk->window, NULL,
                         mtf.rcWork.right - rect.right,
                         mtf.rcWork.top,
                         0,
                         0,
                         SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

            GetWindowRect(gfk->window, &rect);
            gfk->x = (int16_t)(mtf.rcWork.right - (LONG)((float)gfk->npotwidth * gfk->size + 0.5555));
            gfk->y = (int16_t)mtf.rcWork.top;
            WriteSettings(manager);
            break;
        }
        case POPUP_MOVEWINDOW_C: { // Center
            obj = &manager->objects[manager->context_id];
            gfk = &manager->gfk[obj->gfk_index];

            MONITORINFO mtf = {sizeof(MONITORINFO)};
            HMONITOR mtr = MonitorFromWindow(gfk->window, MONITOR_DEFAULTTONEAREST);
            GetMonitorInfo(mtr, &mtf);
            GetClientRect(gfk->window, &rect);

            if (manager->gfk_current == gfk->index)
                _SetSizableWindow(manager, gfk, POS_C);

            SetWindowPos(gfk->window, NULL,
                         mtf.rcWork.right / 2 - rect.right / 2,
                         mtf.rcWork.bottom / 2 - rect.bottom / 2,
                         0,
                         0,
                         SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

            GetWindowRect(gfk->window, &rect);
            gfk->x = (int16_t)(mtf.rcWork.right / 2 - (LONG)(((float)gfk->npotwidth * gfk->size) / 2 + 0.5555));
            gfk->y = (int16_t)(mtf.rcWork.bottom / 2 - (LONG)(((float)gfk->npotheight * gfk->size) / 2 + 0.5555));
            WriteSettings(manager);
            break;
        }
        case POPUP_MOVEWINDOW_LLC: { // Bottom left corner
            obj = &manager->objects[manager->context_id];
            gfk = &manager->gfk[obj->gfk_index];

            MONITORINFO mtf = {sizeof(MONITORINFO)};
            HMONITOR mtr = MonitorFromWindow(gfk->window, MONITOR_DEFAULTTONEAREST);
            GetMonitorInfo(mtr, &mtf);
            GetClientRect(gfk->window, &rect);

            if (manager->gfk_current == gfk->index)
                _SetSizableWindow(manager, gfk, POS_LLC);

            SetWindowPos(gfk->window, NULL,
                         mtf.rcWork.left,
                         mtf.rcWork.bottom - rect.bottom,
                         0,
                         0,
                         SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

            GetWindowRect(gfk->window, &rect);
            gfk->x = (int16_t)mtf.rcWork.left;
            gfk->y = (int16_t)(mtf.rcWork.bottom - (LONG)((float)gfk->npotheight * gfk->size + 0.5555));
            WriteSettings(manager);
            break;
        }
        case POPUP_MOVEWINDOW_RLC: { // Bottom right corner
            obj = &manager->objects[manager->context_id];
            gfk = &manager->gfk[obj->gfk_index];

            MONITORINFO mtf = {sizeof(MONITORINFO)};
            HMONITOR mtr = MonitorFromWindow(gfk->window, MONITOR_DEFAULTTONEAREST);
            GetMonitorInfo(mtr, &mtf);
            GetClientRect(gfk->window, &rect);

            if (manager->gfk_current == gfk->index)
                _SetSizableWindow(manager, gfk, POS_RLC);

            SetWindowPos(gfk->window, NULL,
                         mtf.rcWork.right - rect.right,
                         mtf.rcWork.bottom - rect.bottom,
                         0,
                         0,
                         SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

            GetWindowRect(gfk->window, &rect);
            gfk->x = (int16_t)(mtf.rcWork.right - (LONG)((float)gfk->npotwidth * gfk->size + 0.5555));
            gfk->y = (int16_t)(mtf.rcWork.bottom - (LONG)((float)gfk->npotheight * gfk->size + 0.5555));
            WriteSettings(manager);
            break;
        }

        default:
            break;
    }
    return 0;
}


#endif // HANDLERS_H_INCLUDED


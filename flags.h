#ifndef FLAGS_H_INCLUDED
#define FLAGS_H_INCLUDED

#include "types.h"

/** HWND flags **/

SINLINE void _SetAlignP(HWND hwnd, Object* obj, uint8_t align, float scale_dpi, int n, ...) {
    int args[4] = {0};
    RECT wrect, rect;
    POINT lefttop, rightbottom;

    va_list arg; va_start(arg, n);
    for (int i = 0; i < 4; i++) {
        args[i] = va_arg(arg, int);
    } va_end(arg);
    int p1 = (n >= 1) ? ScaleForDPI(args[0], scale_dpi) : 0;
    int p2 = (n >= 2) ? ScaleForDPI(args[1], scale_dpi) : 0;
    int p3 = (n >= 3) ? ScaleForDPI(args[2], scale_dpi) : 0;
    int p4 = (n >= 4) ? ScaleForDPI(args[3], scale_dpi) : 0;

    /// declaring button
    GetWindowRect(obj->window, &rect);
    lefttop.x = rect.left; lefttop.y = rect.top;
    rightbottom.x = rect.right; rightbottom.y = rect.bottom;
    ScreenToClient(hwnd, &lefttop);
    ScreenToClient(hwnd, &rightbottom);
    rightbottom.x -= lefttop.x; rightbottom.y -= lefttop.y;
    /// declaring hwnd
    GetClientRect(hwnd, &wrect);
    wrect.right -= wrect.left; wrect.bottom -= wrect.top;

    int ox = lefttop.x;
    int oy = lefttop.y;
    int cx = rightbottom.x;
    int cy = rightbottom.y;
    obj->align |= align;

    switch (align) {
        case (OBJ_ALIGNR | OBJ_ALIGNB):
            obj->aor = p1;
            obj->aob = p2;
            ox = wrect.right - p1 - cx;
            oy = wrect.bottom - p2 - cy;
            break;
        case OBJ_ALIGNR:
            obj->aor = p1;
            ox = wrect.right - p1 - cx;
            break;
        case OBJ_ALIGNB:
            obj->aob = p1;
            oy = wrect.bottom - p1 - cy;
            break;
        case (OBJ_ALIGNW | OBJ_ALIGNH):
            obj->aol = p1;
            obj->aot = p2;
            obj->aor = p3;
            obj->aob = p4;
            ox = p1;
            oy = p2;
            cx = wrect.right - p1 - p3;
            cy = wrect.bottom - p2 - p4;
            break;
        case OBJ_ALIGNW:
            obj->aol = p1;
            obj->aor = p2;
            ox = p1;
            cx = wrect.right - p1 - p2;
            break;
        case OBJ_ALIGNH:
            obj->aot = p1;
            obj->aob = p2;
            oy = p1;
            cy = wrect.bottom - p1 - p2;
            break;

        case (OBJ_ALIGNR | OBJ_ALIGNH):
            obj->aor = p1;
            obj->aot = p2;
            obj->aob = p3;
            ox = wrect.right - p1 - cx;
            oy = p2;
            cy = wrect.bottom - p2 - p3;
            break;
        case (OBJ_ALIGNB | OBJ_ALIGNW):
            obj->aob = p1;
            obj->aol = p2;
            obj->aor = p3;
            oy = wrect.bottom - p1 - cy;
            ox = p2;
            cx = wrect.right - p2 - p3;
            break;
        default:
            break;
    }
    MoveWindow(obj->window, ox, oy, cx, cy, TRUE);
}
#define _SetAlign(hwnd, obj, align, scale_dpi, ...) _SetAlignP(hwnd, obj, align, scale_dpi, sizeof((int[]){__VA_ARGS__}) / sizeof(int), __VA_ARGS__)

SINLINE void _SetRoundObject(Manager* manager, Object* obj, int rgnw, int rgnh) {
    obj->rgnw = ScaleForDPI(rgnw, manager->scale_dpi);
    obj->rgnh = ScaleForDPI(rgnh, manager->scale_dpi);

    RECT rect;
    GetClientRect(obj->window, &rect);
//    MapWindowPoints(HWND_DESKTOP, manager->window, (POINT*)&rect, 2);
    obj->rgn = CreateRoundRectRgn(0, 0, rect.right, rect.bottom, obj->rgnw, obj->rgnh);
    SetWindowRgn(obj->window, obj->rgn, TRUE);
}

SINLINE void _SetManagerObjectColorP(Object* obj, int j, int n, ...) {
    va_list arg; va_start(arg, n);
    for (int i = j * 4; i < 64; i++) {
        COLORREF ref = va_arg(arg, COLORREF); if (!ref) break;
        switch (i % 4) {
            case 0:
                obj->colormod[i / 4].disabled = ref;
                continue;
            case 1:
                obj->colormod[i / 4].enabled = ref;
                continue;
            case 2:
                obj->colormod[i / 4].hovered = ref;
                continue;
            case 3:
                obj->colormod[i / 4].pressed = ref;
                continue;
        }
    } va_end(arg);
}
#define _SetManagerObjectColori(obj, j, ...) _SetManagerObjectColorP(obj, j, sizeof((COLORREF[]){__VA_ARGS__}) / sizeof(COLORREF), __VA_ARGS__, 0)
#define _SetManagerObjectColor(obj, ...) _SetManagerObjectColorP(obj, 0, sizeof((COLORREF[]){__VA_ARGS__}) / sizeof(COLORREF), __VA_ARGS__, 0)

SINLINE void _SetTextFormat(Object* obj, unsigned int dt_format) {
    obj->dt_format = dt_format;
}

SINLINE void _SetTextAlign(Object* obj, int tol, int tot, int tor, int tob) {
    obj->tol = tol;
    obj->tot = tot;
    obj->tor = tor;
    obj->tob = tob;
}

SINLINE int _SetProcedurePtr(HWND hwnd, LPARAM lparam) {
    SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)((CREATESTRUCT *)lparam)->lpCreateParams);
    return 0;
}

SINLINE void _SetItemTabP(Object* obj, int n, ...) {
    ShowWindow(obj->window, SW_HIDE);
    va_list arg; va_start(arg, n);
    while (1) {
        int ref = va_arg(arg, int); if (!ref) break;
        obj->tab |= ref;
    } va_end(arg);
}
#define _SetItemTab(obj, ...) _SetItemTabP(obj, sizeof((int[]){__VA_ARGS__}) / sizeof(int), __VA_ARGS__, 0)

SINLINE void _SetScrollObjectsP(Manager* manager, int index, int n, ...) {
    RECT rect; Object* obj = &manager->objects[index];
    if (!obj->stm_count) {
        obj->stm_rect.left = INT_MAX;
        obj->stm_rect.top = INT_MAX;
        obj->stm_rect.right = INT_MIN;
        obj->stm_rect.bottom = INT_MIN;
    }

    va_list arg; va_start(arg, n);
    while (1) {
        int ref = va_arg(arg, int); if (!ref) break;

        obj->stm = realloc(obj->stm, sizeof(int) * ++obj->stm_count);
        obj->stm[obj->stm_count - 1] = ref;
        manager->objects[ref].lts = index;

        GetWindowRect(manager->objects[ref].window, &rect);
        MapWindowPoints(HWND_DESKTOP, manager->window, (POINT*)&rect, 2);
        if (obj->stm_rect.left > rect.left) {
            obj->stm_rect.left = rect.left;
            obj->stm_object_ids[0] = ref;
        }
        if (obj->stm_rect.top > rect.top) {
            obj->stm_rect.top = rect.top;
            obj->stm_object_ids[1] = ref;
        }
        if (obj->stm_rect.right < rect.right) {
            obj->stm_rect.right = rect.right;
            obj->stm_object_ids[2] = ref;
        }
        if (obj->stm_rect.bottom < rect.bottom) {
            obj->stm_rect.bottom = rect.bottom;
            obj->stm_object_ids[3] = ref;
        }
    } va_end(arg);
}
#define _SetScrollObjects(manager, current, ...) _SetScrollObjectsP(manager, current, sizeof((int[]){__VA_ARGS__}) / sizeof(int), __VA_ARGS__, 0)

SINLINE void _SetScrollObjectsM(Manager* manager, int index, int count, int* mas) {
    RECT rect; Object* obj = &manager->objects[index];

    obj->stm_rect.left = INT_MAX;
    obj->stm_rect.top = INT_MAX;
    obj->stm_rect.right = INT_MIN;
    obj->stm_rect.bottom = INT_MIN;

    int* buff = realloc(obj->stm, sizeof(int) * count);
    if (!buff) return;
    obj->stm = buff;

    for (int i = 0; i < count; i++) {
        obj->stm[i] = mas[i];
        manager->objects[mas[i]].lts = index;

        GetWindowRect(manager->objects[mas[i]].window, &rect);
        MapWindowPoints(HWND_DESKTOP, manager->window, (POINT*)&rect, 2);
        rect.left += obj->stm_offset;
        rect.top += obj->stm_offset;
        rect.right += obj->stm_offset;
        rect.bottom += obj->stm_offset;
        if (obj->stm_rect.left > rect.left) {
            obj->stm_rect.left = rect.left;
            obj->stm_object_ids[0] = mas[i];
        }
        if (obj->stm_rect.top > rect.top) {
            obj->stm_rect.top = rect.top;
            obj->stm_object_ids[1] = mas[i];
        }
        if (obj->stm_rect.right < rect.right) {
            obj->stm_rect.right = rect.right;
            obj->stm_object_ids[2] = mas[i];
        }
        if (obj->stm_rect.bottom < rect.bottom) {
            obj->stm_rect.bottom = rect.bottom;
            obj->stm_object_ids[3] = mas[i];
        }
    }
}

SINLINE void _SetScrollObject(Manager* manager, Object* obj, Object* scrollbar) {
    obj->lts = scrollbar->id;

    if (!scrollbar->stm_count) {
        scrollbar->stm_rect.left = INT_MAX;
        scrollbar->stm_rect.top = INT_MAX;
        scrollbar->stm_rect.right = INT_MIN;
        scrollbar->stm_rect.bottom = INT_MIN;
    }

    scrollbar->stm = realloc(scrollbar->stm, sizeof(unsigned int) * ++scrollbar->stm_count);
    scrollbar->stm[scrollbar->stm_count - 1] = obj->index;

    RECT rect;
    GetWindowRect(obj->window, &rect);
    rect.top -= scrollbar->stm_offset;
    MapWindowPoints(HWND_DESKTOP, manager->window, (POINT*)&rect, 2);
    SetWindowPos(obj->window, NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
    if (scrollbar->stm_rect.left > rect.left) {
        scrollbar->stm_rect.left = rect.left;
        scrollbar->stm_object_ids[0] = obj->index;
    }
    if (scrollbar->stm_rect.top > rect.top) {
        scrollbar->stm_rect.top = rect.top;
        scrollbar->stm_object_ids[1] = obj->index;
    }
    if (scrollbar->stm_rect.right < rect.right) {
        scrollbar->stm_rect.right = rect.right;
        scrollbar->stm_object_ids[2] = obj->index;
    }
    if (scrollbar->stm_rect.bottom < rect.bottom) {
        scrollbar->stm_rect.bottom = rect.bottom;
        scrollbar->stm_object_ids[3] = obj->index;
    }
    InvalidateRect(scrollbar->window, NULL, TRUE);
}

SINLINE void _SetScrollObjectByID(Manager* manager, Object* obj, int scrollid) {
    Object* scrollbar;

    for (int i = 0; i < manager->objects_count; i++) {
        if (!&manager->objects[i]) continue;

        if (manager->objects[i].id == scrollid) {
            scrollbar = &manager->objects[i];
            _SetScrollObject(manager, obj, scrollbar);
            break;
        }
    }
}

SINLINE void _SetListBoxText(Manager* manager, Object* obj) {
    RECT rect;

    GetWindowRect(manager->objects[obj->index].window, &rect);

    obj->lbsh = calloc(1, sizeof(Manager));
    memset(obj->lbsh, 0, sizeof(Manager));

    ((Manager *)obj->lbsh)->window = CreateWindowEx(
        0,
        "LBS",
        NULL,
        WS_POPUP | WS_CLIPCHILDREN,
        rect.left,
        rect.top + (rect.bottom - rect.top),
        0,
        0,
        manager->window,
        NULL,
        NULL,
        obj->lbsh
    );
    ((Manager *)obj->lbsh)->scale_dpi = manager->scale_dpi;
    ((Manager *)obj->lbsh)->parent = obj;
}

SINLINE Object* _AddListBoxText(Manager* manager, Object* obj, LPCTSTR text, BOOL enabled,
                                int fonti) {
    RECT rect;
    Manager* lbsh = (Manager *)obj->lbsh;
    GetWindowRect(obj->window, &rect);

    int oy = (int)(UnScaleForDPIf((rect.bottom - rect.top), lbsh->scale_dpi) * lbsh->lbsh_count++ + 0.555555);
    int cx = (int)(UnScaleForDPIf((rect.right - rect.left), lbsh->scale_dpi) + 0.555555);
    int cy = (int)(UnScaleForDPIf((rect.bottom - rect.top), lbsh->scale_dpi) + 0.555555);

    Object* lbshobj = _CreateManagerButton(lbsh, 0, oy, cx, cy, enabled, 0, fonti, text, FALSE);

    SetWindowPos(lbsh->window, NULL,
                 0, 0,
                 ScaleForDPI(cx, manager->scale_dpi),
                 ScaleForDPI(oy + cy, manager->scale_dpi),
                 SWP_NOMOVE | SWP_NOZORDER);

    lbshobj->flags |= OBJ_LBSOBJ;
    return lbshobj;
}

SINLINE void _SetScrollLinearCurrent(Object* obj, int current) {
    if (obj->scr_current == current) return;
    if (current > obj->scr_max && obj->flags & OBJ_TRACKBAR_OVERFLOW) obj->scr_current = obj->scr_min;
    else if (current < obj->scr_min && obj->flags & OBJ_TRACKBAR_OVERFLOW) obj->scr_current = obj->scr_max;
    else if (current >= obj->scr_min && current <= obj->scr_max) obj->scr_current = current;
    InvalidateRect(obj->window, NULL, TRUE);
}

SINLINE void _SetScrollLinearRemote(Manager* manager, Object* obj, int add, int reduce, int info) {
    for (int i = 0; i < manager->objects_count; i++) {
        if (!&manager->objects[i]) continue;

        if (manager->objects[i].id == add ||
            manager->objects[i].id == reduce) {
            manager->objects[i].scr_index = obj->index;
        }
        if (manager->objects[i].id == info) obj->scr_info = i;
    }
}

SINLINE void _SetObjectsFlags(Object* obj, uint8_t flags) {
    obj->flags = flags;
}

SINLINE void _SetParamsFrom(Manager* manager, Object* obj, int source) {
    Object* src = NULL;

    for (int i = 0; i < manager->objects_count; i++) {
        if (!&manager->objects[i]) continue;

        if (manager->objects[i].id == source)
            src = &manager->objects[i];
    }
    if (!src) return;

    HWND hwnd = obj->window;
    int index = obj->index;
    int id = obj->id;
    uint8_t flags = obj->flags;

    *obj = *src;


    obj->window = hwnd;
    obj->index = index;
    obj->id = id;
    obj->flags = flags;

    obj->aol = UnScaleForDPI(obj->aol, manager->scale_dpi);
    obj->aot = UnScaleForDPI(obj->aot, manager->scale_dpi);
    obj->aor = UnScaleForDPI(obj->aor, manager->scale_dpi);
    obj->aob = UnScaleForDPI(obj->aob, manager->scale_dpi);

    switch (obj->align) {
        case (OBJ_ALIGNR | OBJ_ALIGNB):
            _SetAlign(manager->window, obj, obj->align, manager->scale_dpi, obj->aor, obj->aob);
            break;
        case OBJ_ALIGNR:
            _SetAlign(manager->window, obj, obj->align, manager->scale_dpi, obj->aor);
            break;
        case OBJ_ALIGNB:
            _SetAlign(manager->window, obj, obj->align, manager->scale_dpi, obj->aob);
            break;
        case (OBJ_ALIGNW | OBJ_ALIGNH):
            _SetAlign(manager->window, obj, obj->align, manager->scale_dpi, obj->aol, obj->aot, obj->aor, obj->aob);
            break;
        case OBJ_ALIGNW:
            _SetAlign(manager->window, obj, obj->align, manager->scale_dpi, obj->aol, obj->aor);
            break;
        case OBJ_ALIGNH:
            _SetAlign(manager->window, obj, obj->align, manager->scale_dpi, obj->aot, obj->aob);
            break;

        case (OBJ_ALIGNR | OBJ_ALIGNH):
            _SetAlign(manager->window, obj, obj->align, manager->scale_dpi, obj->aor, obj->aot, obj->aob);
            break;
        case (OBJ_ALIGNB | OBJ_ALIGNW):
            _SetAlign(manager->window, obj, obj->align, manager->scale_dpi, obj->aob, obj->aol, obj->aor);
            break;

        default:
            break;
    }
    _SetItemTab(obj, obj->tab);

    if (src->lts) {
        _SetScrollObjectByID(manager, obj, src->lts);
    }
}

SINLINE void _SetScrollableRect(Manager* manager, Object* obj, int left, int top, int right, int bottom) {
    obj->stm_mouse_rect.left = ScaleForDPI(left, manager->scale_dpi);
    obj->stm_mouse_rect.top = ScaleForDPI(top, manager->scale_dpi);
    obj->stm_mouse_rect.right = ScaleForDPI(right, manager->scale_dpi);
    obj->stm_mouse_rect.bottom = ScaleForDPI(bottom, manager->scale_dpi);
}

#endif // FLAGS_H_INCLUDED

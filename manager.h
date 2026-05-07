#ifndef MANAGER_H_INCLUDED
#define MANAGER_H_INCLUDED

#include "types.h"
#include "fonts.h"
#include "handlers.h"

//--------------------------------------------------
// MEMORY SAFE (MAYBE)
//--------------------------------------------------

//--------------------------------------------------
// WNDCLASSEX
//--------------------------------------------------

SINLINE uint8_t _RegisterManagerWcex(WNDCLASSEX* wcex, HINSTANCE instance, LPCTSTR name,
                                     WNDPROC Procedure, UINT style, COLORREF color, LPCSTR idc) {
    wcex->hInstance = instance;
    wcex->lpszClassName = name;
    wcex->lpfnWndProc = Procedure;
    wcex->style = style;
    wcex->cbSize = sizeof(WNDCLASSEX);
    wcex->hIcon = (HICON)LoadImage(GetModuleHandle(NULL),
                                   MAKEINTRESOURCE(RESOURCE_ICON),
                                   IMAGE_ICON, 64, 64,
                                   LR_DEFAULTCOLOR);
    wcex->hIconSm = (HICON)LoadImage(GetModuleHandle(NULL),
                                     MAKEINTRESOURCE(RESOURCE_ICON),
                                     IMAGE_ICON, 64, 64,
                                     LR_DEFAULTCOLOR);
    wcex->hCursor = LoadCursor(NULL, idc);
    wcex->lpszMenuName = NULL;
    wcex->cbClsExtra = 0;
    wcex->cbWndExtra = 0;
    wcex->hbrBackground = (HBRUSH)CreateSolidBrush(color);

    if (!RegisterClassEx(wcex))
        return 0;

    return 1;
}

//--------------------------------------------------
// WINDOW (HWND)
//--------------------------------------------------

SINLINE uint8_t _CreateManagerWindow(Manager* manager, HWND* hwnd,
                                     LPCTSTR name, LPCTSTR cls,
                                     int ox, int oy, int cx, int cy,
                                     HINSTANCE instance, int show) {
    (*hwnd) = CreateWindowEx(
        WS_EX_COMPOSITED,
        cls,
        name,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        ox, oy, cx, cy,
        HWND_DESKTOP,
        NULL,
        instance,
        (void *)manager
    );
    if (!(*hwnd)) return 0;

    manager->tab = MGR_MMWnF;
    manager->width_buff = cx;
    manager->height_buff = cy;
    return 1;
}

SINLINE uint8_t _SetNID(Manager* manager) {
    ZeroMemory(&manager->nid, sizeof(NOTIFYICONDATA));
    manager->nid.cbSize            = sizeof(NOTIFYICONDATA);
    manager->nid.uID               = ID_TRAY;
    manager->nid.hWnd              = manager->window_tray;
    manager->nid.hIcon             = (HICON)LoadImage(GetModuleHandle(NULL),
                                                      MAKEINTRESOURCE(RESOURCE_ICON),
                                                      IMAGE_ICON, 64, 64,
                                                      LR_DEFAULTCOLOR);
    manager->nid.uFlags            = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    manager->nid.uCallbackMessage  = WM_TRAYNOTIFY;
    strcpy(manager->nid.szTip,       APP_NAME);

    return Shell_NotifyIcon(NIM_ADD, &manager->nid);
}

//--------------------------------------------------
// OBJECTS
//--------------------------------------------------

SINLINE Object* _CreateManagerRect(Manager* manager, int ox, int oy, int cx, int cy, BOOL enabled, int id) {
    Object* buff = realloc(manager->objects, sizeof(Object) * (manager->objects_count + 1));
    if (!buff) return NULL;
    manager->objects = buff;

    Object* obj = &manager->objects[manager->objects_count];
    memset(obj, 0, sizeof(Object));
    obj->window = CreateWindow(
        "BUTTON",
        "",
        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | ((enabled) ? 0 : WS_DISABLED) | WS_CLIPSIBLINGS,
        ScaleForDPI(ox, manager->scale_dpi),
        ScaleForDPI(oy, manager->scale_dpi),
        ScaleForDPI(cx, manager->scale_dpi),
        ScaleForDPI(cy, manager->scale_dpi),
        manager->window,
        (HMENU)(manager->objects_count + 1),
        (HINSTANCE)GetWindowLongPtr(manager->window, GWLP_HINSTANCE),
        NULL
    );

    if (!obj->window) {
        if (manager->objects_count == 0) {
            free(manager->objects); manager->objects = NULL;
        }
        else {
            buff = realloc(manager->objects, sizeof(Object) * manager->objects_count);
            if (buff) manager->objects = buff;
        }
        return NULL;
    }

    obj->type = OBJ_RECT;
    obj->index = manager->objects_count;
    obj->id = id;
    obj->gfk_index = -1;
    obj->parent = (void *)manager;

    manager->objects_count++;
    _SetManagerObjectColor(obj, MGR_COLOR_RECT_DEFAULT);
    return obj;
}

SINLINE Object* _CreateManagerStatic(Manager* manager, int ox, int oy, int cx, int cy, BOOL enabled, int id,
                                     int fonti, LPCTSTR text) {
    Object* buff = realloc(manager->objects, sizeof(Object) * (manager->objects_count + 1));
    if (!buff) return NULL;
    manager->objects = buff;

    Object* obj = &manager->objects[manager->objects_count];
    memset(obj, 0, sizeof(Object));
    obj->window = CreateWindow(
        "BUTTON",
        text,
        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | BS_FLAT | ((enabled) ? 0 : WS_DISABLED) | WS_CLIPSIBLINGS,
        ScaleForDPI(ox, manager->scale_dpi),
        ScaleForDPI(oy, manager->scale_dpi),
        ScaleForDPI(cx, manager->scale_dpi),
        ScaleForDPI(cy, manager->scale_dpi),
        manager->window,
        (HMENU)(manager->objects_count + 1),
        (HINSTANCE)GetWindowLongPtr(manager->window, GWLP_HINSTANCE),
        NULL
    );

    if (!obj->window) {
        if (manager->objects_count == 0) {
            free(manager->objects); manager->objects = NULL;
        }
        else {
            buff = realloc(manager->objects, sizeof(Object) * manager->objects_count);
            if (buff) manager->objects = buff;
        }
        return NULL;
    }

    obj->type = OBJ_STATIC;
    obj->index = manager->objects_count;
    obj->id = id;
    obj->gfk_index = -1;
    obj->parent = (void *)manager;

    manager->objects_count++;
    if (obj->window && fonti)
        SendMessage(obj->window, WM_SETFONT, (WPARAM)manager->fonts[fonti], TRUE);

    _SetManagerObjectColor(obj, MGR_COLOR_STATIC_BACKGROUND_DEFAULT, MGR_COLOR_STATIC_TEXT_DEFAULT);
    _SetTextFormat(obj, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    return obj;
}

//SINLINE int DEFBTNBTNUPDEL(void* args)
//SINLINE int DEFBTNBTNDWNDEL(void* args)

Object* _CreateManagerButton(Manager* manager, int ox, int oy, int cx, int cy, BOOL enabled, int id,
                             int fonti, LPCTSTR text, BOOL repeat) {
    Object* buff = realloc(manager->objects, sizeof(Object) * (manager->objects_count + 1));
    if (!buff) return NULL;
    manager->objects = buff;

    Object* obj = &manager->objects[manager->objects_count];
    memset(obj, 0, sizeof(Object));
    obj->window = CreateWindow(
        "BUTTON",
        text,
        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | ((enabled) ? 0 : WS_DISABLED) | WS_CLIPSIBLINGS,
        ScaleForDPI(ox, manager->scale_dpi),
        ScaleForDPI(oy, manager->scale_dpi),
        ScaleForDPI(cx, manager->scale_dpi),
        ScaleForDPI(cy, manager->scale_dpi),
        manager->window,
        (HMENU)(manager->objects_count + 1),
        (HINSTANCE)GetWindowLongPtr(manager->window, GWLP_HINSTANCE),
        NULL
    );

    if (!obj->window) {
        if (manager->objects_count == 0) {
            free(manager->objects); manager->objects = NULL;
        }
        else {
            buff = realloc(manager->objects, sizeof(Object) * manager->objects_count);
            if (buff) manager->objects = buff;
        }
        return NULL;
    }

    obj->type = OBJ_BUTTON;
    obj->index = manager->objects_count;
    obj->btnup = DEFBTNBTNUPDEL;
    obj->btndown = DEFBTNBTNDWNDEL;
    obj->id = id;
    obj->gfk_index = -1;
    obj->parent = (void *)manager;

    manager->objects_count++;
    if (repeat) obj->flags |= OBJ_REPEAT;
    if (obj->window && fonti) {
        SendMessage(obj->window, WM_SETFONT, (WPARAM)manager->fonts[fonti], (LPARAM)TRUE);
    }

    _SetManagerObjectColor(obj, MGR_COLOR_BUTTON_BACKGROUND_DEFAULT, MGR_COLOR_BUTTON_TEXT_DEFAULT);
    _SetTextFormat(obj, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    return obj;
}

// Default Switch Delegate On Button Up
SINLINE int DEFSWHBTNUPDEL(void* args) { return 0; }
// Default Switch Delegate On Button Down
SINLINE int DEFSWHBTNDWNDEL(void* args) {
    Object* obj = (Object *)args;

    if (obj->flags & OBJ_BOOL) {
        obj->flags &= ~OBJ_BOOL;
        _SetManagerObjectColori(obj, 1, MGR_COLOR_SWITCH_SLIDER_DEFAULT);
    }
    else {
        obj->flags |= OBJ_BOOL;
        _SetManagerObjectColori(obj, 1, MGR_COLOR_SWITCH_SLIDERT_DEFAULT);
    }

    InvalidateRect(obj->window, NULL, TRUE);
    return 0;
}

SINLINE Object* _CreateManagerSwitch(Manager* manager, int ox, int oy, int swsize, BOOL enabled, int id) {
    Object* buff = realloc(manager->objects, sizeof(Object) * (manager->objects_count + 1));
    if (!buff) return NULL;
    manager->objects = buff;

    Object* obj = &manager->objects[manager->objects_count];
    memset(obj, 0, sizeof(Object));
    obj->window = CreateWindow(
        "BUTTON",
        "",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW | ((enabled) ? 0 : WS_DISABLED) | WS_CLIPSIBLINGS,
        ScaleForDPI(ox, manager->scale_dpi),
        ScaleForDPI(oy, manager->scale_dpi),
        ScaleForDPI(swsize, manager->scale_dpi),
        ScaleForDPI(swsize / 2, manager->scale_dpi),
        manager->window,
        (HMENU)(manager->objects_count + 1),
        (HINSTANCE)GetWindowLongPtr(manager->window, GWLP_HINSTANCE),
        NULL
    );

    if (!obj->window) {
        if (manager->objects_count == 0) {
            free(manager->objects); manager->objects = NULL;
        }
        else {
            buff = realloc(manager->objects, sizeof(Object) * manager->objects_count);
            if (buff) manager->objects = buff;
        }
        return NULL;
    }

    obj->type = OBJ_SWITCH;
    obj->swsize = ScaleForDPI(swsize, manager->scale_dpi);
    obj->index = manager->objects_count;
    obj->btnup = DEFSWHBTNUPDEL;
    obj->btndown = DEFSWHBTNDWNDEL;
    obj->id = id;
    obj->gfk_index = -1;
    obj->parent = (void *)manager;

    manager->objects_count++;
    _SetManagerObjectColor(obj, MGR_COLOR_SWITCH_BACKGROUND_DEFAULT,
                                MGR_COLOR_SWITCH_SLIDER_DEFAULT,
                                MGR_COLOR_SWITCH_DOT_DEFAULT);
    return obj;
}

//SINLINE int DEFSCRBTNUPDEL(void* args)
//SINLINE int DEFSCRBTNDWNDEL(void* args)

SINLINE Object* _CreateManagerScrollBar(Manager* manager, int ox, int oy, int cx, int cy, BOOL enabled, int id) {
    Object* buff = realloc(manager->objects, sizeof(Object) * (manager->objects_count + 1));
    if (!buff) return NULL;
    manager->objects = buff;

    Object* obj = &manager->objects[manager->objects_count];
    memset(obj, 0, sizeof(Object));
    obj->window = CreateWindow(
        "BUTTON",
        "",
        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | ((enabled) ? 0 : WS_DISABLED),
        ScaleForDPI(ox, manager->scale_dpi),
        ScaleForDPI(oy, manager->scale_dpi),
        ScaleForDPI(cx, manager->scale_dpi),
        ScaleForDPI(cy, manager->scale_dpi),
        manager->window,
        (HMENU)(manager->objects_count + 1),
        (HINSTANCE)GetWindowLongPtr(manager->window, GWLP_HINSTANCE),
        NULL
    );

    if (!obj->window) {
        if (manager->objects_count == 0) {
            free(manager->objects); manager->objects = NULL;
        }
        else {
            buff = realloc(manager->objects, sizeof(Object) * manager->objects_count);
            if (buff) manager->objects = buff;
        }
        return NULL;
    }

    obj->type = OBJ_SCROLLBAR;
    obj->index = manager->objects_count;
    obj->btnup = DEFSCRBTNUPDEL;
    obj->btndown = DEFSCRBTNDWNDEL;
    obj->id = id;
    obj->gfk_index = -1;
    obj->parent = (void *)manager;

    manager->objects_count++;
    _SetManagerObjectColor(obj, MGR_COLOR_SCROLLBAR_BACKGROUND_DEFAULT,
                                MGR_COLOR_SCROLLBAR_SLIDER_DEFAULT);
    _SetRoundObject(manager, obj, cx / 2, 2);

    return obj;
}

// Default ListBox Delegate On Button Up
SINLINE int DEFLTBBTNUPDEL(void* args) { return 0; }
// Default ListBox Delegate On Button Down
SINLINE int DEFLTBBTNDWNDEL(void* args) {
    RECT orect, lbshrect;
    Object* obj = (Object *)args;
    Manager* lbsh = (Manager *)obj->lbsh;

    GetWindowRect(obj->window, &orect);
    GetWindowRect(lbsh->window, &lbshrect);

    if (IsWindowVisible(lbsh->window)) {
        ShowWindow(lbsh->window, SW_HIDE);
    }
    else {
        if (obj->flags & OBJ_LBSDS) {
            obj->flags &= ~OBJ_LBSDS;
        }
        else {
            ShowWindow(lbsh->window, SW_SHOW);
            SetWindowPos(lbsh->window, NULL,
                         orect.left,
                         orect.top + (orect.bottom - orect.top),
                         lbshrect.right - lbshrect.left,
                         lbshrect.bottom - lbshrect.top,
                         SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
            obj->flags &= ~OBJ_LBSDS;
        }
    }

    return 0;
}

SINLINE Object* _CreateManagerListBox(Manager* manager, int ox, int oy, int cx, int cy, BOOL enabled, int id,
                                      int fonti, int marletti, LPCTSTR text) {
    Object* buff = realloc(manager->objects, sizeof(Object) * (manager->objects_count + 1));
    if (!buff) return NULL;
    manager->objects = buff;

    Object* obj = &manager->objects[manager->objects_count];
    memset(obj, 0, sizeof(Object));
    obj->window = CreateWindow(
        "BUTTON",
        text,
        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | ((enabled) ? 0 : WS_DISABLED) | WS_CLIPSIBLINGS,
        ScaleForDPI(ox, manager->scale_dpi),
        ScaleForDPI(oy, manager->scale_dpi),
        ScaleForDPI(cx, manager->scale_dpi),
        ScaleForDPI(cy, manager->scale_dpi),
        manager->window,
        (HMENU)(manager->objects_count + 1),
        (HINSTANCE)GetWindowLongPtr(manager->window, GWLP_HINSTANCE),
        NULL
    );

    if (!obj->window) {
        if (manager->objects_count == 0) {
            free(manager->objects); manager->objects = NULL;
        }
        else {
            buff = realloc(manager->objects, sizeof(Object) * manager->objects_count);
            if (buff) manager->objects = buff;
        }
        return NULL;
    }

    obj->type = OBJ_LISTBOX;
    obj->index = manager->objects_count + 1;
    obj->btnup = DEFLTBBTNUPDEL;
    obj->btndown = DEFLTBBTNDWNDEL;
    obj->id = id;
    obj->gfk_index = -1;
    obj->parent = (void *)manager;

    manager->objects_count++;
    if (obj->window && fonti)
        SendMessage(obj->window, WM_SETFONT, (WPARAM)manager->fonts[fonti], TRUE);

    if (marletti) obj->marlett = manager->fonts[marletti];

    _SetManagerObjectColor(obj, MGR_COLOR_LISTBOX_BACKGROUND_DEFAULT,
                                MGR_COLOR_LISTBOX_ARROW_DEFAULT,
                                MGR_COLOR_LISTBOX_TEXT_DEFAULT);

    _SetTextFormat(obj, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    return obj;
}

//SINLINE int DEFSCLBTNUPDEL(void* args)
//SINLINE int DEFSCLBTNDWNDEL(void* args)

SINLINE Object* _CreateManagerScrollLinear(Manager* manager, int ox, int oy, int cx, int cy, BOOL enabled, int id,
                                           int current, int min, int max) {
    Object* buff = realloc(manager->objects, sizeof(Object) * (manager->objects_count + 1));
    if (!buff) return NULL;
    manager->objects = buff;

    Object* obj = &manager->objects[manager->objects_count];
    memset(obj, 0, sizeof(Object));
    obj->window = CreateWindow(
        "BUTTON",
        "",
        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | ((enabled) ? 0 : WS_DISABLED),
        ScaleForDPI(ox, manager->scale_dpi),
        ScaleForDPI(oy, manager->scale_dpi),
        ScaleForDPI(cx, manager->scale_dpi),
        ScaleForDPI(cy, manager->scale_dpi),
        manager->window,
        (HMENU)(manager->objects_count + 1),
        (HINSTANCE)GetWindowLongPtr(manager->window, GWLP_HINSTANCE),
        NULL
    );

    if (!obj->window) {
        if (manager->objects_count == 0) {
            free(manager->objects); manager->objects = NULL;
        }
        else {
            buff = realloc(manager->objects, sizeof(Object) * manager->objects_count);
            if (buff) manager->objects = buff;
        }
        return NULL;
    }

    obj->type = OBJ_SCRLINEAR;
    obj->index = manager->objects_count;
    obj->btnup = DEFSCLBTNUPDEL;
    obj->btndown = DEFSCLBTNDWNDEL;
    obj->id = id;
    obj->scr_current = current;
    obj->scr_min = min;
    obj->scr_max = max;
    obj->gfk_index = -1;
    obj->parent = (void *)manager;

    manager->objects_count++;

    _SetManagerObjectColor(obj, MGR_COLOR_SCRLINEAR_BACKGROUND_DEFAULT,
                                MGR_COLOR_SCRLINEAR_LINE_DEFAULT,
                                MGR_COLOR_SCRLINEAR_ARROW_DEFAULT);

    return obj;
}

void _DeleteObject(Object* obj) {
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

//--------------------------------------------------
// GIFDESK
//--------------------------------------------------

LRESULT CALLBACK _GIFDeskProcedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    GIFDesk* gfk;
    Manager* manager;
    Object* obj;
    POINT pnt;
    static POINT pnt_buff;
    RECT rect;
    HCURSOR cur;
    HMENU menu, submenu;

    switch (message) {
        case WM_CREATE:
            return _SetProcedurePtr(hwnd, lparam);

        case WM_CLOSE:
            gfk = (GIFDesk *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            manager = (Manager *)gfk->mgr;
            manager->context_id = gfk->obj_index;
            SendMessage(manager->window, WM_COMMAND, POPUP_DELETEFILE, 0);
            return 0;

        case WM_MOUSEMOVE:
            gfk = (GIFDesk *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

            GetCursorPos(&pnt);
            if (GetKeyState(VK_LBUTTON) & 0x8000 &&
               (pnt.x - pnt_buff.x > 2 || pnt.x - pnt_buff.x < -2 ||
                pnt.y - pnt_buff.y > 2 || pnt.y - pnt_buff.y < -2) &&
                !(gfk->flags & SETTINGS_DM))
                SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            pnt_buff = pnt;
            return 0;
        case WM_SETCURSOR:
            gfk = (GIFDesk *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (gfk->flags & SETTINGS_HH) {
                SetTimer(hwnd, 2, 1, NULL);
            }

            if (gfk->flags & SETTINGS_DM) cur = LoadCursor(NULL, IDC_ARROW);
            else cur = LoadCursor(NULL, IDC_SIZEALL);
            SetCursor(cur);
            return 1;
        case WM_LBUTTONDOWN:
            SetTimer(hwnd, 1, 50, NULL);
            return 0;
        case WM_LBUTTONUP:
            gfk = (GIFDesk *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            manager = (Manager *)gfk->mgr;
            obj = NULL;

            for (int i = 0; i < manager->objects_count; i++) {
                if (manager->objects[i].gfk_index == gfk->index &&
                    manager->objects[i].id == MANAGER_BTN_FILE) {
                    obj = &manager->objects[i];

                    break;
                }
            }

            for (uint16_t i = 0; i < manager->gfk_count; i++) {
                if (!&manager->gfk[i]) continue;
                SetWindowPos(manager->gfk[i].window,
                             (manager->gfk[i].flags & SETTINGS_AOT) ? HWND_TOPMOST : HWND_NOTOPMOST,
                             0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            }

            if (obj != NULL) {
                SendMessage(manager->window, WM_COMMAND, obj->index, (LPARAM)obj->window);
                SetFocus(obj->window);
            }
            return 0;
        case WM_RBUTTONDOWN:
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
            return 0;
        case WM_EXITSIZEMOVE:
            gfk = (GIFDesk *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            manager = (Manager *)gfk->mgr;

            if (manager->gfk_current == gfk->index)
                _SetSizableWindow(manager, gfk, POS_NULL);
            else WriteSettings(manager);
            return DefWindowProc(hwnd, message, wparam, lparam);
        case WM_TIMER:
            gfk = (GIFDesk *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            switch (wparam) {
                case 1:
                    if (!(gfk->flags & SETTINGS_DM))
                        SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

                    KillTimer(hwnd, 1);
                    break;
                case 2: // SetTimer
                    gfk = (GIFDesk *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
                    GetCursorPos(&pnt); GetWindowRect(hwnd, &rect);

                    switch (gfk->pos) {
                        case POS_LTC:
                            rect.right = rect.left + (LONG)((float)gfk->npotwidth * gfk->size + 0.5555);
                            rect.bottom = rect.top + (LONG)((float)gfk->npotheight * gfk->size + 0.5555);
                            break;
                        case POS_LLC:
                            rect.right = rect.left + (LONG)((float)gfk->npotwidth * gfk->size + 0.5555);
                            rect.top = rect.bottom - (LONG)((float)gfk->npotheight * gfk->size + 0.5555);
                            break;
                        case POS_RTC:
                            rect.left = rect.right - (LONG)((float)gfk->npotwidth * gfk->size + 0.5555);
                            rect.bottom = rect.top + (LONG)((float)gfk->npotheight * gfk->size + 0.5555);
                            break;
                        case POS_RLC:
                            rect.left = rect.right - (LONG)((float)gfk->npotwidth * gfk->size + 0.5555);
                            rect.top = rect.bottom - (LONG)((float)gfk->npotheight * gfk->size + 0.5555);
                            break;
                        case POS_C:
                            rect.left = rect.left + (LONG)((float)gfk->npotwidth * 2 - (float)gfk->npotwidth * gfk->size + 0.5555) / 2;
                            rect.top = rect.top + (LONG)((float)gfk->npotheight * 2 - (float)gfk->npotheight * gfk->size + 0.5555) / 2;
                            rect.right = rect.right - (LONG)((float)gfk->npotwidth * 2 - (float)gfk->npotwidth * gfk->size + 0.5555) / 2;
                            rect.bottom = rect.bottom - (LONG)((float)gfk->npotheight * 2 - (float)gfk->npotheight * gfk->size + 0.5555) / 2;
                            break;
                        default:
                            break;
                    }

                    if (PtInRect(&rect, pnt) && (gfk->flags & SETTINGS_HH)) SetLayeredWindowAttributes(hwnd, 0x0, 1, LWA_ALPHA);
                    else if (gfk->flags & SETTINGS_HH) SetLayeredWindowAttributes(hwnd, 0x0, 0, LWA_COLORKEY);

                    if (!(gfk->flags & SETTINGS_CT))
                        KillTimer(hwnd, 2);
                    break;
                case 3: // Reserved
                    KillTimer(hwnd, 3);
                    break;
            }
            return 0;
        case WM_COMMAND:
            gfk = (GIFDesk *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            manager = (Manager *)gfk->mgr;
            obj = &manager->objects[gfk->obj_index];

            manager->context_id = (int)obj->index;
            SendMessage(manager->window, WM_COMMAND, wparam, (LPARAM)manager->window);
            return 0;
        case WM_MOVE:
            gfk = (GIFDesk *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            gfk->x = LOWORD(lparam);
            gfk->y = HIWORD(lparam);
            return DefWindowProc(hwnd, message, wparam, lparam);
        case WM_KEYDOWN:
            switch (wparam) {
                case VK_DELETE:
                    gfk = (GIFDesk *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
                    manager = (Manager *)gfk->mgr;
                    manager->context_id = gfk->obj_index;
                    SendMessage(manager->window, WM_COMMAND, POPUP_DELETEFILE, 0);
                    return 0;
                case VK_SPACE:
                    gfk = (GIFDesk *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
                    gfk->start_time = GetTime() - (double)gfk->lengths[gfk->render_frame] / ((double)gfk->speed * 0.05);
                    gfk->change_frames ^= 1;
                    return 0;
                default:
                    return 0;
            }
        default:
            return DefWindowProc(hwnd, message, wparam, lparam);
    }
}

LRESULT WINAPI _ManagerProcedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    static Manager* manager;
//    static RECT rect;

    switch (message) {
        case WM_CREATE:
            SetTimer(hwnd, 3, 50, NULL);
            return _SetProcedurePtr(hwnd, lparam);

        case WM_CLOSE:
            manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (manager->flags & 0x1) {
                ShowWindow(manager->window, SW_HIDE);
            }
            else {
                _SetSizableWindow(manager, NULL, POS_NULL);
                ManagerDestroy(manager);
                PostQuitMessage(0);
            }
            return 0;

        case WM_GETMINMAXINFO:
            manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            return _SetMinimalWindowSize(lparam, (manager) ? manager->scale_dpi : 1.0f);

        case WM_PAINT:
            return _InvalidateManagerBkg(hwnd);
        case WM_DRAWITEM:
            return _InvalidateManagerItem(hwnd, wparam, lparam);
        case WM_CTLCOLORBTN:
            return (LRESULT)GetStockObject(NULL_BRUSH);

        case WM_SETCURSOR:
            return _HandleManagerCursor(hwnd, wparam, lparam);
        case WM_MOUSEWHEEL:
            return _HandleManagerWheel(hwnd, wparam, lparam);
        case WM_MOUSEMOVE:
            return _HandleManagerMouseMove(hwnd, wparam, lparam);
        case WM_SCRLNR:
            return _HandleSCRLNR(hwnd, wparam, lparam);

        case WM_SIZE:
            return _InvalidateResizedItems(hwnd, wparam, lparam);

        case WM_LBUTTONUP:
            return _HandleButtonUp(hwnd, wparam, lparam);

        case WM_COMMAND:
            return _HandleManagerCommand(hwnd, wparam, lparam);

        case WM_TIMER:
            return _HandleManagerTimer(hwnd, wparam, lparam);

        case WM_CONTEXTMENU:
            return _HandleContextMenu(hwnd, wparam, lparam);

        case WM_DROPFILES:
            return _HandleDropFiles(hwnd, wparam, lparam);

        case WM_KEYDOWN:
            return _HandleKeyDown(hwnd, wparam, lparam);

        default:
            return DefWindowProc(hwnd, message, wparam, lparam);
    }
}

LRESULT WINAPI _ManagerProcedureTray(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    static Manager* manager;

    switch (message) {
        case WM_CREATE:
            return _SetProcedurePtr(hwnd, lparam);
        case WM_TRAYNOTIFY:
            return _HandleNotify(hwnd, wparam, lparam);
        case WM_COMMAND:
            manager = (Manager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            switch (LOWORD(wparam)) {
                case POPUP_OPENGIFDESK:
                    ShowWindow(manager->window, SW_SHOW);
                    break;
                case POPUP_QUIT:
                    _SetSizableWindow(manager, NULL, POS_NULL);
                    ManagerDestroy(manager);
                    PostQuitMessage(0);
                    break;
            }
            return 0;
        default:
            return DefWindowProc(hwnd, message, wparam, lparam);
    }
}

LRESULT WINAPI _LBSProcedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_DESTROY:
        case WM_NCDESTROY:
            return 0;

        case WM_ACTIVATE:
            return _HandleLBSActivate(hwnd, message, wparam, lparam);

        default:
            return _ManagerProcedure(hwnd, message, wparam, lparam);
    }
}

SINLINE Manager* ManagerCreate(LPCTSTR name, int argc, char* argv[]) {
    SetDPIContext();

    Manager* manager = (Manager *)calloc(1, sizeof(Manager));
    if (!manager) {
        ManagerHandleError(manager);
    }

    Object* obj = NULL;
    manager->casb = -1;
    manager->gfk_current = -1;
    manager->context_id = -1;
    manager->glmutex = CreateMutex(NULL, FALSE, NULL);
    if (!manager->glmutex) {
         manager->error = MANAGER_ERR_CREATE_MUTEX;
         ManagerHandleError(manager);
    }

    if (!_RegisterManagerWcex(&manager->wcex, NULL, name,
                              _ManagerProcedure, CS_HREDRAW | CS_VREDRAW,
                              MGR_COLOR_BKG, IDC_ARROW)) {
        manager->error = MANAGER_ERR_CREATE_WCEX;
        ManagerHandleError(manager);
    }

    if (!_RegisterManagerWcex(&manager->wcextray, GetModuleHandle(NULL), "GIFDeskTray",
                              _ManagerProcedureTray, 0,
                              MGR_COLOR_BKG, IDC_ARROW)) {
        manager->error = MANAGER_ERR_CREATE_WCEXTRAY;
        ManagerHandleError(manager);
    }

    if (!_RegisterManagerWcex(&manager->lbswcex, (HINSTANCE)GetWindowLongPtr(manager->window, GWLP_HINSTANCE), "LBS",
                              _LBSProcedure, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
                              MGR_COLOR_BKG, IDC_ARROW)) {
        manager->error = MANAGER_ERR_CREATE_LBSWCEX;
        ManagerHandleError(manager);
    }

    if (!_RegisterManagerWcex(&manager->gfkwcex, (HINSTANCE)HWND_DESKTOP, "GIFDesk",
                              _GIFDeskProcedure, CS_OWNDC, RGB(0, 0, 0), IDC_SIZEALL)) {
        manager->error = MANAGER_ERR_CREATE_GFKWCEX;
        ManagerHandleError(manager);
    }

    ///

    if (!_CreateManagerWindow(manager, &manager->window,
                              name, name,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              1, 1, NULL, (int)NULL)) {
        manager->error = MANAGER_ERR_CREATE_WINDOW;
        ManagerHandleError(manager);
    }

    manager->window_tray = CreateWindowEx(
        0,
        "GIFDeskTray",
        "GIFDeskTray",
        WS_POPUP,
        0, 0,
        1, 1,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        (void*)manager
    );

    if (!_SetNID(manager)) {
        manager->error = MANAGER_ERR_CREATE_TRAY;
        ManagerHandleError(manager);
    }

    manager->dpi = GetDPI(manager->window);
    manager->scale_dpi = manager->dpi / (float)DPI_DEFAULT;

    if (!_AddManagerFont(manager, FONT_NULL)) {
        manager->error = MANAGER_ERR_CREATE_FONT;
        ManagerHandleError(manager);
    }

    if (!_AddManagerFont(manager, FONT_MAIN)) {
        manager->error = MANAGER_ERR_CREATE_FONT;
        ManagerHandleError(manager);
    }

    if (!_AddManagerFont(manager, FONT_CORBEL)) {
        manager->error = MANAGER_ERR_CREATE_FONT;
        ManagerHandleError(manager);
    }

    if (!_AddManagerFont(manager, FONT_LABEL_NAME)) {
        manager->error = MANAGER_ERR_CREATE_FONT;
        ManagerHandleError(manager);
    }

    if (!_AddManagerFont(manager, FONT_LABEL)) {
        manager->error = MANAGER_ERR_CREATE_FONT;
        ManagerHandleError(manager);
    }

    if (!_AddManagerFont(manager, FONT_DESCRIPTION)) {
        manager->error = MANAGER_ERR_CREATE_FONT;
        ManagerHandleError(manager);
    }

    if (!_AddManagerFont(manager, FONT_BTN)) {
        manager->error = MANAGER_ERR_CREATE_FONT;
        ManagerHandleError(manager);
    }

    if (!_AddManagerFont(manager, FONT_MARLETT)) {
        manager->error = MANAGER_ERR_CREATE_FONT;
        ManagerHandleError(manager);
    }

    if (!_AddManagerFont(manager, FONT_ARIAL)) {
        manager->error = MANAGER_ERR_CREATE_FONT;
        ManagerHandleError(manager);
    }

    if (!_AddManagerFont(manager, FONT_FRAMES)) {
        manager->error = MANAGER_ERR_CREATE_FONT;
        ManagerHandleError(manager);
    }

    ///

    /** MGR_MMWnF **/

    {
        obj = _CreateManagerStatic(manager, 0, 25, 0, 20, TRUE, 0,
                                   FONT_MAIN_ID, "Seems like there`s nothing...");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetItemTab(obj, MGR_MMWnF);
        _SetAlign(manager->window, obj, OBJ_ALIGNWH, manager->scale_dpi, 0, 21, 0, 0);
        _SetManagerObjectColori(obj, 1, MGR_COLOR_STATIC_TEXT_BRIGHT);
    }

    /** MGR_MMWF **/
    {
        // Buttons
        obj = _CreateManagerButton(manager, 0, 0, 50, 20, TRUE, MANAGER_BTN_NEWFILE,
                                   FONT_MAIN_ID, "File", FALSE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetItemTab(obj, MGR_MMWnF | MGR_MMWF | MGR_MMSF);

        obj = _CreateManagerButton(manager, 50, 0, 69, 20, TRUE, MANAGER_BTN_SETTINGS,
                                   FONT_MAIN_ID, "Settings", FALSE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetItemTab(obj, MGR_MMWnF | MGR_MMWF | MGR_MMSF);

        // Static
        obj = _CreateManagerStatic(manager, 0, 0, 0, 0, TRUE, 0,
                                   FONT_MAIN_ID, "Select file to configure it");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetItemTab(obj, MGR_MMWF);
        _SetAlign(manager->window, obj, OBJ_ALIGNWH, manager->scale_dpi, 231, 21, 0, 0);
        _SetManagerObjectColori(obj, 1, MGR_COLOR_STATIC_TEXT_BRIGHT);

        // Main line
        obj = _CreateManagerRect(manager, 0, 20, 1, 1, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetItemTab(obj, MGR_MMWnF | MGR_MMWF | MGR_MMSF);
        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 0);

        // Rect fills menu right after main buttons
        obj = _CreateManagerRect(manager, 0, 0, 1, 20, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetItemTab(obj, MGR_MMWnF | MGR_MMWF | MGR_MMSF);
        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 70, 0);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_RECT_BKG);

        // Line between file and configuring
        obj = _CreateManagerRect(manager, 230, 0, 1, 1, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNH, manager->scale_dpi, 20, 0);
        _SetItemTab(obj, MGR_MMWF | MGR_MMSF);
    }

    /** MGR_MMSF **/

    {
        int xdiv = 30, x = xdiv, cdiv = xdiv;

        obj = _CreateManagerScrollBar(manager, 0, 0, 5, 0, TRUE, MANAGER_SCROLL_CONFIG);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNRH, manager->scale_dpi, 3, xdiv - 5, 3);
        _SetItemTab(obj, MGR_MMSF);
        _SetScrollableRect(manager, obj, 231, 21, SHRT_MAX, SHRT_MAX);

        obj = _CreateManagerScrollBar(manager, 222, 0, 5, 0, TRUE, MANAGER_SCROLL_FILES);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNH, manager->scale_dpi, xdiv - 5, 3);
        _SetItemTab(obj, MGR_MMSF | MGR_MMWF);
        _SetScrollableRect(manager, obj, 0, 21, 230, SHRT_MAX);

        /// -----------------------------------------------------------------------------

        obj = _CreateManagerStatic(manager, 257, x += 12, 89, 16, TRUE, MANAGER_STATIC_FRAMES,
                                   FONT_FRAMES_ID, "Frame: N/M");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetManagerObjectColori(obj, 0, MGR_COLOR_STATIC_DIV);
        _SetTextFormat(obj, DT_LEFT | DT_SINGLELINE);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_CONFIG);
        _SetItemTab(obj, MGR_MMSF);

        obj = _CreateManagerButton(manager, 257, x += 21, 30, 16, TRUE, MANAGER_BTN_MFRAME,
                                   FONT_LABEL_NAME_ID, "-", TRUE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetRoundObject(manager, obj, 5, 5);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_BUTTON_BACKGROUND_DIV);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_CONFIG);
        _SetItemTab(obj, MGR_MMSF);

        obj = _CreateManagerButton(manager, 292, x, 30, 16, TRUE, MANAGER_BTN_FRAMEPAUSE,
                                   FONT_LABEL_NAME_ID, "P", FALSE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_BTN_MFRAME);
        _SetRoundObject(manager, obj, 5, 5);

        obj = _CreateManagerButton(manager, 327, x, 30, 16, TRUE, MANAGER_BTN_PFRAME,
                                   FONT_LABEL_NAME_ID, "+", TRUE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_BTN_MFRAME);
        _SetRoundObject(manager, obj, 5, 5);

        obj = _CreateManagerScrollLinear(manager, 0, x -= 21, 0, 18, TRUE, MANAGER_SCROLLLINEAR_FRAMES,
                                         1, 1, 8);   x += 24;

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 353, 27);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_SCRLINEAR_BACKGROUND_DIV);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_CONFIG);
        _SetItemTab(obj, MGR_MMSF);
        _SetScrollLinearRemote(manager, obj, MANAGER_BTN_MFRAME, MANAGER_BTN_PFRAME, MANAGER_STATIC_FRAMES);
        _SetObjectsFlags(obj, OBJ_TRACKBAR_OVERFLOW);

        obj = _CreateManagerRect(manager, 0, x += 22, 0, 1, FALSE, MANAGER_RECT_DIV_LINE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 257, 27);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_CONFIG);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_RECT_DIV_LINE);
        _SetItemTab(obj, MGR_MMSF);

        obj = _CreateManagerStatic(manager, 257, x += 4, 89, 16, TRUE, MANAGER_STATIC_SPEED,
                                   FONT_FRAMES_ID, "Speed: Nx");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_FRAMES);

        obj = _CreateManagerButton(manager, 257, x += 21, 30, 16, TRUE, MANAGER_BTN_MSPEED,
                                   FONT_LABEL_NAME_ID, "-", TRUE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_BTN_MFRAME);
        _SetRoundObject(manager, obj, 5, 5);

        obj = _CreateManagerButton(manager, 292, x, 30, 16, TRUE, MANAGER_BTN_PSPEED,
                                   FONT_LABEL_NAME_ID, "+", TRUE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_BTN_MFRAME);
        _SetRoundObject(manager, obj, 5, 5);

        obj = _CreateManagerScrollLinear(manager, 0, x -= 21, 0, 18, TRUE, MANAGER_SCROLLLINEAR_SPEED,
                                         20, 5, 80); x += 24;

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 353, 27);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_SCRLINEAR_BACKGROUND_DIV);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_CONFIG);
        _SetItemTab(obj, MGR_MMSF);
        _SetScrollLinearRemote(manager, obj, MANAGER_BTN_MSPEED, MANAGER_BTN_PSPEED, MANAGER_STATIC_SPEED);

        // MGR_MMWSF DIV 1
        obj = _CreateManagerRect(manager, 0, xdiv, 0, x + 30 - cdiv, FALSE, MANAGER_RECT_DIV);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 240, 10);
        _SetRoundObject(manager, obj, 15, 15);
        _SetItemTab(obj, MGR_MMSF);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_CONFIG);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_RECT_DIV);

        xdiv += x + 30 - cdiv + 10;
        x = xdiv;
        cdiv = xdiv;

        /// -----------------------------------------------------------------------------

        obj = _CreateManagerStatic(manager, 257, x += 12, 89, 16, TRUE, MANAGER_STATIC_SCALE,
                                   FONT_FRAMES_ID, "Scale: N%");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_FRAMES);

        obj = _CreateManagerButton(manager, 257, x += 21, 30, 16, TRUE, MANAGER_BTN_MSCALE,
                                   FONT_LABEL_NAME_ID, "-", TRUE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_BTN_MFRAME);
        _SetRoundObject(manager, obj, 5, 5);

        obj = _CreateManagerButton(manager, 292, x, 30, 16, TRUE, MANAGER_BTN_PSCALE,
                                   FONT_LABEL_NAME_ID, "+", TRUE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_BTN_MFRAME);
        _SetRoundObject(manager, obj, 5, 5);

        obj = _CreateManagerScrollLinear(manager, 0, x -= 21, 0, 18, TRUE, MANAGER_SCROLLLINEAR_SCALE,
                                         100, 1, 200); x += 30;

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 353, 27);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_SCRLINEAR_BACKGROUND_DIV);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_CONFIG);
        _SetItemTab(obj, MGR_MMSF);
        _SetScrollLinearRemote(manager, obj, MANAGER_BTN_MSCALE, MANAGER_BTN_PSCALE, MANAGER_STATIC_SCALE);

        obj = _CreateManagerRect(manager, 0, x += 16, 0, 1, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_RECT_DIV_LINE);

        obj = _CreateManagerStatic(manager, 0, x += 12, 0, 20, TRUE, MANAGER_STATIC_LABEL,
                                   FONT_LABEL_ID, "Always on top");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 257, 70);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_STATIC_DIV);
        _SetTextFormat(obj, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_CONFIG);
        _SetItemTab(obj, MGR_MMSF);

        obj = _CreateManagerSwitch(manager, 0, x += 4, 33, TRUE, MANAGER_SWITCH_AOT);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNR, manager->scale_dpi, 27);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_SWITCH_BACKGROUND_DIV);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_CONFIG);
        _SetItemTab(obj, MGR_MMSF);

        obj = _CreateManagerStatic(manager, 0, x += 17, 0, 15, TRUE, MANAGER_STATIC_DESCRIPTION,
                                   FONT_DESCRIPTION_ID, "Stays above all other windows");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 257, 70);
        _SetManagerObjectColor(obj, MGR_COLOR_STATIC_DIV, MGR_COLOR_STATIC_TEXT_DESCRIPTION);
        _SetTextFormat(obj, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_CONFIG);
        _SetItemTab(obj, MGR_MMSF);

        obj = _CreateManagerRect(manager, 0, x += 30, 0, 1, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_RECT_DIV_LINE);

        obj = _CreateManagerStatic(manager, 0, x += 12, 0, 20, TRUE, 0,
                                   FONT_LABEL_ID, "Show taskbar icon");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_LABEL);

        obj = _CreateManagerSwitch(manager, 0, x += 4, 33, TRUE, MANAGER_SWITCH_STI);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_SWITCH_AOT);

        obj = _CreateManagerStatic(manager, 0, x += 17, 0, 15, TRUE, 0,
                                   FONT_DESCRIPTION_ID, "Displays window icon in the taskbar");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_DESCRIPTION);

        // MGR_MMWSF DIV 2
        obj = _CreateManagerRect(manager, 0, xdiv, 0, x + 30 - cdiv, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_RECT_DIV);

        xdiv += x + 30 - cdiv + 10;
        x = xdiv;
        cdiv = xdiv;

        /// -----------------------------------------------------------------------------

        obj = _CreateManagerStatic(manager, 0, x += 11, 0, 20, TRUE, MANAGER_STATIC_DM,
                                   FONT_LABEL_ID, "Disable moving");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_LABEL);

        obj = _CreateManagerSwitch(manager, 0, x += 4, 33, TRUE, MANAGER_SWITCH_DM);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_SWITCH_AOT);

        obj = _CreateManagerStatic(manager, 0, x += 17, 0, 15, TRUE, MANAGER_STATIC_DM,
                                   FONT_DESCRIPTION_ID, "Locks the window in its current position");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_DESCRIPTION);

        obj = _CreateManagerRect(manager, 0, x += 30, 0, 1, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_RECT_DIV_LINE);

        obj = _CreateManagerStatic(manager, 0, x += 13, 0, 20, TRUE, 0,
                                   FONT_LABEL_ID, "Hide on hover");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_LABEL);

        obj = _CreateManagerSwitch(manager, 0, x += 4, 33, TRUE, MANAGER_SWITCH_HH);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_SWITCH_AOT);

        obj = _CreateManagerStatic(manager, 0, x += 17, 0, 15, TRUE, 0,
                                   FONT_DESCRIPTION_ID, "Disappears when the mouse cursor moves over it");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_DESCRIPTION);

        obj = _CreateManagerRect(manager, 0, x += 30, 0, 1, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_RECT_DIV_LINE);

        obj = _CreateManagerStatic(manager, 0, x += 13, 0, 20, TRUE, 0,
                                   FONT_LABEL_ID, "Click-through");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_LABEL);

        obj = _CreateManagerSwitch(manager, 0, x += 4, 33, TRUE, MANAGER_SWITCH_CT);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_SWITCH_AOT);

        obj = _CreateManagerStatic(manager, 0, x += 17, 0, 15, TRUE, 0,
                                   FONT_DESCRIPTION_ID, "Mouse clicks pass through to the window below");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_DESCRIPTION);

        // MGR_MMWSF DIV 3
        obj = _CreateManagerRect(manager, 0, xdiv, 0, x + 30 - cdiv, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_RECT_DIV);

        obj = _CreateManagerRect(manager, 0, xdiv + (x + 30 - cdiv), 0, 9, FALSE, MANAGER_RECT_DIV_LINE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        // MGR_MMWSF DIV END
        _SetParamsFrom(manager, obj, MANAGER_RECT_DIV);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_BKG);
    }

    /** MGR_ST **/

    {
        obj = _CreateManagerButton(manager, 0, 0, 150, 20, TRUE, MANAGER_BTN_BACK,
                                   FONT_MAIN_ID, "Back", FALSE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetItemTab(obj, MGR_ST1 | MGR_ST2);

        obj = _CreateManagerRect(manager, 150, 0, 1, 1, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNH, manager->scale_dpi, 0, 0);
        _SetItemTab(obj, MGR_ST1 | MGR_ST2);

        obj = _CreateManagerButton(manager, 0, 20, 150, 20, TRUE, MANAGER_BTN_GENERAL,
                                   FONT_MAIN_ID, "General", FALSE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetItemTab(obj, MGR_ST1 | MGR_ST2);

        obj = _CreateManagerButton(manager, 0, 40, 150, 20, TRUE, MANAGER_BTN_TEMPLATE,
                                   FONT_MAIN_ID, "Defaults", FALSE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetItemTab(obj, MGR_ST1 | MGR_ST2);
    }

    /** MGR_ST1 **/

    {
        int xdiv = 10, x = xdiv, cdiv = xdiv;

        obj = _CreateManagerScrollBar(manager, 0, 0, 5, 0, TRUE, MANAGER_SCROLL_ST1);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNRH, manager->scale_dpi, 3, xdiv - 5, 3);
        _SetItemTab(obj, MGR_ST1);
        _SetScrollableRect(manager, obj, 150, 21, SHRT_MAX, SHRT_MAX);

        obj = _CreateManagerStatic(manager, 0, x += 12, 0, 20, TRUE, MANAGER_STATIC_LABELST,
                                   FONT_LABEL_ID, APP_GENERAL_VER);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 177, 70);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_STATIC_DIV);
        _SetTextFormat(obj, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_ST1);
        _SetItemTab(obj, MGR_ST1);

        obj = _CreateManagerStatic(manager, 0, x += 21, 0, 15, TRUE, MANAGER_STATIC_DESCRIPTIONST,
                                   FONT_DESCRIPTION_ID, APP_BUILD_DESC);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 177, 70);
        _SetManagerObjectColor(obj, MGR_COLOR_STATIC_DIV, MGR_COLOR_STATIC_TEXT_DESCRIPTION);
        _SetTextFormat(obj, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_ST1);
        _SetItemTab(obj, MGR_ST1);

        obj = _CreateManagerRect(manager, 0, x += 30, 0, 1, FALSE, MANAGER_RECT_DIV_LINEST);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 177, 27);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_ST1);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_RECT_DIV_LINE);
        _SetItemTab(obj, MGR_ST1);

        obj = _CreateManagerStatic(manager, 0, x += 12, 0, 20, TRUE, 0,
                                   FONT_LABEL_ID, "Launch on system boot");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_LABELST);

        obj = _CreateManagerSwitch(manager, 0, x += 4, 33, TRUE, MANAGER_SWITCH_LSB);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNR, manager->scale_dpi, 27);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_SWITCH_BACKGROUND_DIV);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_ST1);
        _SetItemTab(obj, MGR_ST1);

        obj = _CreateManagerStatic(manager, 0, x += 17, 0, 15, TRUE, 0,
                                   FONT_DESCRIPTION_ID, "Launch the app in the background when Windows starts");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_DESCRIPTIONST);

        obj = _CreateManagerRect(manager, 0, x += 30, 0, 1, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_RECT_DIV_LINEST);

        obj = _CreateManagerStatic(manager, 0, x += 12, 0, 20, TRUE, 0,
                                   FONT_LABEL_ID, "Minimize to system tray on close");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_LABELST);

        obj = _CreateManagerSwitch(manager, 0, x += 4, 33, TRUE, MANAGER_SWITCH_MSTC);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_SWITCH_LSB);

        obj = _CreateManagerStatic(manager, 0, x += 17, 0, 15, TRUE, 0,
                                   FONT_DESCRIPTION_ID, "Keep the app running in the background when you close the window");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_DESCRIPTIONST);

        obj = _CreateManagerRect(manager, 0, xdiv, 0, x + 30 - cdiv, FALSE, MANAGER_RECT_DIVST);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 160, 10);
        _SetRoundObject(manager, obj, 15, 15);
        _SetItemTab(obj, MGR_ST1);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_ST1);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_RECT_DIV);

        obj = _CreateManagerRect(manager, 0, xdiv + (x + 30 - cdiv), 0, 9, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_RECT_DIVST);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_BKG);

        xdiv += x + 30 - cdiv + 10;
        x = xdiv;
        cdiv = xdiv;
    }

    /** MGR_ST2 **/

    {
        int xdiv = 10, x = xdiv, cdiv = xdiv;

        obj = _CreateManagerScrollBar(manager, 0, 0, 5, 0, TRUE, MANAGER_SCROLL_ST2);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNRH, manager->scale_dpi, 3, xdiv - 5, 3);
        _SetItemTab(obj, MGR_ST2);
        _SetScrollableRect(manager, obj, 150, 21, SHRT_MAX, SHRT_MAX);

        obj = _CreateManagerStatic(manager, 177, x += 12, 89, 16, TRUE, MANAGER_STATIC_SPEED_ST2,
                                   FONT_FRAMES_ID, "Speed: Nx");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetManagerObjectColori(obj, 0, MGR_COLOR_STATIC_DIV);
        _SetTextFormat(obj, DT_LEFT | DT_SINGLELINE);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_ST2);
        _SetItemTab(obj, MGR_ST2);

        obj = _CreateManagerButton(manager, 177, x += 21, 30, 16, TRUE, MANAGER_BTN_MSPEED_ST2,
                                   FONT_LABEL_NAME_ID, "-", TRUE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetRoundObject(manager, obj, 5, 5);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_BUTTON_BACKGROUND_DIV);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_ST2);
        _SetItemTab(obj, MGR_ST2);

        obj = _CreateManagerButton(manager, 212, x, 30, 16, TRUE, MANAGER_BTN_PSPEED_ST2,
                                   FONT_LABEL_NAME_ID, "+", TRUE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_BTN_MSPEED_ST2);
        _SetRoundObject(manager, obj, 5, 5);

        obj = _CreateManagerScrollLinear(manager, 0, x -= 21, 0, 18, TRUE, MANAGER_SCROLLLINEAR_SPEED_ST2,
                                         20, 5, 80); x += 24;

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 273, 27);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_SCRLINEAR_BACKGROUND_DIV);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_ST2);
        _SetItemTab(obj, MGR_ST2);
        _SetScrollLinearRemote(manager, obj, MANAGER_BTN_MSPEED_ST2, MANAGER_BTN_PSPEED_ST2, MANAGER_STATIC_SPEED_ST2);

        obj = _CreateManagerRect(manager, 0, xdiv, 0, x + 30 - cdiv, FALSE, MANAGER_RECT_DIVST2);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 160, 10);
        _SetRoundObject(manager, obj, 15, 15);
        _SetItemTab(obj, MGR_ST2);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_ST2);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_RECT_DIV);

        xdiv += x + 30 - cdiv + 10;
        x = xdiv;
        cdiv = xdiv;

        /// ------------------------------------------------------------------------------------

        obj = _CreateManagerStatic(manager, 177, x += 12, 89, 16, TRUE, MANAGER_STATIC_SCALE_ST2,
                                   FONT_FRAMES_ID, "Scale: N%");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_SPEED_ST2);

        obj = _CreateManagerButton(manager, 177, x += 21, 30, 16, TRUE, MANAGER_BTN_MSCALE_ST2,
                                   FONT_LABEL_NAME_ID, "-", TRUE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_BTN_MSPEED_ST2);
        _SetRoundObject(manager, obj, 5, 5);

        obj = _CreateManagerButton(manager, 212, x, 30, 16, TRUE, MANAGER_BTN_PSCALE_ST2,
                                   FONT_LABEL_NAME_ID, "+", TRUE);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_BTN_MSPEED_ST2);
        _SetRoundObject(manager, obj, 5, 5);

        obj = _CreateManagerScrollLinear(manager, 0, x -= 21, 0, 18, TRUE, MANAGER_SCROLLLINEAR_SCALE_ST2,
                                         100, 1, 200); x += 30;

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 273, 27);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_SCRLINEAR_BACKGROUND_DIV);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_ST2);
        _SetItemTab(obj, MGR_ST2);
        _SetScrollLinearRemote(manager, obj, MANAGER_BTN_MSCALE_ST2, MANAGER_BTN_PSCALE_ST2, MANAGER_STATIC_SCALE_ST2);

        obj = _CreateManagerRect(manager, 0, x += 16, 0, 1, FALSE, MANAGER_RECT_DIV_LINE_ST2);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 177, 27);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_ST2);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_RECT_DIV_LINE);
        _SetItemTab(obj, MGR_ST2);

        obj = _CreateManagerStatic(manager, 0, x += 12, 0, 20, TRUE, MANAGER_STATIC_LABEL_ST2,
                                   FONT_LABEL_ID, "Always on top");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 177, 70);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_STATIC_DIV);
        _SetTextFormat(obj, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_ST2);
        _SetItemTab(obj, MGR_ST2);

        obj = _CreateManagerSwitch(manager, 0, x += 4, 33, TRUE, MANAGER_SWITCH_AOT_ST2);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNR, manager->scale_dpi, 27);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_SWITCH_BACKGROUND_DIV);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_ST2);
        _SetItemTab(obj, MGR_ST2);

        obj = _CreateManagerStatic(manager, 0, x += 17, 0, 15, TRUE, MANAGER_STATIC_DESCRIPTION_ST2,
                                   FONT_DESCRIPTION_ID, "Stays above all other windows");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetAlign(manager->window, obj, OBJ_ALIGNW, manager->scale_dpi, 177, 70);
        _SetManagerObjectColor(obj, MGR_COLOR_STATIC_DIV, MGR_COLOR_STATIC_TEXT_DESCRIPTION);
        _SetTextFormat(obj, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        _SetScrollObjectByID(manager, obj, MANAGER_SCROLL_ST2);
        _SetItemTab(obj, MGR_ST2);

        obj = _CreateManagerRect(manager, 0, x += 30, 0, 1, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_RECT_DIV_LINE_ST2);

        obj = _CreateManagerStatic(manager, 0, x += 12, 0, 20, TRUE, 0,
                                   FONT_LABEL_ID, "Show taskbar icon");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_LABEL_ST2);

        obj = _CreateManagerSwitch(manager, 0, x += 4, 33, TRUE, MANAGER_SWITCH_STI_ST2);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_SWITCH_AOT_ST2);

        obj = _CreateManagerStatic(manager, 0, x += 17, 0, 15, TRUE, 0,
                                   FONT_DESCRIPTION_ID, "Displays window icon in the taskbar");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_DESCRIPTION_ST2);

        obj = _CreateManagerRect(manager, 0, xdiv, 0, x + 30 - cdiv, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_RECT_DIVST2);

        xdiv += x + 30 - cdiv + 10;
        x = xdiv;
        cdiv = xdiv;

        /// ------------------------------------------------------------------------------------

        obj = _CreateManagerStatic(manager, 0, x += 11, 0, 20, TRUE, MANAGER_STATIC_DM_ST2,
                                   FONT_LABEL_ID, "Disable moving");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_LABEL_ST2);

        obj = _CreateManagerSwitch(manager, 0, x += 4, 33, TRUE, MANAGER_SWITCH_DM_ST2);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_SWITCH_AOT_ST2);

        obj = _CreateManagerStatic(manager, 0, x += 17, 0, 15, TRUE, 0,
                                   FONT_DESCRIPTION_ID, "Locks the window in its current position");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_DESCRIPTION_ST2);

        obj = _CreateManagerRect(manager, 0, x += 30, 0, 1, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_RECT_DIV_LINE_ST2);

        obj = _CreateManagerStatic(manager, 0, x += 13, 0, 20, TRUE, 0,
                                   FONT_LABEL_ID, "Hide on hover");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_LABEL_ST2);

        obj = _CreateManagerSwitch(manager, 0, x += 4, 33, TRUE, MANAGER_SWITCH_HH_ST2);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_SWITCH_AOT_ST2);

        obj = _CreateManagerStatic(manager, 0, x += 17, 0, 15, TRUE, 0,
                                   FONT_DESCRIPTION_ID, "Disappears when the mouse cursor moves over it");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_DESCRIPTION_ST2);

        obj = _CreateManagerRect(manager, 0, x += 30, 0, 1, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_RECT_DIV_LINE_ST2);

        obj = _CreateManagerStatic(manager, 0, x += 13, 0, 20, TRUE, 0,
                                   FONT_LABEL_ID, "Click-through");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_LABEL_ST2);

        obj = _CreateManagerSwitch(manager, 0, x += 4, 33, TRUE, MANAGER_SWITCH_CT_ST2);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_SWITCH_AOT_ST2);

        obj = _CreateManagerStatic(manager, 0, x += 17, 0, 15, TRUE, 0,
                                   FONT_DESCRIPTION_ID, "Mouse clicks pass through to the window below");

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_STATIC_DESCRIPTION_ST2);

        obj = _CreateManagerRect(manager, 0, xdiv, 0, x + 30 - cdiv, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_RECT_DIVST2);

        /// END_LINE

        obj = _CreateManagerRect(manager, 0, xdiv + (x + 30 - cdiv), 0, 9, FALSE, 0);

        if (!obj) {
            manager->error = MANAGER_ERR_CREATE_OBJ;
            ManagerHandleError(manager);
        }

        _SetParamsFrom(manager, obj, MANAGER_RECT_DIVST2);
        _SetManagerObjectColori(obj, 0, MGR_COLOR_BKG);
    }

    GetSettingsPath(manager);
    ParseSettings(manager);
    DragAcceptFiles(manager->window, TRUE);
    _UpdateTabItems(manager);

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-silent") || !strcmp(argv[i], "-s")) {
            ShowWindow(manager->window, SW_HIDE);
            return manager;
        }
    }

    SetWindowPos(manager->window, NULL, 0, 0,
                 ScaleForDPI(640, manager->scale_dpi), ScaleForDPI(527, manager->scale_dpi),
                 SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
    ShowWindow(manager->window, SW_SHOW);

    /** AnimObjects initial **/

    return manager;
}

SINLINE void ManagerRun(Manager* manager) {
    if (!manager) return;
    while (GetMessage(&manager->message, NULL, 0, 0)) {
        TranslateMessage(&manager->message);
        DispatchMessage(&manager->message);
    }
}

#endif // MANAGER_H_INCLUDED

#ifndef GIFDESK_H_INCLUDED
#define GIFDESK_H_INCLUDED

#include "types.h"

SINLINE void _SetLoadRect(Manager* manager, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) {
    manager->rc_load.left = ScaleForDPI(left, manager->scale_dpi);
    manager->rc_load.top = ScaleForDPI(top, manager->scale_dpi);
    manager->rc_load.right = ScaleForDPI(right, manager->scale_dpi);
    manager->rc_load.bottom = ScaleForDPI(bottom, manager->scale_dpi);
    InvalidateRect(manager->window,
                   (manager->rc_load.left || manager->rc_load.top || manager->rc_load.right || manager->rc_load.bottom) ? &manager->rc_load : NULL,
                   TRUE);
}

SINLINE void _SetReleaseRect(Manager* manager, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) {
    manager->rc_release.left = ScaleForDPI(left, manager->scale_dpi);
    manager->rc_release.top = ScaleForDPI(top, manager->scale_dpi);
    manager->rc_release.right = ScaleForDPI(right, manager->scale_dpi);
    manager->rc_release.bottom = ScaleForDPI(bottom, manager->scale_dpi);
    InvalidateRect(manager->window,
                   (manager->rc_release.left || manager->rc_release.top || manager->rc_release.right || manager->rc_release.bottom) ? &manager->rc_release : NULL,
                   TRUE);
}

#include "data.h"
#include "render.h"
#include "settings.h"
#include "handlers.h"

SINLINE void EnableOpenGL(Manager* manager, uint16_t index) {
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    manager->gfk[index].hdc = GetDC(manager->gfk[index].window);

    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 16;

    iFormat = ChoosePixelFormat(manager->gfk[index].hdc, &pfd);
    SetPixelFormat(manager->gfk[index].hdc, iFormat, &pfd);

    WaitForSingleObject(manager->glmutex, INFINITE);
    manager->gfk[index].hrc = wglCreateContext(manager->gfk[index].hdc);

    wglMakeCurrent(manager->gfk[index].hdc, manager->gfk[index].hrc);

    int major, minor;
    sscanf((const char*)glGetString(GL_VERSION), "%d.%d", &major, &minor);
    manager->gfk[index].glversion = (float)major + (float)minor / 10;

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.75f);

    wglMakeCurrent(NULL, NULL);
    ReleaseMutex(manager->glmutex);
}

SINLINE void DisableOpenGL(Manager* manager) {
    WaitForSingleObject(manager->glmutex, INFINITE);
    wglMakeCurrent(NULL, NULL);
    for (int i = 0; i < manager->gfk_count; i++) {
        wglDeleteContext(manager->gfk[i].hrc);
        ReleaseDC(manager->gfk[i].window, manager->gfk[i].hdc);
    }
    ReleaseMutex(manager->glmutex);
}

SINLINE uint8_t GIFDeskWindow(Manager* manager, uint16_t index) {
    DWORD exstyle = WS_EX_LAYERED | WS_EX_TOOLWINDOW;
    DWORD dstyle = 0;

    HWND window = CreateWindowEx(
        exstyle,
        "GIFDesk",
        manager->gfk[index].filename,
        dstyle,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1,
        1,
        HWND_DESKTOP,
        NULL,
        manager->gfk[index].hinst,
        (LPVOID)&manager->gfk[index]
    );
    if (!window) {
        manager->error = MANAGER_WARN_CREATE_WINDOW;
        ManagerHandleError(manager);
        return 0;
    }
    manager->gfk[index].window = window;

    dstyle = GetWindowLong(manager->gfk[index].window, GWL_STYLE);
    dstyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
    SetWindowLong(manager->gfk[index].window, GWL_STYLE, dstyle);

    EnableOpenGL(manager, index);

    SetLayeredWindowAttributes(manager->gfk[index].window, 0x0, 0, LWA_COLORKEY);
    SetWindowPos(manager->gfk[index].window, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    return 1;
}

SINLINE void GIFDeskRelease(Manager* manager, uint16_t index) {
    if (&manager->gfk[index]) {
        manager->gfk[index].render_thread = 0;
        if (manager->gfk[index].textures && manager->gfk[index].count && manager->gfk[index].hrc) {
            WaitForSingleObject(manager->glmutex, INFINITE);
            wglMakeCurrent(manager->gfk[index].hdc, manager->gfk[index].hrc);
            glDeleteTextures(manager->gfk[index].count, manager->gfk[index].textures);
            wglMakeCurrent(NULL, NULL);
            ReleaseMutex(manager->glmutex);

            free(manager->gfk[index].textures); manager->gfk[index].textures = NULL;
        }
        if (manager->gfk[index].window) { DestroyWindow(manager->gfk[index].window); manager->gfk[index].window = NULL; }
        if (manager->gfk[index].hrc) { wglDeleteContext(manager->gfk[index].hrc); manager->gfk[index].hrc = NULL; }
        if (manager->gfk[index].hdc) { ReleaseDC(NULL, manager->gfk[index].hdc); manager->gfk[index].hdc = NULL; }
        if (manager->gfk[index].frame_points) { free(manager->gfk[index].frame_points); manager->gfk[index].frame_points = NULL; }
        if (manager->gfk[index].delays) { free(manager->gfk[index].delays); manager->gfk[index].delays = NULL; }
        if (manager->gfk[index].lengths) { free(manager->gfk[index].lengths); manager->gfk[index].lengths = NULL; }
        if (manager->gfk[index].frame) { free(manager->gfk[index].frame); manager->gfk[index].frame = NULL; }
        if (manager->gfk[index].buff) { free(manager->gfk[index].buff); manager->gfk[index].buff = NULL; }
        if (manager->gfk[index].data) { free(manager->gfk[index].data); manager->gfk[index].data = NULL; }
        memset(&manager->gfk[index], 0, sizeof(GIFDesk));
    }
}

DWORD WINAPI GIFDeskFromParams(Manager* manager, uint16_t index, char* filepath, float size, int16_t x, int16_t y,
                               uint16_t speed, uint8_t transparency, uint8_t lang,
                               uint8_t flags, uint8_t sfu) {
    float vertex[] = {-1, 1, 1, 1, 1, -1, -1, -1};
    float texcoord[] = {0, 0, 1, 0, 1, 1, 0, 1};
    static DWORD style;

    for (int i = 0; i < manager->objects_count; i++) {
        if (!&manager->objects[i]) continue;

        if (manager->objects[i].id == MANAGER_BTN_FILE) {
            _SetManagerObjectColori(&manager->objects[i], 0, MGR_COLOR_BUTTON_BACKGROUND_DEFAULT);
            InvalidateRect(manager->objects[i].window, NULL, TRUE);
        }
    }

    manager->tab = MGR_MMWF;
    _UpdateTabItems(manager);

    _SetLoadRect(manager, 20, 20 + 20 * index, 30, 40 + 20 * index); // = 5%

    for (int i = 0; i < manager->gfk_count; i++)
        SetWindowLongPtr(manager->gfk[i].window, GWLP_USERDATA, (LONG_PTR)&manager->gfk[i]);

    manager->gfk[index].mgr = (void *)manager;
    memcpy(&manager->gfk[index].vertex, vertex, sizeof(vertex));
    memcpy(&manager->gfk[index].texcoord, texcoord, sizeof(texcoord));
    manager->gfk[index].change_frames = 1;
    manager->gfk[index].schange_frames = 1;
    manager->gfk[index].index = index;
    manager->gfk[index].render_thread = 1;
    manager->gfk[index].size = size;
    manager->gfk[index].x = x;
    manager->gfk[index].y = y;
    manager->gfk[index].speed = speed;
    manager->gfk[index].transparency = transparency;
    manager->gfk[index].lang = lang;
    manager->gfk[index].flags = flags;
    manager->gfk[index].sfu = sfu;
    manager->is_loading = 1;

    /** Checking the file header **/
    uint8_t filetype = CheckFile(filepath, manager, index);
    if (!filetype) return 0;

    _SetLoadRect(manager, 20, 20 + 20 * index, 34, 40 + 20 * index); // = 7%

    /** Confirming file **/
    strncpy(manager->buff_filepath, filepath, MAX_PATH);
    SwapFilenames(manager, index);

    /** Loading the window **/
    LRESULT msg = SendMessage(manager->window, WM_COMMAND, MANAGER_THREAD_CREATEWINDOW, index);
/// ѕочему-то не всегда возращает 1, хот€ по логам ничто не вызывает возврат нул€
    if (!msg) {
        manager->error = MANAGER_WARN_APPLY_WINDOW;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 38, 40 + 20 * index); // = 9%

    /** Loading the file **/
    filetype = LoadFile(manager, index, filetype); // = [10% ... 90%]
    if (!filetype) return 0;

    _SetLoadRect(manager, 20, 20 + 20 * index, 200, 40 + 20 * index); // 90%

    /** Setting window **/
    manager->gfk[index].x = x;
    manager->gfk[index].y = y;

    SetWindowPos(manager->gfk[index].window,
                 (manager->gfk[index].flags & SETTINGS_AOT) ? HWND_TOPMOST : HWND_NOTOPMOST,
                 x, y,
                 (int)((float)manager->gfk[index].width * manager->gfk[index].size + 0.5),
                 (int)((float)manager->gfk[index].height * manager->gfk[index].size + 0.5),
                 SWP_SHOWWINDOW);

    /** Setting showing tasbar icon **/
    style = GetWindowLong(manager->gfk[index].window, GWL_EXSTYLE);

    SetWindowLong(manager->gfk[index].window, GWL_EXSTYLE,
                 (manager->gfk[index].flags & SETTINGS_STI) ? (style | WS_EX_APPWINDOW) & ~WS_EX_TOOLWINDOW :
                                                              (style | WS_EX_TOOLWINDOW) & ~WS_EX_APPWINDOW);

    ShowWindow(manager->gfk[index].window, SW_HIDE);
    ShowWindow(manager->gfk[index].window, SW_SHOW);

    _SetLoadRect(manager, 20, 20 + 20 * index, 210, 40 + 20 * index); // 95%

//    /** Creating the file Object* **/
//    msg = SendMessage(manager->window, WM_COMMAND, MANAGER_THREAD_CREATEOBJECT, (LPARAM)index);
//    if (!msg) return 0;

    /** Setting sync playback **/
    if (manager->flags & SETTINGS_SP && index)
        manager->gfk[index].start_time = manager->gfk[index - 1].start_time;
    else
        manager->gfk[index].start_time = GetTime();

    /** Setting thread loop **/
    GIFDeskLoop* args = malloc(sizeof(GIFDeskLoop));
    if (!args) {
        manager->error = MANAGER_WARN_GFKLOOP_NULL;
        ManagerHandleError(manager);
        return 0;
    }

    args->manager = manager;
    args->gfk = &manager->gfk[index];
    args->gfk_index = manager->gfk[index].index;

    manager->gfk_count++;
    if (!CreateThread(NULL, 0, Loop, (LPVOID)args, 0, NULL)) {
        manager->error = MANAGER_WARN_CREATE_LOOP;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 215, 40 + 20 * index); // 98%

    /** Creating the file Object* **/
    msg = SendMessage(manager->window, WM_COMMAND, MANAGER_THREAD_CREATEOBJECT, (LPARAM)index);
    if (!msg) return 0;

    _SetLoadRect(manager, 0, 0, 0, 0);
    manager->is_loading = 0;

    return 1;
}

SINLINE DWORD WINAPI GIFDeskNew(LPVOID arg) {
    Manager* manager = (Manager *)arg;

    GIFDesk* buff = realloc(manager->gfk, sizeof(GIFDesk) * (manager->gfk_count + 1));
    if (!buff) {
        manager->error = MANAGER_WARN_GIFDESK_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }
    manager->gfk = buff;
    memset(&manager->gfk[manager->gfk_count], 0, sizeof(GIFDesk));

    DragAcceptFiles(manager->window, FALSE);

    GIFDeskFromParams(manager,
                      manager->gfk_count,
                      manager->buff_filepath,
                      manager->size,
                      (int16_t)CW_USEDEFAULT,
                      (int16_t)CW_USEDEFAULT,
                      manager->speed, manager->transparency,
                      manager->lang, manager->flags,
                      0);

    DragAcceptFiles(manager->window, TRUE);
    WriteSettings(manager);
    return 0;
}

SINLINE DWORD WINAPI GIFDeskClose(LPVOID arg) {
    Manager* manager = (Manager *)arg;

    if (manager->context_id < 0) return 0;

    Object* obj;
    GIFDesk* gfk;
    RECT rect, /* stm_rect, */ wrect;
    int stm_last = 0, stm_delta = 0, stm_current = 0;

    obj = &manager->objects[manager->context_id];
    gfk = &manager->gfk[obj->gfk_index];

    uint16_t obj_index = obj->index;
    uint16_t gfk_index = gfk->index;

    /** Clearing Object **/

    for (int i = 0; i < manager->objects_count; i++)
        if (manager->objects[i].gfk_index > gfk_index)
            manager->objects[i].gfk_index--;

    /* LRESULT msg = */ SendMessage(manager->window, WM_COMMAND, MANAGER_THREAD_DELETEOBJECT, (LPARAM)obj_index);

    for (uint16_t i = obj_index; i < manager->objects_count - 1; i++) {
        memmove(&manager->objects[i], &manager->objects[i + 1], sizeof(Object));
        manager->objects[i].index = i;
        SetWindowLongPtr(manager->objects[i].window, GWLP_ID, (LONG_PTR)(i + 1));

        if (manager->objects[i].id != MANAGER_BTN_FILE && obj_index != i) continue;
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


    /** Clearing GFK **/

    _SetReleaseRect(manager, 20, 20 + 20 * gfk->index, 38, 40 + 20 * gfk->index); // = 9%

    /* msg = */ SendMessage(manager->window, WM_COMMAND, MANAGER_THREAD_DELETEWINDOW, (LPARAM)gfk_index);

    for (uint16_t i = gfk_index; i < manager->gfk_count - 1; i++) {
        manager->gfk[i + 1].render_thread = 0;
        memmove(&manager->gfk[i], &manager->gfk[i + 1], sizeof(GIFDesk));
        manager->gfk[i].index = i;

        _SetReleaseRect(manager, 20, 20 + 20 * gfk->index, (uint16_t)(61 + (float)i / (float)manager->gfk_count - 1 * (float)(198 - 61)), 40 + 20 * gfk->index); // = [10% ... 90%]
    }

    _SetReleaseRect(manager, 20, 20 + 20 * gfk->index, 200, 40 + 20 * gfk->index); // = 90%

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

    _SetReleaseRect(manager, 20, 20 + 20 * gfk->index, 210, 40 + 20 * gfk->index); // = 95%

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

    _SetReleaseRect(manager, 20, 20 + 20 * gfk->index, 215, 40 + 20 * gfk->index); // = 98%

    /** Setting current gfk_current **/

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

    /** Setting current CASB **/

    if (manager->casb == obj_index) manager->casb = -1;
    else if (manager->casb > obj_index) manager->casb--;

    /** Setting current context_id **/

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

    /** Setting current TAB **/
    if (manager->gfk_count && manager->gfk_current > -1 && manager->context_id > -1) manager->tab = MGR_MMSF;
    else if (manager->gfk_count) manager->tab = MGR_MMWF;
    else manager->tab = MGR_MMWnF;
    _UpdateTabItems(manager);

    _SetReleaseRect(manager, 0, 0, 0, 0);

    return 0;
}

SINLINE DWORD WINAPI GIFDeskCloseAll(LPVOID arg) {
    Manager* manager = (Manager *)arg;
    for (int i = manager->gfk_count - 1; i >= 0; i--) {
        manager->context_id = manager->gfk[i].obj_index;

        GIFDeskClose(arg);
    }
    return 0;
}

SINLINE DWORD WINAPI GIFDeskCloseAllWS(LPVOID arg) {
    Manager* manager = (Manager *)arg;
    GIFDeskCloseAll(arg);
    WriteSettings(manager);
    return 0;
}

#endif // GIFDESK_H_INCLUDED

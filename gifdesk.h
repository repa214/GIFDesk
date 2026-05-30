#ifndef GIFDESK_H_INCLUDED
#define GIFDESK_H_INCLUDED

#include "types.h"
#include "data.h"
#include "render.h"
#include "settings.h"

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

    /** Confirming file **/
    strncpy(manager->buff_filepath, filepath, MAX_PATH);
    SwapFilenames(manager, index);

    /** Loading the window **/
    LRESULT msg = SendMessage(manager->window, WM_COMMAND, MANAGER_THREAD_CREATEWINDOW, index);
    if (!msg) return 0;

    /** Loading the file **/
    filetype = LoadFile(manager, index, filetype);
    if (!filetype) return 0;

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

    /** Creating the file Object* **/
    msg = SendMessage(manager->window, WM_COMMAND, MANAGER_THREAD_CREATEOBJECT, (LPARAM)index);
    if (!msg) return 0;

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

#endif // GIFDESK_H_INCLUDED

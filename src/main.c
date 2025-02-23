/**


        0.50: первый релиз
        0.51: исправлена ошибка переполнения памяти при загрузке анимации с большим разрешением
        0.52: отображенеие анимации соответствует её фреймрейту
        0.53: исправлено отображение анимаций с неявным значением Frame Blending Mode (1)
        0.54: исправлено отображение анимаций с неявным значением Frame Blending Mode (2)
        0.55: добавлена возможность показывать или скрывать окно приложения в панеле задач
        0.56: исправлено отображение анимаций с неявным значением Frame Blending Mode (3)
        0.57: дополнен сценарий обработки неподдерживаемых типов файла
        0.58: исправлен алгоритм обработки неподдерживаемых типов файла
        0.59: переименованы некоторые пункты настроек

        0.60: добавлена возможность масштабировать изображение
        0.61: исправлено отображение анимаций с неявным значением Frame Blending Mode (4)
        0.62: исправлена логика поведеня окна в таскбаре
        0.63: все параметры из ПКМ-меню сохраняются при перезапуске утилиты
        0.64: исправлен баг при котором не работало ПКМ-меню после манипуляций с масштабом
        0.65: доработана кнопка сохранения масштаба изображения
        0.66: исправлен баг отсутствия изображения, если отменить выбор GIF-файла, а затем изменить его масштаб
        0.67: исправлено поведение основного окна во время изменения масштаба
        0.68: добавлен предпросмотр изображения во время изменения масштаба (beta)
        0.69: исправлен предпросмотр у анимаций с неявными границами кадра

        ** 0.70: добавлена возможность поменять анимацию путём перетаскивания файла в окно


**/

#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <pthread.h>
#include <commctrl.h>

#include "resource.h" /** Для иконки приложения **/

#include "../Libraries/GIF_LOAD/gif_load.h"

#define APP_NAME "GIFDesk 0.69"


/**
        Инициализация
**/

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WindowProc_2(HWND, UINT, WPARAM, LPARAM);

void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

HWND hwnd;
HDC hdc;
HGLRC hRC;
WNDCLASSEX wcex;
MSG msg;
pthread_t thread;
PAINTSTRUCT ps;
RECT rect;
HINSTANCE hInstance;
int nCmdShow;
POINT p;
HMENU hMenu;
LONG_PTR exStyle;
FILE *file;

HWND hwnd_2;
WNDCLASSEX wcex_2;
HDC hdc_2;
HGLRC hRC_2;
MSG msg_2;
HWND hTrackbar;
HRGN hRgn;
HWND hButton;
HDC hdc_b;
HFONT hFont;

pthread_t render;

int width = 0, height = 0, fc = 0, TASKBAR = 1, TOPMOST = 1, DESTROY_WINDOW = 0, k = 0, WAITING = 0, HOVERED = 0;
struct timeval t_start, t_current;
double start, current;
char filename[260] = "";
char settings_path[260];
float size = 1;
char str_size[11];

float vertex[] = {-1,-1,0,  1,-1,0,  1,1,0,  -1,1,0};
float texCoord[] = {0,  1,
                    1,  1,
                    1,  0,
                    0,  0};

/**
        Инициализирует параметры окна
**/

void WcexInit(WNDCLASSEX *wcex, const char *lpszClassName, LRESULT CALLBACK Proc) {
    (*wcex).cbSize =           sizeof(WNDCLASSEX);
    (*wcex).style =            CS_OWNDC;
    (*wcex).lpfnWndProc =      Proc;
    (*wcex).cbClsExtra =       0;
    (*wcex).cbWndExtra =       0;
    (*wcex).hInstance =        hInstance;
    (*wcex).hIcon =            (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 64, 64, LR_DEFAULTCOLOR);
    (*wcex).hCursor =          LoadCursor(NULL, IDC_ARROW);
    (*wcex).hbrBackground =    (HBRUSH)GetStockObject(BLACK_BRUSH);
    (*wcex).lpszMenuName =     NULL;
    (*wcex).lpszClassName =    lpszClassName;
    (*wcex).hIconSm =          (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 64, 64, LR_DEFAULTCOLOR);
}


/**
        Получает путь к файлу "settings"
**/

char* GetSettingsPath() {
    static char str[260] = "";
    GetModuleFileName(NULL, str, sizeof(str));
    char* ls = strrchr(str, '\\');
    if (ls) *(ls + 1) = '\0';
    strcat(str, "settings");
    return str;
}


/**
        Работает с файлом "settings"
**/

int WriteSettings(const char *filename, float size, int taskbar, int topmost)
{
    FILE *f = fopen(settings_path, "wb");
    fwrite(filename, sizeof(char), 261, f);
    fwrite(&size, sizeof(float), 1, f);
    fwrite(&taskbar, sizeof(int), 1, f);
    fwrite(&topmost, sizeof(int), 1, f);

    fclose(f);
}

int ReadSettings(int fi)
{
    FILE *f = fopen(settings_path, "rb");
    if (f != NULL && !fi) {
        if (fread(filename, sizeof(char), 261, f) < 261) goto filename_init;
        if (fread(&size, sizeof(float), 1, f) < 1) goto filename_init;
        if (fread(&TASKBAR, sizeof(float), 1, f) < 1) goto filename_init;
        if (fread(&TOPMOST, sizeof(float), 1, f) < 1) goto filename_init;
    }
    else {
        goto filename_init;
    }
    fclose(f);
    return 1;

    filename_init:
        fclose(f);
        OPENFILENAME ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = filename;
        ofn.lpstrFile[0] = '\0';
        ofn.lpstrFilter = "GIF Files (*.gif)\0*.gif\0All Files (*.*)\0*.*\0";
        ofn.nMaxFile = sizeof(filename);
        ofn.lpstrTitle = "Select a GIF file to display";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileName(&ofn)) {
            if (CheckExtension((char const *)filename)) {
                WriteSettings(filename, size, TASKBAR, TOPMOST);
            }
            else {
                MessageBox(NULL, "This file is not a GIF-animation", APP_NAME, MB_ICONEXCLAMATION);
                return 0;
            }
        }
        else return 0;
        return 1;
}


/**
        Проверяет на GIF-анимацию
**/

int CheckExtension(const char *filename)
{
    void *data;

    void check_frame(void *anim, struct GIF_WHDR *whdr) {
        width = whdr->xdim;
        height = whdr->ydim;
    }

    FILE *file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    data = malloc(size);
    fread(data, 1, size, file);
    fclose(file);

    GIF_Load(data, size, check_frame, NULL, NULL, 0);

    free(data);

    if (width && height) {
        return 1;
    }

    return 0;
}


/**
        Записывает GIF-анимации в виде списка текстур
**/

unsigned int *textures = NULL;       /* Массив из текстур */
double *delays = NULL;               /* Массив времени существования каждого кадра */

void LoadTextures(const char *filename, int tran_lp)
{
    int past_mode;
    textures = (unsigned int *)malloc(sizeof(unsigned int) * 1);
    unsigned char *frame = (unsigned char *)malloc(width * height * 4);
    memset(frame, 0, width * height * 4);

    void write_frame(void *anim, struct GIF_WHDR *whdr) {
        textures = (unsigned int *)realloc(textures, sizeof(unsigned int) * (fc + 1));
        delays = (double *)realloc(delays, sizeof(double) * (fc + 1));
        *(delays + fc) = (whdr->time) ? (double)whdr->time / 100 : 0.1;

        // printf("Frame: %d Res: %dx%d\n", fc, whdr->frxd, whdr->fryd);

        if (whdr->mode == GIF_BKGD && past_mode == GIF_BKGD) memset(frame, 0, width * height * 4);
        else if (whdr->mode == GIF_CURR && past_mode == GIF_BKGD) memset(frame, 0, width * height * 4);
        else if (whdr->mode == GIF_PREV) memset(frame, 0, width * height * 4);

        unsigned int index = (width * whdr->fryo) * 4;

        for (long y = 0; y < whdr->fryd; y++) {
            index += whdr->frxo * 4;
            for (long x = 0; x < whdr->frxd; x++) {

                uint8_t i = whdr->bptr[y * whdr->frxd + x];

                if (whdr->tran != i) {
                    frame[index] = (whdr->cpal[i].R) ? whdr->cpal[i].R : whdr->cpal[i].R + 1;
                    frame[index + 1] = (whdr->cpal[i].G) ? whdr->cpal[i].G : whdr->cpal[i].G + 1;
                    frame[index + 2] = (whdr->cpal[i].B) ? whdr->cpal[i].B : whdr->cpal[i].B + 1;
                    frame[index + 3] = 255;
                }

                index += 4;
            }
            index += (width - whdr->frxd - whdr->frxo) * 4;
        }
        if (tran_lp) {
            index = 0;
            for (long y = 0; y < height; y++) {
                for (long x = 0; x < width; x++) {
                    if (x >= width - 1 || y >= height - 1) frame[index + 3] = 0;
                    index += 4;
                }
            }
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ZERO);

        glGenTextures(1, &textures[fc]);
        glBindTexture(GL_TEXTURE_2D, textures[fc]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                                    0, GL_RGBA, GL_UNSIGNED_BYTE, frame);

        glBindTexture(GL_TEXTURE_2D, 0);
        past_mode = whdr->mode;
        fc++;
    }

    FILE *file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    void *data = malloc(size);
    fread(data, 1, size, file);
    fclose(file);

    GIF_Load(data, size, write_frame, NULL, NULL, 0);

    free(data); free(frame);
}


/**
        Отобразить текстуру в окно
**/

void ShowFrame(int k)
{
    wglMakeCurrent(hdc, hRC);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, textures[k]);

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

    SwapBuffers(hdc);

    wglMakeCurrent(NULL, NULL);
}


/**
        Отображает текстуры в отдельном потоке когда окно занято
**/

void Render_Thread() {
    wglMakeCurrent(hdc, hRC);
    while (!DESTROY_WINDOW) {
        gettimeofday(&t_start, NULL);
        start = t_start.tv_sec + t_start.tv_usec / 1e6;

        ShowFrame(k);

        gettimeofday(&t_current, NULL);
        current = t_current.tv_sec + t_current.tv_usec / 1e6;

        while (current < start + *(delays + k) - 0.002) {
            if (current + 0.015 < start + *(delays + k)) Sleep(10);
            gettimeofday(&t_current, NULL); current = t_current.tv_sec + t_current.tv_usec / 1e6;
        }

        if (k >= fc - 1) k = 0;
        else k++;
    }
}


/**
        Место, где всё происходит
**/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    setlocale(LC_ALL, "Russian");

    WcexInit(&wcex, "Window", WindowProc);
    WcexInit(&wcex_2, "Window_2", WindowProc_2);

    if (!RegisterClassEx(&wcex)) return 0;
    if (!RegisterClassEx(&wcex_2)) return 0;

    strcpy(settings_path, GetSettingsPath());

    if (!ReadSettings(0)) { return 0; }


    CheckExtension((const char*)filename);
    printf("filename: %s\nsize: %.2f TASKBAR: %d TOPMOST: %d\n", filename, size, TASKBAR, TOPMOST);

    hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST,
                          "Window",
                          APP_NAME,
                          WS_POPUP | WS_VISIBLE,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          (width * size < 10.0) ? 10.0 : width * size,
                          (height * size < 10.0) ? 10.0 : height * size,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

    if (!TASKBAR) exStyle |= WS_EX_TOOLWINDOW;
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

    SetWindowPos(hwnd, (TOPMOST) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);

    SetLayeredWindowAttributes(hwnd, 0x0, 0, LWA_COLORKEY);

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    EnableOpenGL(hwnd, &hdc, &hRC);
    LoadTextures((char const *)filename, 0);

    while (1) {
        gettimeofday(&t_start, NULL);
        start = t_start.tv_sec + t_start.tv_usec / 1e6;

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); }
        else if (DESTROY_WINDOW) break;
        else {
            wglMakeCurrent(NULL, NULL);

            ShowFrame(k);

            gettimeofday(&t_current, NULL);
            current = t_current.tv_sec + t_current.tv_usec / 1e6;

            while (current < start + *(delays + k) - 0.002) {
                if (current + 0.015 < start + *(delays + k)) Sleep(10);
                gettimeofday(&t_current, NULL); current = t_current.tv_sec + t_current.tv_usec / 1e6;
            }

            if (k >= fc - 1) k = 0;
            else k++;

            wglMakeCurrent(hdc, hRC);
        }
    }

    pthread_join(render, NULL);
    DisableOpenGL(hwnd, hdc, hRC);
    DestroyWindow(hwnd);
    free(textures);
    free(delays);

    return 0;
}

/**
        Реагировать на сообщения от Windows
**/

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_QUIT: {
            DESTROY_WINDOW = 1;
        } break;

        case WM_CLOSE: {
            DESTROY_WINDOW = 1;
        } break;

        case WM_LBUTTONDOWN: {
            wglMakeCurrent(NULL, NULL);
            if (!IsWindow(hwnd_2)) {
                pthread_create(&render, NULL, &Render_Thread, NULL);

                SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

                DESTROY_WINDOW = 1; pthread_join(render, NULL); DESTROY_WINDOW = 0;
            }
            else {
                SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            }
        }   break;

        case WM_RBUTTONDOWN: {
            if (!IsWindow(hwnd_2)) {
                wglMakeCurrent(NULL, NULL);
                pthread_create(&render, NULL, Render_Thread, NULL);

                hMenu = CreatePopupMenu();

                GetCursorPos(&p);

                sprintf(str_size, "Scale (%.0f%%)", size * 100);

                AppendMenu(hMenu, MF_STRING, NULL, APP_NAME);
                AppendMenu(hMenu, MF_SEPARATOR, NULL, NULL);
                AppendMenu(hMenu, MF_STRING, 1, "Change GIF");
                AppendMenu(hMenu, MF_STRING, 2, str_size);
                AppendMenu(hMenu, MF_STRING | (TASKBAR) ? MF_CHECKED : 0, 3, "Show taskbar icon");
                AppendMenu(hMenu, MF_STRING | (TOPMOST) ? MF_CHECKED : 0, 4, "Always on top");
                AppendMenu(hMenu, MF_STRING, 5, "Exit");

                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);

                DestroyMenu(hMenu);

                DESTROY_WINDOW = 1; pthread_join(render, NULL); DESTROY_WINDOW = 0;
            }
        }   break;

        case WM_COMMAND: {
            switch (wParam) {
                /** Change GIF **/
                case 1: {
                    OPENFILENAME ofn;

                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = NULL;
                    ofn.lpstrFile = filename;
                    ofn.lpstrFile[0] = '\0';
                    ofn.lpstrFilter = "GIF Files (*.gif)\0*.gif\0All Files (*.*)\0*.*\0";
                    ofn.nMaxFile = sizeof(filename);
                    ofn.lpstrTitle = "Select GIF-file";
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                    if (TOPMOST) SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);
                    WAITING = 1;

                    if (GetOpenFileName(&ofn)) {
                        width = 0; height = 0;
                        if (CheckExtension((char const *)filename)) {
                            WriteSettings(filename, size, TASKBAR, TOPMOST);

                            GetWindowRect(hwnd, &rect);
                            DisableOpenGL(hwnd, hdc, hRC);
                            DestroyWindow(hwnd);
                            fc = 0; k = 0;

                            hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST,
                                                  "Window",
                                                  APP_NAME,
                                                  WS_POPUP | WS_VISIBLE,
                                                  rect.left,
                                                  rect.top,
                                                  width * size,
                                                  height * size,
                                                  NULL,
                                                  NULL,
                                                  hInstance,
                                                  NULL);

                            exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

                            if (!TASKBAR) exStyle |= WS_EX_TOOLWINDOW;
                            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

                            SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW);

                            SetLayeredWindowAttributes(hwnd, 0x0, 0, LWA_COLORKEY);

                            ShowWindow(hwnd, SW_SHOWDEFAULT);

                            EnableOpenGL(hwnd, &hdc, &hRC);

                            free(textures);
                            LoadTextures((char const *)filename, 0);
                            if (TOPMOST) SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);
                            else SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);
                        }
                        else MessageBox(NULL, "This file is not a GIF-animation", APP_NAME, MB_ICONEXCLAMATION);
                    }
                    else ReadSettings(0);
                    WAITING = 0;
                    ZeroMemory(&ofn, sizeof(ofn));
                }   break;

                /** Scale (%.0f%%) **/
                case 2: {
                    GetCursorPos(&p);
                    if (!RegisterClassEx(&wcex_2));

                    GetWindowRect(hwnd, &rect);
                    DisableOpenGL(hwnd, hdc, hRC);
                    DestroyWindow(hwnd);
                    fc = 0;

                    hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST,
                                          "Window",
                                          APP_NAME,
                                          WS_POPUP | WS_VISIBLE,
                                          rect.left,
                                          rect.top,
                                          width * 2,
                                          height * 2,
                                          NULL,
                                          NULL,
                                          hInstance,
                                          NULL);

                    texCoord[1] = 2 / size;
                    texCoord[2] = 2 / size;
                    texCoord[3] = 2 / size;
                    texCoord[4] = 2 / size;

                    exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

                    if (!TASKBAR) exStyle |= WS_EX_TOOLWINDOW;
                    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

                    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW);

                    SetLayeredWindowAttributes(hwnd, 0x0, 0, LWA_COLORKEY);

                    ShowWindow(hwnd, SW_SHOWDEFAULT);

                    EnableOpenGL(hwnd, &hdc, &hRC);

                    free(textures);
                    LoadTextures((char const *)filename, 1);

                    wglMakeCurrent(NULL, NULL);
                    pthread_create(&render, NULL, &Render_Thread, NULL);

                    WAITING = 1;

                    hwnd_2 = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                                            "Window_2",
                                            APP_NAME,
                                            WS_POPUP,
                                            p.x - 20,
                                            p.y - 20,
                                            164,
                                            73,
                                            NULL,
                                            NULL,
                                            hInstance,
                                            NULL);

                    hTrackbar = CreateWindowEx(0,
                                               TRACKBAR_CLASS,
                                               NULL,
                                               WS_CHILD | WS_VISIBLE | TBS_NOTICKS | TBS_TOOLTIPS | TBS_BOTH,
                                               -1,
                                               5,
                                               166,
                                               24,
                                               hwnd_2,
                                               NULL,
                                               NULL,
                                               NULL);

                    hButton = CreateWindowEx(0,
                                             "BUTTON",
                                             "Button",
                                             WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                                             4,
                                             42,
                                             156,
                                             25,
                                             hwnd_2,
                                             (HMENU)1,
                                             NULL,
                                             NULL);

                    hFont = CreateFont(16,
                                       0,
                                       0,
                                       0,
                                       FW_NORMAL,
                                       FALSE,
                                       FALSE,
                                       FALSE,
                                       DEFAULT_CHARSET,
                                       OUT_DEFAULT_PRECIS,
                                       CLIP_DEFAULT_PRECIS,
                                       DEFAULT_QUALITY,
                                       DEFAULT_QUALITY,
                                       "Segoe UI");

                    SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);

                    SendMessage(hTrackbar, TBM_SETRANGE, TRUE, MAKELONG(1, 200));
                    SendMessage(hTrackbar, TBM_SETPOS, TRUE, size * 100);
                    SendMessage(hTrackbar, TBM_SETTIC, 100, 0);

                    hRgn = CreateRoundRectRgn(0, 0, 164, 73, 5, 5);
                    SetWindowRgn(hwnd_2, hRgn, TRUE);

                    SetLayeredWindowAttributes(hwnd_2, 0x0, 0, LWA_COLORKEY);
                    ShowWindow(hwnd_2, SW_SHOWDEFAULT);

                    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);

                    while (GetMessage(&msg_2, NULL, 0, 0)) {
                        TranslateMessage(&msg_2);
                        DispatchMessage(&msg_2);
                    }

                    DestroyWindow(hwnd_2);

                    WriteSettings(filename, size, TASKBAR, TOPMOST);

                    WAITING = 0;
                    DESTROY_WINDOW = 1; pthread_join(render, NULL); DESTROY_WINDOW = 0;
                    GetWindowRect(hwnd, &rect);

                    DisableOpenGL(hwnd, hdc, hRC);
                    DestroyWindow(hwnd);
                    fc = 0;

                    hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST,
                                          "Window",
                                          APP_NAME,
                                          WS_POPUP | WS_VISIBLE,
                                          rect.left,
                                          rect.top,
                                          (width * size < 10.0) ? 10.0 : width * size,
                                          (height * size < 10.0) ? 10.0 : height * size,
                                          NULL,
                                          NULL,
                                          hInstance,
                                          NULL);

                    texCoord[1] = 1;
                    texCoord[2] = 1;
                    texCoord[3] = 1;
                    texCoord[4] = 1;

                    exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

                    if (!TASKBAR) exStyle |= WS_EX_TOOLWINDOW;
                    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

                    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW);

                    SetLayeredWindowAttributes(hwnd, 0x0, 0, LWA_COLORKEY);

                    ShowWindow(hwnd, SW_SHOWDEFAULT);

                    EnableOpenGL(hwnd, &hdc, &hRC);

                    free(textures);
                    LoadTextures((char const *)filename, 0);

                    if (TOPMOST) SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);
                    else SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);
                }   break;

                /** Show taskbar icon **/
                case 3: {
                    exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

                    if (TASKBAR) {
                        exStyle |= WS_EX_TOOLWINDOW;
                        TASKBAR = 0;
                    }
                    else {
                        exStyle &= ~WS_EX_TOOLWINDOW;
                        TASKBAR = 1;
                    }

                    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
                    WriteSettings(filename, size, TASKBAR, TOPMOST);
                    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW);
                } break;

                /** Always on top **/
                case 4: {
                    if (TOPMOST) {
                        TOPMOST = 0; SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);
                    }
                    else {
                        TOPMOST = 1; SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);
                    }
                    WriteSettings(filename, size, TASKBAR, TOPMOST);
                } break;

                /** Exit **/
                case 5: DESTROY_WINDOW = 1; break;

            }   break;
        }
        default: return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

/**
      Реагировать на сообщения от Windows
**/

LRESULT CALLBACK WindowProc_2(HWND hwnd_2, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_QUIT: {
            PostQuitMessage(0);
        } break;

        case WM_CLOSE: {
            PostQuitMessage(0);
        } break;

        case WM_SETFOCUS: SetFocus(hwnd_2); break;

        case WM_LBUTTONDOWN: {
            SendMessage(hwnd_2, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }   break;

        case WM_HSCROLL: {
            SetFocus(hwnd_2);
            int pos = SendMessage(hTrackbar, TBM_GETPOS, 0, 0);
            size = (float)pos / 100;
            texCoord[1] = 2 / size;
            texCoord[2] = 2 / size;
            texCoord[3] = 2 / size;
            texCoord[4] = 2 / size;
            ShowFrame(k);
        }   break;

        case WM_SETCURSOR: {
            GetCursorPos(&p);
            ScreenToClient(hwnd, &p);
            RECT rect;
            GetWindowRect(hButton, &rect);

            if (PtInRect(&rect, p)) {
                if (!HOVERED) { HOVERED = 1; InvalidateRect(hButton, NULL, TRUE); }
            }
            else if (PtInRect(&rect, p)) {
                if (HOVERED) { HOVERED = 0; InvalidateRect(hButton, NULL, TRUE); }
            }
            else { HOVERED = 0; InvalidateRect(hButton, NULL, TRUE); }
        }

        case WM_PAINT: {
            hdc_2 = BeginPaint(hwnd_2, &ps);

            /** Рисует фон окна **/
            HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));
            FillRect(hdc_2, &ps.rcPaint, hBrush);

            /** Рисует делитель между трекбарем и кнопкой **/
            RECT rect = {4, 37, 160, 38};

            hBrush = CreateSolidBrush(RGB(210, 210, 210));
            FillRect(hdc_2, &rect, hBrush);

            DeleteObject(hBrush);
            EndPaint(hwnd_2, &ps);
        }   break;

        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT drawitem = (LPDRAWITEMSTRUCT)lParam;
            hdc_b = drawitem->hDC;
            rect = drawitem->rcItem;

            /** Рисует кнопку **/
            SetBkMode(hdc_b, TRANSPARENT);
            HBRUSH hBrush = CreateSolidBrush(HOVERED ? RGB(230, 230, 230) : RGB(240, 240, 240));
            FillRect(hdc_b, &rect, hBrush);
            DrawText(hdc_b, "           Save scale", -1, &rect, DT_VCENTER | DT_SINGLELINE);

            DeleteObject(hBrush);
        }   break;

        case WM_COMMAND: {
            switch (wParam) {
                case 1: SendMessage(hwnd_2, WM_CLOSE, 0, 0);
            }
        }

        default: return DefWindowProc(hwnd_2, uMsg, wParam, lParam);
    }
}

/**
        Включает OpenGL для текущего окна
**/

void EnableOpenGL(HWND hwnd, HDC* hdc, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    *hdc = GetDC(hwnd);

    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hdc, &pfd);

    SetPixelFormat(*hdc, iFormat, &pfd);

    *hRC = wglCreateContext(*hdc);

    wglMakeCurrent(*hdc, *hRC);
}

/**
        Выключает OpenGL для текущего окна
**/

void DisableOpenGL (HWND hwnd, HDC hdc, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hdc);
}


#include "opengl_proc.h"
#include "loadgif.h"
#include "settings.h"
#include "window_proc.h"

float vertex[] = {-1, -1, 0,
                   1, -1, 0,
                   1,  1, 0,
                  -1,  1, 0};
float texCoord[] = {0, 1,
                    1, 1,
                    1, 0,
                    0, 0};

int k = 0;
int DRAWING = 0;

struct timeval t_start, t_current;
double start, current;

/**

        gettimeofday

**/

void gettimeofday(struct timeval* tp, void* tzp) {
    (void)tzp; // Не используется
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    // Преобразование в микросекунды (начиная с 1601-01-01)
    uli.QuadPart /= 10;
    // Вычитаем смещение эпохи (1970-01-01)
    uli.QuadPart -= 11644473600000000ULL;

    tp->tv_sec = (long)(uli.QuadPart / 1000000);
    tp->tv_usec = (long)(uli.QuadPart % 1000000);
}

/**
        EnableOpenGL
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
        DisableOpenGL
**/

void DisableOpenGL (HWND hwnd, HDC hdc, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hdc);
}


/**
        ShowFrame
**/

void ShowFrame(int k)
{
    DRAWING = 1;
    wglMakeCurrent(hdc, hRC);

    glViewport(0, 0, (width * size < 10.0) ? 10.0 : width * size, (height * size < 10.0) ? 10.0 : height * size);
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
    DRAWING = 0;
}

/**
        RenderThread
**/

void* RenderThread(void *arg) {
    wglMakeCurrent(hdc, hRC);
    while (!DESTROY_WINDOW) {
        gettimeofday(&t_start, NULL);
        start = t_start.tv_sec + t_start.tv_usec / 1e6;

        if (!SETTING_POS) ShowFrame(k);

        gettimeofday(&t_current, NULL);
        current = t_current.tv_sec + t_current.tv_usec / 1e6;

        while (current < start + *(delays + k) - 0.002) {
            if (current + 0.015 < start + *(delays + k)) Sleep(10);
            gettimeofday(&t_current, NULL); current = t_current.tv_sec + t_current.tv_usec / 1e6;
        }

        if (k >= fc - 1) k = 0;
        else k++;
    }
    wglMakeCurrent(NULL, NULL);
    return NULL;
}

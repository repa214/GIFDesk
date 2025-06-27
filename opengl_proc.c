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
float inaccuracy = 0;
double start_time = 0, current_time = 0;

int CollisionWidth() { return (width * size < 10.0) ? 10.0 : width * size + size; }
int CollisionHeight() { return (height * size < 10.0) ? 10.0 : height * size + size; }

/**
        Vsleep
**/

void VSleep(double s) {
    current_time = GetTime();

    while (current_time < start_time + s + inaccuracy) {
        if (DESTROY_WINDOW) return;
        if (current_time + 0.002 < start_time + s + inaccuracy) Sleep(1);
        current_time = GetTime();
    }

    inaccuracy += s - (current_time - start_time);
    if ((inaccuracy < 0 ? -inaccuracy : inaccuracy) > s * 10) inaccuracy = 0;

    // printf("%.2f | % 2.5f\n", s, inaccuracy);

    start_time = GetTime();
}


/**
        GetTime
**/

double GetTime() {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / freq.QuadPart;
}

/**
        EnableOpenGL
**/

void EnableOpenGL(HWND hwnd, HDC* hdc, HGLRC* hRC) {
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

void DisableOpenGL (HWND hwnd, HDC hdc, HGLRC hRC) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hdc);
}


/**
        ShowFrame
**/

void ShowFrame(int k) {
    DRAWING = 1;
    printf("%d |", k);
    wglMakeCurrent(hdc, hRC);

    glViewport(0, 0, CollisionWidth(), CollisionHeight());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0, 0, 0, 0); glClear(GL_COLOR_BUFFER_BIT);

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
    while (!DESTROY_WINDOW) {
        ShowFrame(k); VSleep(*(delays + k));

        if (k >= fc - 1) k = 0;
        else k++;
    }
    return NULL;
}

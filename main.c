/**


        0.50: ������ �����
        0.51: ���������� ������ ������������ ������ ��� �������� �������� � ������� �����������
        0.52: ����������� �������� ������������� � ����������
        0.53: ���������� ����������� �������� � ������� ��������� Frame Blending Mode (1)
        0.54: ���������� ����������� �������� � ������� ��������� Frame Blending Mode (2)
        0.55: ��������� ����������� ���������� ��� �������� ���� ���������� � ������ �����
        0.56: ���������� ����������� �������� � ������� ��������� Frame Blending Mode (3)
        0.57: �������� �������� ��������� ���������������� ����� �����
        0.58: ��������� �������� ��������� ���������������� ����� �����
        0.59: ������������� ��������� ������ ��������

        0.60: ��������� ����������� �������������� �����������
        0.61: ���������� ����������� �������� � ������� ��������� Frame Blending Mode (4)
        0.62: ���������� ������ �������� ���� � ��������
        0.63: ��� ��������� �� ���-���� ����������� ��� ����������� �������
        0.64: ��������� ��� ��� ������� �� �������� ���-���� ����� ����������� � ���������
        0.65: ���������� ������ ���������� �������� �����������
        0.66: ��������� ��� ���������� �����������, ���� �������� ����� GIF-�����, � ����� �������� ��� �������
        0.67: ���������� ��������� ��������� ���� �� ����� ��������� ��������
        0.68: �������� ������������ ����������� �� ����� ��������� �������� (beta)
        0.69: ��������� ������������ � �������� � �������� ��������� �����

        0.70: ������ ������� ��������� �������� ����
        0.71: ���������� ��������� ������� ��������� ��������
        0.72: ��������� ����������� ��������� ���� �� ���� �������� �����
        0.73: ��������� ������ � ���-���� ����������
        0.74: ��������� ������������ ���������� ����� ������ �����
        0.75: ��������� ����������� �������� �������� ���� �������������� ����� � ����
        0.76: ������ ������������� ������������ �������� �� ����� � ���������������
        0.77: ���� ��� ��������������� �������� �� ������� �� ������� ������
        0.78: ���������� ��������� �������� ��� ��������� �������� ��������
        0.79: ��������� ������������������ �� ������, ���� �������� �������� ������� �� ����� ������

        0.80: ��������� ����������� ������� �������� ��������
        0.81: ���������� ������ ������ �������������� ���������� � ����� ��������
        0.82: ��������� ��������� �������� �����
        0.83: ��������� ������������ �����, ������� ���������� �������, ��� ������ ������� �������
        0.84: ���������� ������ ������ � ��������� delays ��� ��������
        0.85: ���������� ������ ������ � ��������� ������ ��� �������
        0.86: ���������� ����� ������ ��������� � ����
        0.87: �������� ������� ���������� ��������� ��������
        0.88: ����� �������� �������� �� 1000%
        0.89: ���������� �������� ������ "���������"
        0.90: ��������� ������� �� �������
        0.91: �������� ����� �� ���� �������� Escape
        0.92: ��������� ������������ �������� �� ����� ������� �� ������ +
        0.93: �������������� ��������� �������� �����������
        0.94: ���������� ������ ��������� ������� � ������ ����������� ����� � �����������
        0.95: ���������� ��������� ���� �� ����� ������� ��������
        0.96: ���� ��� �������� ��������� �� Enter, �� ��������� �����/����, �� Num 8/2
        0.97: �������������� �������� ��������
        0.98: ��������� ������� �� �������
        0.99: ������� �������� �� �� �� ����� ��������������� ��������
        0.100: ������� ����������� �������� ��������������� ��������

        �����:
        - �������� ��������������, ��� 200+% ����� ������ �������� �� ������������������

        - ��������� ��������� �������� ��� �������������� �������

        - ���� ������ �������� ���������� �� delay

        - �������� ��������� WEBP, APNG, MNG, AVIF


**/

#include "gifdesk.h"

/**

    Main window function

**/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    SetCursor(LoadCursor(NULL, IDC_APPSTARTING));

    setlocale(LC_ALL, "Russian");

    WcexInit(&wcex, "Window", (WNDPROC)WindowProc);
    WcexInit(&wcex_2, "Window_2", (WNDPROC)WindowProc_2);

    if (!RegisterClassEx(&wcex)) return 0;
    if (!RegisterClassEx(&wcex_2)) return 0;

    strcpy(settings_path, GetSettingsPath());

    if (!ReadSettings(0)) { return 0; }

    filetype = CheckFile((const char*)filename);
    if (!filetype) { if (!ReadSettings(1)) { return 0; } }

    printf("filename: %s | size: %.2f | taskbar: %d | topmost: %d | langgif: %d\n", filename, size, TASKBAR, TOPMOST, LANGGIF);

    hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST,
                          "Window",
                          APP_NAME,
                          WS_POPUP | WS_VISIBLE,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CollisionHeight(),
                          CollisionWidth(),
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

    if (!TASKBAR) exStyle |= WS_EX_TOOLWINDOW;
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

    SetWindowPos(hwnd, (TOPMOST) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOSIZE);

    SetLayeredWindowAttributes(hwnd, 0x0, 0, LWA_COLORKEY);

    GetApplicationIcon();

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    DragAcceptFiles(hwnd, TRUE);
    EnableOpenGL(hwnd, &hdc, &hRC);
    LoadFile((char const *)filename, GIF_FORMAT);

    SystemParametersInfo(SPI_GETWORKAREA, 0, &res, 0);

    if (width * size > res.right - res.left) size = ((float)res.right - (float)res.left) / (float)width;
    if (height * size > res.bottom - res.top) size = ((float)res.bottom - (float)res.top) / (float)height;

    SetWindowPos(hwnd,
                 (TOPMOST) ? HWND_TOPMOST : HWND_NOTOPMOST,
                 0,
                 0,
                 CollisionWidth(),
                 CollisionHeight(),
                 SWP_NOMOVE);

    SetCursor(LoadCursor(NULL, IDC_ARROW));

    const GLubyte* version = glGetString(GL_VERSION);
    printf("OpenGL Version: %s\n", version);

    start = GetTime();
    start_animating = GetTime();
    while (1) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); }
        else if (DESTROY_WINDOW) break;
        else {
            ShowFrame(k); VSleep(*(delays + k));

            if (k >= fc - 1) k = 0;
            else k++;
        }
    }

    pthread_join(render, NULL);
    DisableOpenGL(hwnd, hdc, hRC);
    DestroyWindow(hwnd);
    DeleteObject(appIcon);
    if (delays != NULL) { free(delays); delays = NULL; }
    if (textures != NULL) { free(textures); textures = NULL; }

    return 0;
}

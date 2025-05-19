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

        0.70: изменён принцип изменения масштаба окна
        0.71: исправлено поведение полоски изменения масштаба
        0.72: добавлена возможность поместить окно на края рабочего стола
        0.73: добавлена иконка в ПКМ-меню приложения
        0.74: добавлено реагирование приложения через курсор мышки
        0.75: добавлена возможность поменять анимацию путём перетаскивания файла в окно
        0.76: сильно оптимизирован предпросмотр анимации во время её масштабирования
        0.77: окно для масштабирования анимации не выходит за пределы экрана
        0.78: исправлено поведение ползунка для изменения масштаба анимации


        Планы:
        - исправить, чтобы окно Scale не выходило за рамки экрана
        - изменить предел Scale до 1000%
        - добавить возможность менять режим отображения (GL_LINEAR или GL_NEAREST)
        - добавить поддержку нескольких языков
        - добавить полоску загрузки анимации
        - добавить автомасштабирование на случай если загружаемая анимация выходит за пределы экрана
        - добавить поддержку WEBP, APNG, MNG, AVIF, JXL; пакета из PNG, JPG


**/

#include "gifdesk.h"
#include "resource.h"

/**

    Main window function

**/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    SetCursor(LoadCursor(NULL, IDC_APPSTARTING));

    setlocale(LC_ALL, "Russian");

    WcexInit(&wcex, "Window", WindowProc);
    WcexInit(&wcex_2, "Window_2", WindowProc_2);

    if (!RegisterClassEx(&wcex)) return 0;
    if (!RegisterClassEx(&wcex_2)) return 0;

    strcpy(settings_path, GetSettingsPath());

    if (!ReadSettings(0)) { return 0; }

    CheckExtension((const char*)filename, 1);
    // printf("filename: %s\nsize: %.2f TASKBAR: %d TOPMOST: %d\n", filename, size, TASKBAR, TOPMOST);

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

    GetApplicationIcon();

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    DragAcceptFiles(hwnd, TRUE);
    EnableOpenGL(hwnd, &hdc, &hRC);
    LoadTextures((char const *)filename, 0);

    SetCursor(LoadCursor(NULL, IDC_ARROW));

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

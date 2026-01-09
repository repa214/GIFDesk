#include <windows.h>
#include "gifdesk.h"

/**

        Совместимость:
            Windows Vista
            OpenGL 1.1+ (1.2+ recommended)

        1.2:

            - Оптимизирован принцип изменения размера окна
            - Исправлен критический баг поломки интерфейса утилиты, если долго держать включенным Playback
            - Исправлено отображение границ обновления кадра во время изменения размера окна
            - Исправлено дёрганое появление всплывающих окон настройки

            (нужно) переделать счётчик кадров. он хоть и не потребляет ресурсы, но дико тормозит
            (нужно) сохранение профилей
            (нужно) добавить управление через tray icon
            (не очень нужно) добавить поддержку MNG
            (не очень нужно) добавить поддержку MP4

**/

//int WINAPI WinMain(HINSTANCE hInstance,
//                   HINSTANCE hPrevInstance,
//                   LPSTR lpCmdLine,
//                   int nCmdShow)
int main()
{
    setlocale(LC_ALL, "Russian");

    GIFDesk app;
    int c = Run(&app);

    return c;
}

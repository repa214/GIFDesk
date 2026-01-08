#include <windows.h>
#include "gifdesk.h"

/**

        Совместимость:
            Windows Vista
            OpenGL 1.1+ (1.2+ recommended)

        1.2b:

            - Добавлен интерактив
            - Исправлена возможность создавать несколько всплывающих окон
            - Исправлено зависание окна при открытии настроек
            - Исправлено произвольное перемещение окна при открытии и закрытии меню настроек
            - Исправлено сохранение положения окна после установки его посередине
            - Исправлена запись размера окна во время его изменения

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

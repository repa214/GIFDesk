#include <windows.h>
#include "gifdesk.h"

/**

        Совместимость:
            Windows Vista
            OpenGL 1.1 (1.2 recommended)

        1.2.2:

            - Исправлено отображение размера окна при изменении через кнопки
            - Исправлен редкий баг пустого окна при первоначальной загрузке утилиты

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

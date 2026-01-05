#include <windows.h>
#include "gifdesk.h"

/**

        Совместимость:
            Windows Vista
            OpenGL 1.1+ (1.2+ recommended)

        1.1.5:

            - Исправлено сохранение расположение окна в файле settings
            - Испралена ситуация, при которой угол, от которого меняется размер, не сохранялся после применения "Move window to"

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

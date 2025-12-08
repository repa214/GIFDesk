#include <windows.h>
#include "gifdesk.h"

/**
        PopupMenu мечты:
            GIFDesk 1.1
            ------------------------
            Open file...
            Scale                  >
                                      [1-1000%]       +  -
                                      |------------------|
            Playback controls      >
                                      Frame: 1/39
                                        |<     ||     >|
                                      |------------------|
                                      Speed: [1-1000%]
                                        |<     ||     >|
                                      |------------------|
                                      Transparency: [1-100%]
                                      |------------------|
                                      Show frame updates (GIF, PNG)
            Interaction Mode       >
                                      Control via system tray icon
                                      Ignore all input (except Esc)
            Pin window             >
                                      Default
                                      Always on top
                                      Always on bottom
            Move window to         >
                                      Top left corner
                                      Top right corner
                                      Center
                                      Bottom left corner
                                      Bottom right corner
            ------------------------
            Close window

        - Preview available up to 200% for performance
        - High values may slow down playback


        Совместимость:
            Windows Vista
            OpenGL 1.1 и более

        1.0:

            исправлена работа кнопки "Show taskbar icon"
            исправлено перетаскивание окна зажатием ЛКМ после первого запуска утилиты

            (нужно) сохранение профилей
            (нужно) добавить скорость воспроизведения анимации
            (нужно) добавить управление через скрытые значки
            (нужно) добавлена возможность настраивать прозрачность окна
            (нужно) оптимизирован предпросмотр окна при выставлении размера меньше 10х10 пикселей
            (нужно) предпросмотр размера меняется вдоль середины
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

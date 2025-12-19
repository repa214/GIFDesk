#include <windows.h>
#include "gifdesk.h"

/**
        PopupMenu:
            GIFDesk 1.0.x
            ------------------------
            Open file...

            Playback               >
                                      Frame: 1/39
                                           |<     ||     >|
                                      |-----------------------|

                                      Speed: 1x
                                           ||<  |<  >|  >||
                                      |-----------------------|

                                      Show frame updates (GIF, PNG)

            Interaction            >
                                      Enable Drag n` Drop
                                      Only tray icon (disable RMB)
                                      Ignore all input (except Esc)

            Window                 >
                                      Scale: 100%          +  -
                                      |-----------------------|

                                      Transparency: 100%   +  -
                                      |-----------------------|

                                      Always on top

                                      Show taskbar icon

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


        —овместимость:
            Windows Vista
            OpenGL 1.1 и более

        1.0.3:

            - ƒоработан интерфейс регулировки воспроизведени€
            - ƒоработано позицирование всплывающих окон при нестандартном положении окна

        1.0.4:

            - ¬озращено автомасштабирование, если анимаци€ не вмещаетс€ в окно

            (нужно) сохран€ть позицию окна в файле settings
            (нужно) исправить автоположение всплывающих окон
            (нужно) сохранение профилей
            (нужно) добавить скорость воспроизведени€ анимации
            (нужно) добавить управление через tray icon
            (нужно) добавлена возможность настраивать прозрачность окна
            (нужно) оптимизирован предпросмотр окна при выставлении размера меньше 10х10 пикселей
            (нужно) предпросмотр размера мен€етс€ вдоль середины
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

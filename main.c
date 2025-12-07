#include <windows.h>
#include "gifdesk.h"

/**
        PopupMenu:
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


        —овместимость:
            Windows Vista
            OpenGL 1.1 и более

        1.0 (release-candidate_1):
             изменена анимаци€ загрузки
             добавлена обработка известных ошибок от декодера
             добавлен интерактив при выборе файла
             добавлена поддержка interlaced GIF
             поток рендера отдел€етс€ от основного только при необходимости
             утилита исправно показывает область обновлени€ кадра
             границы обновлени€ кадра корректно работают с измен€ющимс€ Scale
             исправлен вылет по нажатию случайной клавиши
             декодер теперь исправно читает GIF-файлы
             оптимизирована загрузка данных с файла
             оптимизирована проверка файлов на соответствие формата
             изменЄн стиль всплывающего меню с настройками
             исправлен недочЄт зависани€ окна при быстром кликании мышки
             оптимизировано хранение анимации путЄм очистки буфера после синхронизации с VRAM

             добавлена полоска дл€ регулировани€ кадров анимации
             добавлено возможность поставить окно в центр

             убрана поддержка нескольких €зыков

             (нужно) сохранение профилей
             (нужно) добавить скорость воспроизведени€ анимации
             (нужно) добавить управление через скрытые значки
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

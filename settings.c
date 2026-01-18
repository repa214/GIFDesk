#include "settings.h"


/// Gets the path to the "settings" file

/// BYTE CONFIG: filename, size,  x,   y,   speed, transparency, taskbar, topmost, lang, ignore_input, hide_on_hover
//               char,     float, int, int, byte,  byte,         byte,    byte,    byte  byte          byte
/// returns 0 if settings file is invalid
/// returns 1 if settings file is valid
void GetSettingsPath(Settings* st)
{
    /**
            char              : FILENAME
            size              : [0.01 ... 2] (2.01 ... 10)
            x                 : int
            y                 : int
            speed             : [1 ... 200]  * 0.05
            transparency      : [1 ... 255]
            language          : [ ??? ]

            flags             : [0 ... 255]
                0b10000000    : show_taskbar
                0b01000000    : topmost
                0b00100000    : ignore_input
                0b00010000    : hide_on_hover
                0b00001000    : click_through
                0b00000100    : disable_moving
                0b00000010    : show_tray_icon

    **/

    st->size = 1;
    st->x = 0;
    st->y = 0;
    st->speed = 20;
    st->transparency = 255;
    st->lang = 255;
    st->flags = 0xC2;
    st->sfu = 0;

    const char filter[] =
        "Animations (*.gif, *.webp, *.png, *.avif)\0*.gif; *.webp; *.png; *.apng; *.avif; *.avifs\0"
        "GIF (*.gif)\0*.gif\0"
        "WEBP (*.webp)\0*.webp\0"
        "APNG (*.png; *.apng)\0*.png; *.apng\0"
        "AVIF (*.avif; *.avifs)\0*.avif; *.avifs\0"
        "All files (*.*)\0*.*\0";
    memcpy((void *)st->ofnfilter, filter, sizeof(filter));

    static char str[MAX_PATH] = "";
    GetModuleFileName(NULL, str, sizeof(str));
    char* ls = strrchr(str, '\\');
    if (ls) *(ls + 1) = '\0';
    strcat(str, "settings");
    strcpy(st->settings_path, str);
}

void* _LoadSettings(void* arg)
{
    RenderPtr* rptr = (RenderPtr*)arg;

    _LoadDropFile(NULL, rptr->window, rptr->st, rptr->dt, rptr->rd);

    return 0;
}

void _LoadDropFile(HDROP drop, Window* window, Settings* st, Data* dt, Render* rd)
{
    window->wcex.lpfnWndProc = EscapeWindowProc;

    if (drop != NULL) {
        DragQueryFile(drop, 0, st->buff_filename, MAX_PATH);
        DragFinish(drop);
    }

//    rd->change_frames = 0;

    uint8_t filetype = 0x00; do
    {
        if (drop != NULL)
            filetype = CheckFile((const char*)st->buff_filename, dt);
        else {
            filetype = SetSettings(window, st, dt);
            drop = NULL;
        }

        if (filetype == 0xFF) { /// User didn`t select file
            strcpy(st->buff_filename, "");
            break;
        }

        if (filetype == 0x00) { /// Header file is invalid
            drop = NULL;
            dt->error = 255;
            if (DialogBox(GetModuleHandle(NULL), (LPCTSTR)IDD_DIALOG_BCANCEL, window->hwnd, (DLGPROC)DlgProc))
                continue;
            else {
                strcpy(st->buff_filename, "");
                break;
            }
        }

        /// Clearing media before loading
        ClearMedia(dt);

        /// Checking selected file
        rd->loading = 1;
        uint8_t valid = LoadFile(window, st, dt, filetype); dt->error = valid;
        if (valid == 0) {
            SwapFilenames(st); strcpy(st->buff_filename, ""); WriteSettings(st);
            break;
        }

        /// If file is not valid
        if (DialogBoxParam(GetModuleHandle(NULL), (LPCTSTR)IDD_DIALOG_BCANCEL_E, NULL, (DLGPROC)DlgProc, 0)) {
            drop = NULL;
            continue;
        }
        else {
            drop = NULL;
            filetype = CheckFile((const char*)st->filename, dt);
            if (filetype == 0) continue;

            valid = LoadFile(window, st, dt, filetype);
            if (valid == 0) {
                strcpy(st->buff_filename, ""); WriteSettings(st);
                break;
            }
        }

    }   while (1);

    ReloadWindow(window, st, dt, 1); rd->loading = 0; rd->start_time = GetTime(); rd->change_frames = 1;
    window->wcex.lpfnWndProc = MainWindowProc;
}

/// returns 0 if settings file is invalid
/// returns 1 if settings file is valid
uint8_t GetSettings(Settings* st)
{
    FILE *f = fopen(st->settings_path, "rb");
    if (f != NULL)
    {
        if (fread(st->filename, sizeof(char), 261, f) < 261) goto nofile;
        if (fread(&st->size, sizeof(float), 1, f) < 1) goto nofile;
        st->trackbar_size = st->size;
        if (fread(&st->x, sizeof(uint32_t), 1, f) < 1) goto nofile;
        if (fread(&st->y, sizeof(uint32_t), 1, f) < 1) goto nofile;
        if (fread(&st->speed, sizeof(uint8_t), 1, f) < 1) goto nofile;
        if (fread(&st->transparency, sizeof(uint8_t), 1, f) < 1) goto nofile;
        if (fread(&st->lang, sizeof(uint8_t), 1, f) < 1) goto nofile;
        if (fread(&st->flags, sizeof(uint8_t), 1, f) < 1) goto nofile;
    }
    else goto nofile;

    fclose(f); return 1;
nofile:
    fclose(f); return 0;
}

/// spawns GetOpenFileName
/// returns 0 if file is setted and invalid
/// returns 1-5 if file is setted and valid
/// returns 255 if user cancelled select
uint8_t SetSettings(Window* window, Settings* st, Data* dt)
{
    memset(&st->ofn, 0, sizeof(OPENFILENAME));
    st->ofn.lStructSize = sizeof(OPENFILENAME);
    st->ofn.hwndOwner = NULL;
    st->ofn.lpstrFile = st->buff_filename;
    st->ofn.lpstrFile[0] = '\0';
    st->ofn.nMaxFile = MAX_PATH;
    st->ofn.lpstrFilter = st->ofnfilter;
    st->ofn.nFilterIndex = 1;
    st->ofn.nMaxFileTitle = 0;
    st->ofn.lpstrTitle = "Choose animation file";
    st->ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileName(&st->ofn)) {
        uint8_t filetype = CheckFile((const char*) st->buff_filename, dt);
        if (filetype) {
            return filetype;
        }
        else
            return 0x00;
    }
    else return 0xFF;
}

/// makes filename valid
void SwapFilenames(Settings* st)
{
    strncpy(st->filename, st->buff_filename, MAX_PATH);
}

void WriteSettings(Settings* st)
{
    FILE *f = fopen(st->settings_path, "wb");
    fwrite(st->filename, sizeof(char), 261, f);
    fwrite(&st->size, sizeof(float), 1, f);
    fwrite(&st->x, sizeof(uint32_t), 1, f);
    fwrite(&st->y, sizeof(uint32_t), 1, f);
    fwrite(&st->speed, sizeof(uint8_t), 1, f);
    fwrite(&st->transparency, sizeof(uint8_t), 1, f);
    fwrite(&st->lang, sizeof(uint8_t), 1, f);
    fwrite(&st->flags, sizeof(uint8_t), 1, f);

    fclose(f);
}






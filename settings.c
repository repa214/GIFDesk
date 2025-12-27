#include "settings.h"


/// Gets the path to the "settings" file
void GetSettingsPath(Settings* st)
{
    st->size = 1;
    st->taskbar = 1;
    st->topmost = 1;
    st->lang = 0;
    st->sfu = 0;
    st->transparency = 255;
    st->speed = 16;
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

DWORD WINAPI _LoadSettings(LPVOID arg)
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

    ReloadWindow(window, st, dt, 1); rd->loading = 0; rd->frame = 0; // rd->change_frames = 1;
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
        if (fread(&st->taskbar, sizeof(int), 1, f) < 1) goto nofile;
        if (fread(&st->topmost, sizeof(int), 1, f) < 1) goto nofile;
        if (fread(&st->lang, sizeof(int), 1, f) < 1) goto nofile;
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
    fwrite(&st->taskbar, sizeof(int), 1, f);
    fwrite(&st->topmost, sizeof(int), 1, f);
    fwrite(&st->lang, sizeof(int), 1, f);

    fclose(f);
}

void ShowSettings(Settings* st)
{
    printf("filename: %s\n", st->filename);
    printf("size: %f\n", st->size);
    printf("taskbar: %d\n", st->taskbar);
    printf("topmost: %d\n", st->topmost);
    printf("lang: %d\n", st->lang);
}







#include "settings.h"
#include "loadgif.h"
#include "language.h"

float size = 1;
int TASKBAR = 1;
int TOPMOST = 1;
int LANGGIF = 0;

char settings_path[MAX_PATH];
char filename[MAX_PATH];
char str_size[11];
const char OFNfilter[200] = "Анимированные изображения\0*.gif; *.webp; *.png; *.apng\0"
                            "GIF (*.gif)\0*.gif\0"
                            "WEBP (*.webp)\0*.webp\0"
                            "APNG (*.png; *.apng)\0*.png; *.apng\0"
                            "Все файлы (*.*)\0*.*\0";


/**
        GetSettingsPath
**/

char* GetSettingsPath() {
    static char str[MAX_PATH] = "";
    GetModuleFileName(NULL, str, sizeof(str));
    char* ls = strrchr(str, '\\');
    if (ls) *(ls + 1) = '\0';
    strcat(str, "settings");
    return str;
}

/**
        WriteSettings
**/

void WriteSettings(const char *filename, float size, int taskbar, int topmost, int lang)
{
    FILE *f = fopen(settings_path, "wb");
    fwrite(filename, sizeof(char), 261, f);
    fwrite(&size, sizeof(float), 1, f);
    fwrite(&taskbar, sizeof(int), 1, f);
    fwrite(&topmost, sizeof(int), 1, f);
    fwrite(&lang, sizeof(int), 1, f);

    fclose(f);
}

/**
        ReadSettings
**/

int ReadSettings(int fi)
{
    FILE *f = fopen(settings_path, "rb");
    if (f != NULL && !fi) {
        if (fread(filename, sizeof(char), 261, f) < 261) goto filename_init;
        if (fread(&size, sizeof(float), 1, f) < 1) goto filename_init;
        if (fread(&TASKBAR, sizeof(float), 1, f) < 1) goto filename_init;
        if (fread(&TOPMOST, sizeof(float), 1, f) < 1) goto filename_init;
        if (fread(&LANGGIF, sizeof(float), 1, f) < 1) goto filename_init;
    }
    else {
        goto filename_init;
    }
    fclose(f);
    return 1;

    filename_init:
        printf("filename_init\n");
        fclose(f);
        OPENFILENAME ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = filename;
        ofn.lpstrFile[0] = '\0';
        ofn.lpstrFilter = OFNfilter;
        ofn.nMaxFile = sizeof(filename);
        ofn.lpstrTitle = lang.selectGIF[LANGGIF];
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileName(&ofn)) {
            filetype = CheckFile((char const *)filename);
            if (filetype) {
                if (GetUserDefaultUILanguage() == 1049) LANGGIF = 1;
                WriteSettings(filename, size, TASKBAR, TOPMOST, LANGGIF);
                return 1;
            }
            else {
                MessageBox(NULL, lang.notGIF[LANGGIF], APP_NAME, MB_ICONEXCLAMATION);
                return 0;
            }
        }
        else return 0;
        return 0;
}

#include "settings.h"

float size = 1;
int TASKBAR = 1;
int TOPMOST = 1;
int LANGGIF = 0;

char settings_path[MAX_PATH];
char filename[MAX_PATH];
char str_size[11];

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

int WriteSettings(const char *filename, float size, int taskbar, int topmost, int lang)
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
        fclose(f);
        OPENFILENAME ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = filename;
        ofn.lpstrFile[0] = '\0';
        ofn.lpstrFilter = "GIF Files (*.gif)\0*.gif\0All Files (*.*)\0*.*\0";
        ofn.nMaxFile = sizeof(filename);
        ofn.lpstrTitle = "Select a GIF file to display";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileName(&ofn)) {
            if (CheckExtension((char const *)filename)) {
                if (GetUserDefaultUILanguage() == 1049) LANGGIF = 1;
                WriteSettings(filename, size, TASKBAR, TOPMOST, LANGGIF);
            }
            else {
                MessageBox(NULL, "This file is not a GIF-animation", APP_NAME, MB_ICONEXCLAMATION);
                return 0;
            }
        }
        else return 0;
        return 1;
}

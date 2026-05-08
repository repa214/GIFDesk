#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#include "types.h"
#include "gifdesk.h"

SINLINE void GetSettingsPath(Manager* manager) {
    const char ofnfilter[] =
        "Animations (*.gif, *.webp, *.png, *.avif)\0*.gif; *.webp; *.png; *.apng; *.avif; *.avifs\0"
        "GIF (*.gif)\0*.gif\0"
        "WEBP (*.webp)\0*.webp\0"
        "APNG (*.png; *.apng)\0*.png; *.apng\0"
        "AVIF (*.avif; *.avifs)\0*.avif; *.avifs\0"
        "All files (*.*)\0*.*\0";
    if (!memcpy((void *)manager->ofnfilter, ofnfilter, sizeof(ofnfilter))) {
        manager->error = MANAGER_ERR_GET_OFNFILTER;
        ManagerHandleError(manager);
        return;
    }

    static char settings_path[MAX_PATH] = "";
    if (!GetModuleFileName(NULL, settings_path, sizeof(settings_path))) {
        manager->error = MANAGER_ERR_GET_EXEPATH;
        ManagerHandleError(manager);
        return;
    }
    char* ls = strrchr(settings_path, '\\');
    if (ls) *(ls + 1) = '\0';
    strcat(settings_path, "settings.bin");
    strcpy(manager->settings_path, settings_path);
//    printf("%s\n", manager->settings_path);
}

SINLINE uint8_t SetSettings(Manager* manager) {
    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = manager->window;
    ofn.lpstrFile = manager->buff_filepath;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = manager->ofnfilter;
    ofn.nFilterIndex = 1;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrTitle = "Choose animation file";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) return 1;
    else return 0;
}

SINLINE void SwapFilenames(Manager* manager, uint16_t index) {
    strncpy(manager->gfk[index].filepath, manager->buff_filepath, MAX_PATH);
    manager->gfk[index].filepath[MAX_PATH - 1] = '\0';
    /* D:/Folder/filename.img -> filename.img */
    strncpy(manager->gfk[index].filename, strrchr(manager->gfk[index].filepath, '\\') + 1, MAX_PATH - 1);
    manager->gfk[index].filename[MAX_PATH - 1] = '\0';
}

SINLINE void WriteSettings(Manager* manager) {
    FILE* f = fopen(manager->settings_path, "wb");
    if (f == NULL) return;

    fwrite(&manager->settings_ver, sizeof(uint8_t), 1, f);
    fwrite(&manager->settings, sizeof(uint8_t), 1, f);
    fwrite(&manager->size, sizeof(float), 1, f);
    fwrite(&manager->speed, sizeof(uint16_t), 1, f);
    fwrite(&manager->transparency, sizeof(uint8_t), 1, f);
    fwrite(&manager->lang, sizeof(uint8_t), 1, f);
    fwrite(&manager->flags, sizeof(uint8_t), 1, f);
    fwrite(&manager->sfu, sizeof(uint8_t), 1, f);
    fwrite(&manager->gfk_count, sizeof(uint16_t), 1, f);

    for (uint16_t i = 0; i < manager->gfk_count; i++) {
        if (!manager->gfk) continue;

        fwrite(manager->gfk[i].filepath, sizeof(char), 260, f);
        fwrite(&manager->gfk[i].size, sizeof(float), 1, f);
        fwrite(&manager->gfk[i].x, sizeof(int16_t), 1, f);
        fwrite(&manager->gfk[i].y, sizeof(int16_t), 1, f);
        fwrite(&manager->gfk[i].speed, sizeof(uint16_t), 1, f);
        fwrite(&manager->gfk[i].flags, sizeof(uint8_t), 1, f);
        fwrite(&manager->gfk[i].transparency, sizeof(uint8_t), 1, f);
        fwrite(&manager->gfk[i].lang, sizeof(uint8_t), 1, f);
    }

    fclose(f); return;
}

SINLINE void ParseSettings(Manager* manager) {
    manager->settings_ver = SETTINGS_VER;
    manager->settings = 0x1;
    manager->size = 1.0f;
    manager->speed = 20; // 20 * 0.05 = 1
    manager->transparency = 255;
    manager->lang = 255;
    manager->flags = 0x1;
    manager->sfu = 0;
    manager->gfk_count = 0;

    FILE* f = fopen(manager->settings_path, "rb");
    if (f == NULL) return;

    char filepath[MAX_PATH];
    float size = 1;
    int16_t x = 0, y = 0;
    uint16_t speed;
    uint8_t flags, transparency, lang, gfk_count;

    if (fread(&manager->settings_ver, sizeof(uint8_t), 1, f) < 1) goto _parse_settings_fail;
    if (fread(&manager->settings, sizeof(uint8_t), 1, f) < 1) goto _parse_settings_fail;
    if (fread(&manager->size, sizeof(float), 1, f) < 1) goto _parse_settings_fail;
    if (fread(&manager->speed, sizeof(uint16_t), 1, f) < 1) goto _parse_settings_fail;
    if (fread(&manager->transparency, sizeof(uint8_t), 1, f) < 1) goto _parse_settings_fail;
    if (fread(&manager->lang, sizeof(uint8_t), 1, f) < 1) goto _parse_settings_fail;
    if (fread(&manager->flags, sizeof(uint8_t), 1, f) < 1) goto _parse_settings_fail;
    if (fread(&manager->sfu, sizeof(uint8_t), 1, f) < 1) goto _parse_settings_fail;
    if (fread(&gfk_count, sizeof(uint16_t), 1, f) < 1) goto _parse_settings_fail;

    if (gfk_count) {
        GIFDesk* buff = realloc(manager->gfk, sizeof(GIFDesk) * gfk_count);
        if (!buff) {
            manager->error = MANAGER_WARN_GIFDESK_ALLOC;
            ManagerHandleError(manager);
            return;
        }
        manager->gfk = buff;
        memset(manager->gfk, 0, sizeof(GIFDesk) * gfk_count);
    }

    for (uint16_t i = 0; i < gfk_count; i++) {
        if (fread(filepath, sizeof(char), 260, f) < 260) continue;
        if (fread(&size, sizeof(float), 1, f) < 1) continue;
        if (fread(&x, sizeof(int16_t), 1, f) < 1) continue;
        if (fread(&y, sizeof(int16_t), 1, f) < 1) continue;
        if (fread(&speed, sizeof(uint16_t), 1, f) < 1) continue;
        if (fread(&flags, sizeof(uint8_t), 1, f) < 1) continue;
        if (fread(&transparency, sizeof(uint8_t), 1, f) < 1) continue;
        if (fread(&lang, sizeof(uint8_t), 1, f) < 1) continue;

        DragAcceptFiles(manager->window, FALSE);
        GIFDeskFromParams(manager, i, filepath, size, x, y, speed, transparency, lang, flags, 0);
        DragAcceptFiles(manager->window, TRUE);
    }

    fclose(f);
_parse_settings_fail:
    manager->error = MANAGER_WARN_PARSE_SETTINGS;
    fclose(f);
}

#endif // SETTINGS_H_INCLUDED

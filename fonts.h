#ifndef FONTS_H_INCLUDED
#define FONTS_H_INCLUDED

#include "types.h"

/** DPI MANAGEMENT **/

SINLINE void SetDPIContext() {
    OSVERSIONINFO osvi = {0};
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);

    if (osvi.dwMajorVersion >= 6) {
        HMODULE module = LoadLibraryA("user32.dll");
        if (module) {
            typedef BOOL (__stdcall *DPIPTR) (void);
            DPIPTR SetProcessDPI = (DPIPTR)GetProcAddress(module, "SetProcessDPIAware");
            if (SetProcessDPI) SetProcessDPI();
            FreeLibrary(module);
        }
    }
}

SINLINE UINT GetDPI(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    if (!hdc) return DPI_DEFAULT;

    UINT dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(hwnd, hdc);
    return dpi;
}

SINLINE int ScaleForDPI(int value, float scale) {
    return (int)((float)value * scale + 0.555555);
}

SINLINE float ScaleForDPIf(int value, float scale) {
    return (float)value * scale;
}

SINLINE int UnScaleForDPI(int value, float scale) {
    return (int)((float)value / scale + 0.555555);
}

SINLINE float UnScaleForDPIf(int value, float scale) {
    return (float)value / scale;
}

/** FONTS **/

#define FONT_NULL "", 16, 0, 0, 0, FW_DONTCARE, \
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, \
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_QUALITY

#define FONT_MAIN "Segoe UI", 16, 0, 0, 0, FW_DONTCARE, \
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, \
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_QUALITY

#define FONT_CORBEL "Cambria", 24, 0, 0, 0, FW_DONTCARE, \
        TRUE, FALSE, FALSE, DEFAULT_CHARSET, \
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_QUALITY

#define FONT_LABEL_NAME "Segoe UI", 16, 0, 0, 0, FW_SEMIBOLD, \
        TRUE, FALSE, FALSE, DEFAULT_CHARSET, \
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_QUALITY

#define FONT_LABEL "Segoe UI", 20, 0, 0, 0, FW_DONTCARE, \
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, \
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_QUALITY

#define FONT_DESCRIPTION "Segoe UI", 15, 0, 0, 0, FW_DONTCARE, \
        TRUE, FALSE, FALSE, DEFAULT_CHARSET, \
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_QUALITY

#define FONT_BTN "Segoe UI", 17, 0, 0, 0, FW_DONTCARE, \
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, \
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_QUALITY

#define FONT_MARLETT "Marlett", 17, 0, 0, 0, FW_DONTCARE, \
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, \
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_QUALITY

#define FONT_ARIAL "Cascadia Mono", 17, 0, 0, 0, FW_DONTCARE, \
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, \
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_QUALITY

#define FONT_FRAMES "Segoe UI", 17, 0, 0, 0, FW_DONTCARE, \
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, \
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_QUALITY

SINLINE uint8_t _AddManagerFont(Manager* manager, const char* pszFaceName, int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight,
                                DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet,
                                DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily) {
    HFONT font = CreateFont(
        ScaleForDPI(cHeight, manager->scale_dpi),
        ScaleForDPI(cWidth, manager->scale_dpi),
        cEscapement,
        cOrientation,
        cWeight,
        bItalic,
        bUnderline,
        bStrikeOut,
        iCharSet,
        iOutPrecision,
        iClipPrecision,
        iQuality,
        iPitchAndFamily,
        pszFaceName
    );

    if (!font) return 0;

    HFONT* buff = realloc(manager->fonts, sizeof(HFONT) * (manager->fonts_count + 1));
    if (!buff) {
        DeleteObject(font);
        return 0;
    }

    manager->fonts = buff;
    manager->fonts[manager->fonts_count] = font;
    manager->fonts_count++;

    return 1;
}

#endif // FONTS_H_INCLUDED

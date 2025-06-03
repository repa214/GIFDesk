#ifndef LANGUAGE_H_INCLUDED
#define LANGUAGE_H_INCLUDED

struct LANG {
    const char notGIF[2][20];

    const char changeGIF[2][16];
    const char scaleGIF[2][17];
    const char showiconGIF[2][29];
    const char topmostGIF[2][17];
    const char movewindowGIF[2][15];
    const char langGIF[2][9];
    const char exitGIF[2][6];

    const char tlcGIF[2][19];
    const char trcGIF[2][20];
    const char blcGIF[2][21];
    const char brcGIF[2][22];

    const char saveGIF[2][21];

    const char selectGIF[2][14];
};

extern struct LANG lang;
extern int LANGGIF;

#endif // LANGUAGE_H_INCLUDED

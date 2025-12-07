#include "gifdesk.h"

int Run(GIFDesk* gf)
{
    /** Init structures **/

    WindowInit(&gf->window, "Window", MainWindowProc);
    WindowInit(&gf->popup_window, "PopupWindow", PopupMenuProc);
    WindowInit(&gf->mwt_window, "FrameInfo", MWTProc);

    GetSettingsPath(&gf->settings); DataInit(&gf->data); GetSettings(&gf->settings);

    RptrInit(&rptr, &gf->settings, &gf->data, &gf->rd,
             &gf->window, &gf->popup_window, &gf->debug_window,

             &gf->scale_trackbar, &gf->frames_trackbar,

             &gf->scale_button, &gf->title_button, &gf->openfile_button, &gf->addscale_button,
             &gf->decscale_button, &gf->pause_button, &gf->sfp_button,
             &gf->sti_button, &gf->aot_button, &gf->mwt_button, &gf->tlc_button, &gf->trc_button,
             &gf->cnr_button, &gf->blc_button, &gf->brc_button, &gf->language_button,
             &gf->exit_button, &gf->mwt_window);

    LoadWindow(&gf->window, &gf->settings, NULL, "Window", 0, 0, 1, 1, MAIN_WINDOW);

    /** Check settings and file **/

    uint8_t fs = 0; /// Don`t create dialogbox at «first start»
    uint8_t filetype = 0x00; do
    {
        /// Setting settings file
        if (GetSettings(&gf->settings))
            filetype = CheckFile((const char *)&gf->settings.filename, &gf->data);
        else {
            fs = 1;
            filetype = SetSettings(&gf->window, &gf->settings, &gf->data);
        }

        /// If user didn`t select file
        if (filetype == 0xFF) goto _gifdesk_release;

        /// User selected INVALIDE_FORMAT
        if (filetype == 0x00) {
            gf->data.error = 255;
            if (!fs || DialogBoxParam(GetModuleHandle(NULL), (LPCTSTR)IDD_DIALOG_BEXIT, NULL, (DLGPROC)DlgProc, 0))
            {
                remove((const char *)&gf->settings.settings_path);
                continue;
            }
            else
                goto _gifdesk_release;
        }

        /// Checking selected file
        uint8_t valid = LoadFile(&gf->window, &gf->settings, &gf->data, filetype); gf->data.error = valid;
        if (valid == 0)
            break;

        /// If user pushed Select file
        if (!fs || DialogBoxParam(GetModuleHandle(NULL), (LPCTSTR)IDD_DIALOG_BEXIT_E, NULL, (DLGPROC)DlgProc, 0)) {
            continue;
        }
        else
            goto _gifdesk_release;

    }   while (1);

    if (fs) {
        SwapFilenames(&gf->settings);
        WriteSettings(&gf->settings);
    }

    ReloadWindow(&gf->window, &gf->settings, &gf->data);

    Loop(&rptr);

    goto _gifdesk_release;

_gifdesk_release:
    ReleaseWindow(&gf->window);

    ClearData(&gf->data);

    return 0;
}

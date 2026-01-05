#include "gifdesk.h"

int Run(GIFDesk* gf)
{
    /** Init structures **/

    WindowInit(&gf->window, "Window", MainWindowProc);
    WindowInit(&gf->window_popup, "window_popup", PopupMenuProc);
    WindowInit(&gf->window_wc, "window_wc", WCProc);
    WindowInit(&gf->window_pb, "window_pb", PBProc);
    WindowInit(&gf->window_im, "window_im", IMProc);
    WindowInit(&gf->window_mwt, "window_mwt", MWTProc);

    GetSettingsPath(&gf->settings); DataInit(&gf->data); GetSettings(&gf->settings);

    RptrInit(&rptr, &gf->settings, &gf->data, &gf->rd,
             &gf->window, &gf->window_popup, &gf->window_debug,
             &gf->window_wc, &gf->window_pb, &gf->window_im, &gf->window_pw, &gf->window_mwt,

             &gf->trackbar_scale, &gf->trackbar_frames, &gf->trackbar_speed, &gf->trackbar_transparency,

             &gf->btn_title, &gf->btn_openfile, &gf->label_window_scale, &gf->label_scale, &gf->btn_add_scale,
             &gf->btn_subtract_scale, &gf->label_playback, &gf->label_frames, &gf->btn_prev_frame, &gf->btn_play,
             &gf->btn_next_frame, &gf->label_speed, &gf->btn_slow_rewind, &gf->btn_fast_rewind, &gf->btn_slow_wind,
             &gf->btn_fast_wind, &gf->label_transparency, &gf->btn_frame_updates,
             &gf->label_interaction, &gf->btn_ignore_input,
             &gf->btn_pin_top, &gf->label_move_window, &gf->btn_move_topleft,
             &gf->btn_move_topright, &gf->btn_move_center, &gf->btn_move_left, &gf->btn_move_right, &gf->btn_close_window,
             &gf->btn_taskbar);

    LoadWindow(&gf->window, &gf->settings, NULL, &gf->rd, "Window", 0, 0, 1, 1, MAIN_WINDOW);

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

    ReloadWindow(&gf->window, &gf->settings, &gf->data, 0);

    Loop(&rptr);

    goto _gifdesk_release;

_gifdesk_release:
    ReleaseWindow(&gf->window);

    ClearData(&gf->data);

    return 0;
}

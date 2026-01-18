#ifndef GIFDESK_H_INCLUDED
#define GIFDESK_H_INCLUDED

#include "types.h"
#include "window.h"
#include "settings.h"
#include "data.h"
#include "render.h"

typedef struct
{
    Window window;
    Window window_popup;
    Window window_debug;

    Window window_pb;
    Window window_im;
    Window window_wc;
    Window window_pw;
    Window window_mwt;

    Button btn_title;
    Button btn_openfile;

    Button label_playback;

        Button label_frames;
        Button btn_prev_frame;
        Button btn_play;
        Button btn_next_frame;

        Trackbar trackbar_frames;

        Button label_speed;
        Button btn_slow_rewind;
        Button btn_fast_rewind;
        Button btn_slow_wind;
        Button btn_fast_wind;

        Trackbar trackbar_speed;

        Button btn_frame_updates;

    Button label_interaction;

        Button btn_disable_moving;
        Button btn_hide_hover;
        Button btn_click_through;
        Button btn_ignore_input;

    Button label_window_scale;

        Button label_scale;
        Button btn_add_scale;
        Button btn_subtract_scale;

        Trackbar trackbar_scale;

        Button label_transparency;

        Trackbar trackbar_transparency;

        Button btn_pin_top;
        Button btn_show_tray;
        Button btn_taskbar;

    Button label_move_window;

        Button btn_move_topleft;
        Button btn_move_topright;
        Button btn_move_center;
        Button btn_move_left;
        Button btn_move_right;

    Button label_debug;

    Button btn_close_window;

    Settings settings;
    Data data;
    Render rd;

}   GIFDesk;

int Run(GIFDesk* gf);


#endif // GIFDESK_H_INCLUDED

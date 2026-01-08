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

    /** Popup Menu **/

    Button btn_title;
    Button btn_openfile;

    /// Playback
    Button label_playback;
/**
        Frame: 1/39
          |<     ||     >|
        |------------------|
**/
        Button label_frames;
        Button btn_prev_frame;
        Button btn_play;
        Button btn_next_frame;

        Trackbar trackbar_frames;

/**
        Speed: [0.25x-2.0x] [+- 0.05x]
        |------------------|
**/
        Button label_speed;
        Button btn_slow_rewind;
        Button btn_fast_rewind;
        Button btn_slow_wind;
        Button btn_fast_wind;

        Trackbar trackbar_speed;

/**
        Show frame updates (GIF, PNG)
**/
        Button btn_frame_updates;

    /// Interaction
    Button label_interaction;
/**
        Disable moving
        Hide on hover
        Click-through
        Ignore all input (Esc to disable)
**/
        Button btn_disable_moving;
        Button btn_hide_hover;
        Button btn_click_through;
        Button btn_ignore_input;

    /// Window
    Button label_window_scale;
/**
        Scale [1-1000%] +  -
        |------------------|
**/
        Button label_scale;
        Button btn_add_scale;
        Button btn_subtract_scale;

        Trackbar trackbar_scale;
/**
        Transparency [1-100%]
        |------------------|
**/
        Button label_transparency;

        Trackbar trackbar_transparency;

/**
        Always on top
        Show taskbar icon
**/
        Button btn_pin_top;
        Button btn_taskbar;

    /// Move window to
    Button label_move_window;
/**
        Top left corner
        Top right corner
        Center
        Bottom left corner
        Bottom right corner
**/
        Button btn_move_topleft;
        Button btn_move_topright;
        Button btn_move_center;
        Button btn_move_left;
        Button btn_move_right;

    /// Close window
    Button btn_close_window;

    Settings settings;
    Data data;
    Render rd;

}   GIFDesk;

int Run(GIFDesk* gf);


#endif // GIFDESK_H_INCLUDED

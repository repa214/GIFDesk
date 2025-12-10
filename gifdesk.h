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
    Window popup_window;
    Window mwt_window;
    Window debug_window;

    /** trackbars **/
    Trackbar scale_trackbar;
    Trackbar frames_trackbar;

    /** buttons **/
    Button title_button;
    Button openfile_button;

    Button scale_button;
    Button addscale_button;
    Button decscale_button;

    Button pause_button;

    Button sfp_button;
    Button sti_button;
    Button aot_button;

    Button mwt_button;
    Button tlc_button;
    Button trc_button;
    Button cnr_button;
    Button blc_button;
    Button brc_button;

    Button language_button;
    Button exit_button;

    Settings settings;
    Data data;
    Render rd;

}   GIFDesk;

int Run(GIFDesk* gf);


#endif // GIFDESK_H_INCLUDED

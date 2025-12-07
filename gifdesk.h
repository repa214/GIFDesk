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
    Window debug_window;

    /** trackbars **/
    Window scale_trackbar;
    Window frames_trackbar;

    /** buttons **/
    Window title_button;
    Window openfile_button;

    Window scale_button;
    Window addscale_button;
    Window decscale_button;

    Window pause_button;

    Window sfp_button;
    Window sti_button;
    Window aot_button;

    Window mwt_button;
    Window tlc_button;
    Window trc_button;
    Window cnr_button;
    Window blc_button;
    Window brc_button;

    Window language_button;

    Window exit_button;

    Window mwt_window;

    Settings settings;
    Data data;
    Render rd;

}   GIFDesk;

int Run(GIFDesk* gf);


#endif // GIFDESK_H_INCLUDED

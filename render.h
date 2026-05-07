#ifndef RENDER_H_INCLUDED
#define RENDER_H_INCLUDED

#include "types.h"

SINLINE void ChangeTexFilt(Manager* manager, GIFDesk* gfk, GLint param) {
    wglMakeCurrent(gfk->hdc, gfk->hrc);

    for (int i = 0; i < gfk->count; i++) {
        glBindTexture(GL_TEXTURE_2D, gfk->textures[i]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    wglMakeCurrent(NULL, NULL);
}

SINLINE double GetTime() {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / freq.QuadPart;
}

SINLINE int GetCurrentFrame(GIFDesk* gfk) {
    gfk->current_time = GetTime();
    int frame = 0;

    while (
       (float)((int)((gfk->current_time -
                      gfk->start_time) * 100)
               %
       (int)((gfk->lengths[
              gfk->count - 1] /
       ((float)gfk->speed * 0.05)) * 100)) / 100 >
       (gfk->lengths[frame] / ((float)gfk->speed * 0.05)) && (int)gfk->count - 1 >= frame
    )
        frame++;

    return frame;
}

SINLINE void ShowFrame(Manager* manager, uint16_t gfk_index, uint8_t skip) {
    GIFDesk* gfk = &manager->gfk[gfk_index];
    int cframe = gfk->render_frame;
//
    if (gfk->change_frames) gfk->render_frame = GetCurrentFrame(gfk);
//
    if (cframe == gfk->render_frame && !skip && gfk->count > 1) return;

    WaitForSingleObject(manager->glmutex, INFINITE);
    wglMakeCurrent(gfk->hdc, gfk->hrc);
//
    float dw_ = (float)gfk->width,     dh_ = (float)gfk->height,
          dwh = (float)gfk->npotwidth, dhh = (float)gfk->npotheight,
          stt = gfk->size;

    int coordh_LTC = (int)   (dhh * 2 - dhh * stt - (dh_ - dhh) * stt + 0.5),
        coordh_LLC = (int) - ((dh_ - dhh) * stt + 0.5),
        coordw_RTC = (int)   (dwh * 2 - dwh * stt + 0.5),
        coordh_RTC = (int)   (dhh * 2 - dhh * stt - (dh_ - dhh) * stt + 0.5),

        coordw_C   = (int)   ((dwh * 2 - dwh * stt) / 2 + 0.5),
        coordh_C   = (int)   ((dhh * 2 - dhh * stt) / 2 - ((dh_ - dhh) * stt) + 0.5),

        coordw     = (int)   (dw_ * stt + 0.5),
        coordh     = (int)   (dh_ * stt + 0.5);

//

    switch (gfk->pos) {
        case POS_NULL:
            glViewport(0, 0, coordw, coordh);
            break;
        case POS_LTC:
            glViewport(0, coordh_LTC, coordw, coordh);
            break;
        case POS_LLC:
            glViewport(0, coordh_LLC, coordw, coordh);
            break;
        case POS_RTC:
            glViewport(coordw_RTC, coordh_RTC, coordw, coordh);
            break;
        case POS_RLC:
            glViewport(coordw_RTC, coordh_LLC, coordw, coordh);
            break;
        case POS_C:
            glViewport(coordw_C, coordh_C, coordw, coordh);
            break;
        default:
            break;
    }

    if (gfk->index >= manager->gfk_count) { return; }
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gfk->textures[gfk->render_frame]);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glPushMatrix();

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(2, GL_FLOAT, 0, &gfk->vertex);
        glTexCoordPointer(2, GL_FLOAT, 0, &gfk->texcoord);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);

        if (0) { // (gfk->sfu) {
            glColor4f(1.0, 0.0, 0.0, 1.0);
            glLineWidth(1.0f);

            glBegin(GL_LINE_LOOP);
                glVertex2f(gfk->frame_points[gfk->render_frame * 4],
                           gfk->frame_points[gfk->render_frame * 4 + 3]);
                glVertex2f(gfk->frame_points[gfk->render_frame * 4 + 2],
                           gfk->frame_points[gfk->render_frame * 4 + 3]);
                glVertex2f(gfk->frame_points[gfk->render_frame * 4 + 2],
                           gfk->frame_points[gfk->render_frame * 4 + 1]);
                glVertex2f(gfk->frame_points[gfk->render_frame * 4],
                           gfk->frame_points[gfk->render_frame * 4 + 1]);
            glEnd();

            glColor4f(0.0, 1.0, 0.0, 1.0);

            glBegin(GL_LINE_LOOP);
                glVertex2f(-0.9999f, -0.9999f);
                glVertex2f(0.9999f, -0.9999f);
                glVertex2f(0.9999f, 0.9999f);
                glVertex2f(-0.9999f, 0.9999f);
            glEnd();
        }

    glPopMatrix();

    SwapBuffers(gfk->hdc);
    wglMakeCurrent(NULL, NULL);
    ReleaseMutex(manager->glmutex);
}

SINLINE DWORD WINAPI Loop(LPVOID args) {
    GIFDeskLoop* loopargs = (GIFDeskLoop *)args;
    Manager* manager = loopargs->manager;
    uint16_t gfk_index = loopargs->gfk_index;
    GIFDesk* gfk = NULL;

    while (1) {
        if (gfk_index >= manager->gfk_count) break;
        gfk = &manager->gfk[gfk_index];
        if (!gfk->render_thread) break;

        ShowFrame(manager, gfk_index, 0);

        Sleep(1);
    }
    free(loopargs);
    return 0;
}

#endif // RENDER_H_INCLUDED

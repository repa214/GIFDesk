#ifndef RENDER_H_INCLUDED
#define RENDER_H_INCLUDED

#include "types.h"
#include <math.h>

SINLINE void ChangeTexFilt(Manager* manager, uint16_t index, GLint param) {
    wglMakeCurrent(manager->gfk[index].hdc, manager->gfk[index].hrc);

    for (int i = 0; i < manager->gfk[index].count; i++) {
        glBindTexture(GL_TEXTURE_2D, manager->gfk[index].textures[i]);

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

SINLINE int GetCurrentFrame(Manager* manager, uint16_t index) {
//    double current_time = GetTime();
//    double elapsed = current_time - manager->gfk[index].start_time;
//    double speed_factor = (double)manager->gfk[index].speed * 0.05;
//    double total_duration = manager->gfk[index].lengths[manager->gfk[index].count - 1] / speed_factor;
//
//    double cycle_time = fmod(elapsed, total_duration);
//    if (cycle_time < 0) cycle_time += total_duration;
//
//    int frame = 0;
//    while (frame < manager->gfk[index].count - 1 &&
//           cycle_time > (manager->gfk[index].lengths[frame] / speed_factor)) {
//        frame++;
//    }

    int frame = 0;
    while (
       (float)((int)((GetTime() - manager->gfk[index].start_time) * 100)
               %
       (int)((manager->gfk[index].lengths[manager->gfk[index].count - 1] / ((float)manager->gfk[index].speed * 0.05)) * 100)) / 100 >
       (manager->gfk[index].lengths[frame] / ((float)manager->gfk[index].speed * 0.05)) && (int)manager->gfk[index].count - 1 >= frame
    )
        frame++;

    return frame;
}

SINLINE void ShowFrame(Manager* manager, uint16_t gfk_index, uint8_t skip) {
    int cframe = manager->gfk[gfk_index].render_frame;

    if (manager->gfk[gfk_index].change_frames &&
        manager->gfk[gfk_index].schange_frames) manager->gfk[gfk_index].render_frame = GetCurrentFrame(manager, gfk_index);

    if (cframe == manager->gfk[gfk_index].render_frame && !skip && manager->gfk[gfk_index].count > 1) { return; }

    WaitForSingleObject(manager->glmutex, INFINITE);
    wglMakeCurrent(manager->gfk[gfk_index].hdc, manager->gfk[gfk_index].hrc);

    float dw_ = (float)manager->gfk[gfk_index].width,     dh_ = (float)manager->gfk[gfk_index].height,
          dwh = (float)manager->gfk[gfk_index].npotwidth, dhh = (float)manager->gfk[gfk_index].npotheight,
          stt = manager->gfk[gfk_index].size;

    int coordh_LTC = (int)   (dhh * 2 - dhh * stt - (dh_ - dhh) * stt + 0.5),
        coordh_LLC = (int) - ((dh_ - dhh) * stt + 0.5),
        coordw_RTC = (int)   (dwh * 2 - dwh * stt + 0.5),
        coordh_RTC = (int)   (dhh * 2 - dhh * stt - (dh_ - dhh) * stt + 0.5),

        coordw_C   = (int)   ((dwh * 2 - dwh * stt) / 2 + 0.5),
        coordh_C   = (int)   ((dhh * 2 - dhh * stt) / 2 - ((dh_ - dhh) * stt) + 0.5),

        coordw     = (int)   (dw_ * stt + 0.5),
        coordh     = (int)   (dh_ * stt + 0.5);

//

    switch (manager->gfk[gfk_index].pos) {
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

    if (manager->gfk[gfk_index].index >= manager->gfk_count) { return; }
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, manager->gfk[gfk_index].textures[manager->gfk[gfk_index].render_frame]);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glPushMatrix();

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(2, GL_FLOAT, 0, &manager->gfk[gfk_index].vertex);
        glTexCoordPointer(2, GL_FLOAT, 0, &manager->gfk[gfk_index].texcoord);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);

        if (0) { // (manager->gfk[gfk_index].sfu) {
            glColor4f(1.0, 0.0, 0.0, 1.0);
            glLineWidth(1.0f);

            glBegin(GL_LINE_LOOP);
                glVertex2f(manager->gfk[gfk_index].frame_points[manager->gfk[gfk_index].render_frame * 4],
                           manager->gfk[gfk_index].frame_points[manager->gfk[gfk_index].render_frame * 4 + 3]);
                glVertex2f(manager->gfk[gfk_index].frame_points[manager->gfk[gfk_index].render_frame * 4 + 2],
                           manager->gfk[gfk_index].frame_points[manager->gfk[gfk_index].render_frame * 4 + 3]);
                glVertex2f(manager->gfk[gfk_index].frame_points[manager->gfk[gfk_index].render_frame * 4 + 2],
                           manager->gfk[gfk_index].frame_points[manager->gfk[gfk_index].render_frame * 4 + 1]);
                glVertex2f(manager->gfk[gfk_index].frame_points[manager->gfk[gfk_index].render_frame * 4],
                           manager->gfk[gfk_index].frame_points[manager->gfk[gfk_index].render_frame * 4 + 1]);
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

    SwapBuffers(manager->gfk[gfk_index].hdc);
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

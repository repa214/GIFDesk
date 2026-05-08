#ifndef DATA_H_INCLUDED
#define DATA_H_INCLUDED

#include "types.h"

//--------------------------------------------------
// OPENGL
//--------------------------------------------------

SINLINE int _GetCollisionSize(int n, float size) {
    return ((int)((float)n * size + 0.5) < 10) ? 10 : (int)((float)n * size + 0.5);
}

SINLINE int GetPOTSize(int width, int height) {
    int s = 1, n = (width > height) ? width : height;
    while (s < n) s *= 2;
    return s;
}

SINLINE GLenum _GLImage(Manager* manager, uint16_t index) {
    WaitForSingleObject(manager->glmutex, INFINITE);

    if (!manager->gfk[index].hdc || !manager->gfk[index].hrc) {
        ReleaseMutex(manager->glmutex);
        return 0;
    }

    if (!wglMakeCurrent(manager->gfk[index].hdc, manager->gfk[index].hrc)) {
        ReleaseMutex(manager->glmutex);
        return 0;
    }

    glGenTextures(1, &manager->gfk[index].textures[manager->gfk[index].count]);
    glBindTexture(GL_TEXTURE_2D, manager->gfk[index].textures[manager->gfk[index].count]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    manager->gfk[index].glversion < 1.2 ? GL_CLAMP : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    manager->gfk[index].glversion < 1.2 ? GL_CLAMP : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    (manager->gfk[index].size == 1) ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    (manager->gfk[index].size == 1) ? GL_NEAREST : GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, manager->gfk[index].width, manager->gfk[index].height,
                                0, GL_RGBA, GL_UNSIGNED_BYTE, manager->gfk[index].frame);

    glBindTexture(GL_TEXTURE_2D, 0);

    if (glGetError() != 0) return 0;

    wglMakeCurrent(NULL, NULL);
    ReleaseMutex(manager->glmutex);

    return 1;
}

//--------------------------------------------------
// CHECKS FILE
//--------------------------------------------------

SINLINE uint8_t _CheckGIF(void* data, long size) {
    struct GIF_GHDR {
        uint8_t head[6];
        uint16_t xdim, ydim;
        uint8_t flgs;
    } *ghdr = (struct GIF_GHDR*)data;

    uint8_t *buff;
    long header;

    if (!ghdr || (size <= (long)sizeof(*ghdr)) || (*(buff = ghdr->head) != 71)
    || (buff[1] != 73) || (buff[2] != 70) || (buff[3] != 56)
    || ((buff[4] != 55) && (buff[4] != 57)) || (buff[5] != 97))
        return 0;

    header = (ghdr->flgs & 0x80)? (2 << (ghdr->flgs & 7)) : 0;

    buff = (uint8_t*)(ghdr + 1) + header * 3L;

    if ((size -= buff - (uint8_t*)ghdr) <= 0)
        return 0;

    return 1;
}

SINLINE uint8_t CheckFile(const char* filepath, Manager* manager, uint16_t index) {
    /** file, filesize, filedata **/
    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        manager->error = MANAGER_WARN_FILE_NEXIST;
        ManagerHandleError(manager);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    manager->gfk[index].data_size = ftell(file);

    fseek(file, 0, SEEK_SET);
    manager->gfk[index].data = calloc(1, manager->gfk[index].data_size);

    fread(manager->gfk[index].data, 1, manager->gfk[index].data_size, file);
    fclose(file); file = NULL;

    if (_CheckGIF(manager->gfk[index].data, manager->gfk[index].data_size)) return GIF;
    else {
        manager->error = MANAGER_WARN_NO_FORMAT;
        ManagerHandleError(manager);
        return NO_FORMAT;
    }
}

//--------------------------------------------------
// LOADS FILE
//--------------------------------------------------

SINLINE int _gifr(GifFileType* gif, GifByteType* bytes, int size) {
    GIFReader* reader = (GIFReader*)gif->UserData;
    size_t available = reader->size - reader->position;
    size_t to_read = (size < available) ? size : available;

    if (to_read > 0) {
        memcpy(bytes, reader->data + reader->position, to_read);
        reader->position += to_read;
    }
    return to_read;
}

SINLINE uint8_t _LoadGIF(Manager* manager, uint16_t index) {
    GIFReader reader = { (uint8_t*)manager->gfk[index].data, manager->gfk[index].data_size, 0 };

    GifFileType* gif = NULL;
    int gif_error = 0;
    gif = DGifOpen(&reader, _gifr, &gif_error);

    if (gif == NULL) {
        switch (gif_error) {
            case D_GIF_ERR_OPEN_FAILED:
                manager->error = MANAGER_WARN_GIFLIB_OPEN_FAILED;
                break;
            case D_GIF_ERR_READ_FAILED:
                manager->error = MANAGER_WARN_GIFLIB_READ_FAILED;
                break;
            case D_GIF_ERR_NOT_GIF_FILE:
                manager->error = MANAGER_WARN_GIFLIB_NOT_GIF_FILE;
                break;
            case D_GIF_ERR_NO_SCRN_DSCR:
                manager->error = MANAGER_WARN_GIFLIB_NO_SCRN_DSCR;
                break;
            case D_GIF_ERR_NO_IMAG_DSCR:
                manager->error = MANAGER_WARN_GIFLIB_NO_IMAG_DSCR;
                break;
            case D_GIF_ERR_NO_COLOR_MAP:
                manager->error = MANAGER_WARN_GIFLIB_NO_COLOR_MAP;
                break;
            case D_GIF_ERR_WRONG_RECORD:
                manager->error = MANAGER_WARN_GIFLIB_WRONG_RECORD;
                break;
            case D_GIF_ERR_DATA_TOO_BIG:
                manager->error = MANAGER_WARN_GIFLIB_DATA_TOO_BIG;
                break;
            case D_GIF_ERR_NOT_ENOUGH_MEM:
                manager->error = MANAGER_WARN_GIFLIB_NOT_ENOUGH_MEM;
                break;
            case D_GIF_ERR_CLOSE_FAILED:
                manager->error = MANAGER_WARN_GIFLIB_CLOSE_FAILED;
                break;
            case D_GIF_ERR_NOT_READABLE:
                manager->error = MANAGER_WARN_GIFLIB_NOT_READABLE;
                break;
            case D_GIF_ERR_IMAGE_DEFECT:
                manager->error = MANAGER_WARN_GIFLIB_IMAGE_DEFECT;
                break;
            case D_GIF_ERR_EOF_TOO_SOON:
                manager->error = MANAGER_WARN_GIFLIB_EOF_TOO_SOON;
                break;
            default:
                manager->error = MANAGER_WARN_GIFLIB_NULL;
                break;
        }
        ManagerHandleError(manager);
        return 0;
    }

    if (DGifSlurp(gif) != GIF_OK) {
        DGifCloseFile(gif, &gif_error);
        manager->error = MANAGER_WARN_GIFLIB_CLOSE_FAILED;
        ManagerHandleError(manager);
        return 0;
    }

    manager->gfk[index].width = gif->SWidth;
    manager->gfk[index].height = gif->SHeight;

    if (manager->gfk[index].glversion < 2) {
        manager->gfk[index].width = GetPOTSize(manager->gfk[index].width, manager->gfk[index].height); manager->gfk[index].height = manager->gfk[index].width;
    }
    manager->gfk[index].npotwidth = manager->gfk[index].width; manager->gfk[index].npotheight = manager->gfk[index].height;

    int num = gif->ImageCount;
    long pixcount = manager->gfk[index].width * manager->gfk[index].height * 4;

    manager->gfk[index].frame = calloc(pixcount, 1);
    if (!manager->gfk[index].frame) {
        manager->error = MANAGER_WARN_FRAME_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    manager->gfk[index].buff = calloc(pixcount, 1);
    if (!manager->gfk[index].buff) {
        manager->error = MANAGER_WARN_BUFF_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    manager->gfk[index].delays = calloc(num, sizeof(float));
    if (!manager->gfk[index].delays) {
        manager->error = MANAGER_WARN_DELAYS_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    manager->gfk[index].lengths = calloc(num, sizeof(float));
    if (!manager->gfk[index].lengths) {
        manager->error = MANAGER_WARN_LENGTHS_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    manager->gfk[index].frame_points = calloc(num, sizeof(float) * 4);
    if (!manager->gfk[index].frame_points) {
        manager->error = MANAGER_WARN_FRAMEP_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    manager->gfk[index].textures = calloc(num, sizeof(GLuint));
    if (!manager->gfk[index].textures) {
        manager->error = MANAGER_WARN_TEXTURES_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    for (int i = 0; i < num; i++) {
        SavedImage savedImage = gif->SavedImages[i];
        GifImageDesc imageDesc = savedImage.ImageDesc;

        ColorMapObject* colorMap = imageDesc.ColorMap;
        if (colorMap == NULL) colorMap = gif->SColorMap;
        if (colorMap == NULL) continue;

        int transparent = -1;
        GraphicsControlBlock gcb; memset(&gcb, 0, sizeof(GraphicsControlBlock));

        for (int j = 0; j < savedImage.ExtensionBlockCount; j++) {
            if (savedImage.ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
                DGifExtensionToGCB(savedImage.ExtensionBlocks[j].ByteCount,
                                 savedImage.ExtensionBlocks[j].Bytes, &gcb);
                transparent = gcb.TransparentColor;
                break;
            }
        }
        int frxo = imageDesc.Left;
        int fryo = imageDesc.Top;
        int frxd = imageDesc.Width;
        int fryd = imageDesc.Height;

        unsigned long pix = manager->gfk[index].width * fryo * 4 + frxo * 4;
        for (int y = 0; y < fryd; y++) {
            for (int x = 0; x < frxd; x++) {
                int colorIndex = savedImage.RasterBits[y * frxd + x];

                if (colorIndex == transparent) {
                    pix += 4;
                    continue;
                }
                else {
                    GifColorType color = colorMap->Colors[colorIndex];
                    manager->gfk[index].frame[pix] = color.Red;
                    manager->gfk[index].frame[pix + 1] = color.Green;
                    manager->gfk[index].frame[pix + 2] = color.Blue;
                    manager->gfk[index].frame[pix + 3] = 255;
                }

                if (manager->gfk[index].frame[pix] == 0 && manager->gfk[index].frame[pix + 1] == 0 &&
                    manager->gfk[index].frame[pix + 2] == 0 && manager->gfk[index].frame[pix + 3] > 0) {
                    manager->gfk[index].frame[pix] = 1;
                    manager->gfk[index].frame[pix + 1] = 1;
                    manager->gfk[index].frame[pix + 2] = 1;

                }
                pix += 4;
            }
            pix += (manager->gfk[index].width - frxd) * 4;
        }
        if (gcb.DisposalMode != DISPOSE_PREVIOUS) memcpy(manager->gfk[index].buff, manager->gfk[index].frame, pixcount);
        manager->gfk[index].delays[i] = (gcb.DelayTime == 0) ? 0.1 : (float)gcb.DelayTime / 100;
        if (!manager->gfk[index].count) manager->gfk[index].lengths[0] = manager->gfk[index].delays[0];
        else manager->gfk[index].lengths[i] = manager->gfk[index].lengths[i - 1] + manager->gfk[index].delays[i];

        if (!_GLImage(manager, index)) {
            switch (glGetError()) {
                case GL_INVALID_ENUM:
                    manager->error = MANAGER_WARN_OPENGL_INVALID_ENUM;
                    break;
                case GL_INVALID_VALUE:
                    manager->error = MANAGER_WARN_OPENGL_INVALID_VALUE;
                    break;
                case GL_INVALID_OPERATION:
                    manager->error = MANAGER_WARN_OPENGL_INVALID_OPERATION;
                    break;
                case GL_STACK_OVERFLOW:
                    manager->error = MANAGER_WARN_OPENGL_STACK_OVERFLOW;
                    break;
                case GL_STACK_UNDERFLOW:
                    manager->error = MANAGER_WARN_OPENGL_STACK_UNDERFLOW;
                    break;
                case GL_OUT_OF_MEMORY:
                    manager->error = MANAGER_WARN_OPENGL_OUT_OF_MEMORY;
                    break;
            }
            ManagerHandleError(manager);
            return 0;
        }
        switch (gcb.DisposalMode)
        {
            case DISPOSE_BACKGROUND:
            {
                /// Cleaning frame`s region
                unsigned long pix = manager->gfk[index].width * fryo * 4;
                for (int y = 0; y < fryd; y++) {
                    pix += frxo * 4;
                    for (int x = 0; x < frxd; x++) {
                        memset(&manager->gfk[index].frame[pix], 0, 4);
                        pix += 4;
                    }
                    pix += (manager->gfk[index].width - frxd - frxo) * 4;
                }
            }   break;
            case DISPOSE_PREVIOUS:
            {
                memcpy(manager->gfk[index].frame, manager->gfk[index].buff, pixcount);
            }   break;
            case DISPOSE_DO_NOT:
            default : break;
        }
        manager->gfk[index].frame_points[(manager->gfk[index].count) * 4] = ((float)frxo) / ((float)_GetCollisionSize(manager->gfk[index].width, 1)) * 2 - 0.9999f;
        manager->gfk[index].frame_points[(manager->gfk[index].count) * 4 + 1] = -( ((float)fryo) / ((float)_GetCollisionSize(manager->gfk[index].height, 1)) * 2 - 0.9999f );
        manager->gfk[index].frame_points[(manager->gfk[index].count) * 4 + 2] = (float)(frxo + frxd) / ((float)_GetCollisionSize(manager->gfk[index].width, 1)) * 2 - 1.0001f;
        manager->gfk[index].frame_points[(manager->gfk[index].count) * 4 + 3] = -( (float)(fryo + fryd) / ((float)_GetCollisionSize(manager->gfk[index].height, 1)) * 2 - 1.0001f );
        manager->gfk[index].count++;
    }

    if (manager->gfk[index].frame != NULL)          { free(manager->gfk[index].frame);          manager->gfk[index].frame = NULL; }
    if (manager->gfk[index].buff != NULL)           { free(manager->gfk[index].buff);           manager->gfk[index].buff = NULL; }

    DGifCloseFile(gif, &gif_error);

    return 1;
}

SINLINE uint8_t LoadFile(Manager* manager, uint16_t index, uint8_t filetype) {
    switch (filetype) {
        case GIF:
            return _LoadGIF(manager, index);
        default:
            return NO_FORMAT;
    }
}

#endif // DATA_H_INCLUDED

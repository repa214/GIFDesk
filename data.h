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
    if (glGetError() != 0) return 0;
    glBindTexture(GL_TEXTURE_2D, manager->gfk[index].textures[manager->gfk[index].count]);
    if (glGetError() != 0) return 0;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    manager->gfk[index].glversion < 1.2 ? GL_CLAMP : GL_CLAMP_TO_EDGE);
    if (glGetError() != 0) return 0;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    manager->gfk[index].glversion < 1.2 ? GL_CLAMP : GL_CLAMP_TO_EDGE);
    if (glGetError() != 0) return 0;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    (manager->gfk[index].size == 1) ? GL_NEAREST : GL_LINEAR);
    if (glGetError() != 0) return 0;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    (manager->gfk[index].size == 1) ? GL_NEAREST : GL_LINEAR);
    if (glGetError() != 0) return 0;

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

SINLINE uint8_t _CheckAPNG(void* data, long size) {
    if (png_sig_cmp((png_bytep)data, 0, 8)) return 0;

    return 1;
}

SINLINE uint8_t _CheckWEBP(void* data, long size) {
    WebPData webp_data = { data, (size_t)size };
    WebPAnimDecoderOptions dec_options;
    WebPAnimDecoderOptionsInit(&dec_options);

    WebPAnimDecoder* dec = WebPAnimDecoderNew(&webp_data, &dec_options);
    if (!dec) return 0;

    WebPAnimInfo anim_info;
    if (!WebPAnimDecoderGetInfo(dec, &anim_info)) {
        WebPAnimDecoderDelete(dec);
        return 0;
    }

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
    else if (_CheckAPNG(manager->gfk[index].data, manager->gfk[index].data_size)) return APNG;
    else if (_CheckWEBP(manager->gfk[index].data, manager->gfk[index].data_size)) return WEBP;
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
                manager->error = MANAGER_WARN_GIF_OPEN_FAILED;
                break;
            case D_GIF_ERR_READ_FAILED:
                manager->error = MANAGER_WARN_GIF_READ_FAILED;
                break;
            case D_GIF_ERR_NOT_GIF_FILE:
                manager->error = MANAGER_WARN_GIF_NOT_GIF_FILE;
                break;
            case D_GIF_ERR_NO_SCRN_DSCR:
                manager->error = MANAGER_WARN_GIF_NO_SCRN_DSCR;
                break;
            case D_GIF_ERR_NO_IMAG_DSCR:
                manager->error = MANAGER_WARN_GIF_NO_IMAG_DSCR;
                break;
            case D_GIF_ERR_NO_COLOR_MAP:
                manager->error = MANAGER_WARN_GIF_NO_COLOR_MAP;
                break;
            case D_GIF_ERR_WRONG_RECORD:
                manager->error = MANAGER_WARN_GIF_WRONG_RECORD;
                break;
            case D_GIF_ERR_DATA_TOO_BIG:
                manager->error = MANAGER_WARN_GIF_DATA_TOO_BIG;
                break;
            case D_GIF_ERR_NOT_ENOUGH_MEM:
                manager->error = MANAGER_WARN_GIF_NOT_ENOUGH_MEM;
                break;
            case D_GIF_ERR_CLOSE_FAILED:
                manager->error = MANAGER_WARN_GIF_CLOSE_FAILED;
                break;
            case D_GIF_ERR_NOT_READABLE:
                manager->error = MANAGER_WARN_GIF_NOT_READABLE;
                break;
            case D_GIF_ERR_IMAGE_DEFECT:
                manager->error = MANAGER_WARN_GIF_IMAGE_DEFECT;
                break;
            case D_GIF_ERR_EOF_TOO_SOON:
                manager->error = MANAGER_WARN_GIF_EOF_TOO_SOON;
                break;
            default:
                manager->error = MANAGER_WARN_GIF_NULL;
                break;
        }
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 46, 40 + 20 * index); /// = 13%

    if (DGifSlurp(gif) != GIF_OK) {
        DGifCloseFile(gif, &gif_error);
        manager->error = MANAGER_WARN_GIF_CLOSE_FAILED;
        ManagerHandleError(manager);
        return 0;
    }

    manager->gfk[index].width = gif->SWidth;
    manager->gfk[index].height = gif->SHeight;

    if (manager->gfk[index].glversion < 2) {
        manager->gfk[index].width = GetPOTSize(manager->gfk[index].width, manager->gfk[index].height);
        manager->gfk[index].height = manager->gfk[index].width;
    }
    manager->gfk[index].npotwidth = manager->gfk[index].width;
    manager->gfk[index].npotheight = manager->gfk[index].height;

    _SetLoadRect(manager, 20, 20 + 20 * index, 48, 40 + 20 * index); /// = 14%

    int num = gif->ImageCount;
    long pixcount = manager->gfk[index].width * manager->gfk[index].height * 4;

    manager->gfk[index].frame = calloc(pixcount, 1);
    if (!manager->gfk[index].frame) {
        manager->error = MANAGER_WARN_FRAME_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 50, 40 + 20 * index); /// = 15%

    manager->gfk[index].buff = calloc(pixcount, 1);
    if (!manager->gfk[index].buff) {
        manager->error = MANAGER_WARN_BUFF_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 52, 40 + 20 * index); /// = 16%

    manager->gfk[index].delays = calloc(num, sizeof(float));
    if (!manager->gfk[index].delays) {
        manager->error = MANAGER_WARN_DELAYS_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 54, 40 + 20 * index); /// = 17%

    manager->gfk[index].lengths = calloc(num, sizeof(float));
    if (!manager->gfk[index].lengths) {
        manager->error = MANAGER_WARN_LENGTHS_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 56, 40 + 20 * index); /// = 18%

    manager->gfk[index].frame_points = calloc(num, sizeof(float) * 4);
    if (!manager->gfk[index].frame_points) {
        manager->error = MANAGER_WARN_FRAMEP_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 58, 40 + 20 * index); /// = 19%

    manager->gfk[index].textures = calloc(num, sizeof(GLuint));
    if (!manager->gfk[index].textures) {
        manager->error = MANAGER_WARN_TEXTURES_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 60, 40 + 20 * index); /// = 20%

    /** -------------------------------------------- **/

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
                default:
                    manager->error = MANAGER_WARN_OPENGL_OUT_OF_CONTEXT;
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

        _SetLoadRect(manager, 20, 20 + 20 * index, (uint16_t)(61 + (float)i / (float)num * (float)(198 - 61)), 40 + 20 * index); /// [21% ... 89%] [61 .. 198]
    }

    /** -------------------------------------------- **/

    if (manager->gfk[index].frame != NULL)          { free(manager->gfk[index].frame);          manager->gfk[index].frame = NULL; }
    if (manager->gfk[index].buff != NULL)           { free(manager->gfk[index].buff);           manager->gfk[index].buff = NULL; }

    DGifCloseFile(gif, &gif_error);

    return 1;
}

static void _pngr(png_structp png_ptr, png_bytep data, png_size_t length) {
    void** data_ptr = (void**)png_get_io_ptr(png_ptr);
    unsigned char* current_pos = (unsigned char*)*data_ptr;

    memcpy(data, current_pos, length);
    *data_ptr = current_pos + length;
}

static void _pngw(png_structp png_ptr, png_const_charp msg) {
    printf("_pngw: %s\n", msg);
}

static void _pnge(png_structp png_ptr, png_const_charp msg) {
    printf("_pnge: %s\n", msg);
}

SINLINE uint8_t _LoadAPNG(Manager* manager, uint16_t index) {
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep* row_pointers = NULL;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, _pnge, _pngw);
    if (!png_ptr) {
        manager->error = MANAGER_WARN_APNG_STRUCT;
        ManagerHandleError(manager);
        return 0;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        manager->error = MANAGER_WARN_APNG_INFO;
        ManagerHandleError(manager);
        return 0;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        manager->error = MANAGER_WARN_APNG_JUMP;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 46, 40 + 20 * index); /// = 13%

    png_set_read_fn(png_ptr, &manager->gfk[index].data, _pngr);
    png_read_info(png_ptr, info_ptr);

    manager->gfk[index].width = png_get_image_width(png_ptr, info_ptr);
    manager->gfk[index].height = png_get_image_height(png_ptr, info_ptr);

    if (manager->gfk[index].glversion < 2) {
        manager->gfk[index].width = GetPOTSize(manager->gfk[index].width, manager->gfk[index].height);
        manager->gfk[index].height = manager->gfk[index].width;
    }

    manager->gfk[index].npotwidth = manager->gfk[index].width;
    manager->gfk[index].npotheight = manager->gfk[index].height;

    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    if (bit_depth == 16)
        png_set_strip_16(png_ptr);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    if (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);

    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    _SetLoadRect(manager, 20, 20 + 20 * index, 48, 40 + 20 * index); /// = 14%

    int rpa = 0;
    int num = png_get_num_frames(png_ptr, info_ptr);
    int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    long pixcount = manager->gfk[index].width * manager->gfk[index].height * 4;

    manager->gfk[index].frame = calloc(pixcount, 1);
    if (!manager->gfk[index].frame) {
        manager->error = MANAGER_WARN_FRAME_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 50, 40 + 20 * index); /// = 15%

    manager->gfk[index].buff = calloc(pixcount, 1);
    if (!manager->gfk[index].buff) {
        manager->error = MANAGER_WARN_BUFF_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 52, 40 + 20 * index); /// = 16%

    manager->gfk[index].delays = calloc(num, sizeof(float));
    if (!manager->gfk[index].delays) {
        manager->error = MANAGER_WARN_DELAYS_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 54, 40 + 20 * index); /// = 17%

    manager->gfk[index].lengths = calloc(num, sizeof(float));
    if (!manager->gfk[index].lengths) {
        manager->error = MANAGER_WARN_LENGTHS_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 56, 40 + 20 * index); /// = 18%

    manager->gfk[index].frame_points = calloc(num, sizeof(float) * 4);
    if (!manager->gfk[index].frame_points) {
        manager->error = MANAGER_WARN_FRAMEP_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 58, 40 + 20 * index); /// = 19%

    manager->gfk[index].textures = calloc(num, sizeof(GLuint));
    if (!manager->gfk[index].textures) {
        manager->error = MANAGER_WARN_TEXTURES_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    row_pointers = malloc(manager->gfk[index].height * sizeof(png_bytep));
    if (!row_pointers) {
        manager->error = MANAGER_WARN_APNG_POINTERS;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 60, 40 + 20 * index); /// = 20%

    for (rpa = 0; rpa < manager->gfk[index].height; rpa++) {
        row_pointers[rpa] = malloc(row_bytes);
        if (!row_pointers[rpa]) {
            manager->error = MANAGER_WARN_APNG_POINTER;
            ManagerHandleError(manager);
            return 0;
        }
    }

    png_uint_16 delay, den;
    png_uint_32 frxo, fryo, frxd, fryd;
    png_byte mode, blend;

    png_byte prev_mode = 0;

    /** -------------------------------------------- **/

    if (num > 1) {
        for (int i = 0; i < num; i++) {
            switch (prev_mode) {
                /// APNG_DISPOSE_OP_NONE: no disposal is done on this frame before
                /// rendering the next; the contents of the output buffer are left as is

                /// APNG_DISPOSE_OP_BACKGROUND:
                /// the frame's region of the output buffer is to be cleared
                /// to fully transparent black before rendering the next frame
                case PNG_DISPOSE_OP_BACKGROUND:
                {
                    unsigned long pix = (manager->gfk[index].width * fryo) * 4;
                    for (long y = 0; y < fryd; y++) {
                        pix += frxo * 4;
                        for (long x = 0; x < frxd; x++) {
                            memset(&manager->gfk[index].frame[pix], 0, 4);

                            pix += 4;
                        }
                        pix += (manager->gfk[index].width - frxd - frxo) * 4;
                    }
                }   break;
                /// APNG_DISPOSE_OP_PREVIOUS: the frame's region of the output
                /// buffer is to be reverted to the previous contents before
                /// rendering the next frame
                case PNG_DISPOSE_OP_PREVIOUS:
                {
                    unsigned long pix = (manager->gfk[index].width * fryo) * 4;
                    for (long y = 0; y < fryd; y++) {
                        pix += frxo * 4;
                        for (long x = 0; x < frxd; x++) {
                            memcpy(&manager->gfk[index].frame[pix], &manager->gfk[index].buff[pix], 4);

                            pix += 4;
                        }
                        pix += (manager->gfk[index].width - frxd - frxo) * 4;
                    }
                }   break;
                default: break;
            }

            png_read_frame_head(png_ptr, info_ptr);
            png_get_next_frame_fcTL(png_ptr, info_ptr, &frxd, &fryd,
                                    &frxo, &fryo, &delay, &den,
                                    &mode, &blend);

            png_read_image(png_ptr, row_pointers);
            row_bytes = png_get_rowbytes(png_ptr, info_ptr);

            manager->gfk[index].delays[i] = (den == 0) ? 0.1 : (float)delay / (float)den;
            if (!manager->gfk[index].count) manager->gfk[index].lengths[0] = manager->gfk[index].delays[0];
            else manager->gfk[index].lengths[i] = manager->gfk[index].lengths[i - 1] + manager->gfk[index].delays[i];

            switch (blend) {
                /// APNG_BLEND_OP_SOURCE all color components of the frame,
                /// including alpha, overwrite the current contents of the frame's
                /// output buffer region
                case PNG_BLEND_OP_SOURCE: {
                    unsigned long pix = (manager->gfk[index].width * fryo) * 4;
                    for (long y = 0; y < fryd; y++) {
                        pix += frxo * 4;

                        for (long x = 0; x < frxd; x++) {
                            manager->gfk[index].frame[pix]     = row_pointers[y][x * 4];
                            manager->gfk[index].frame[pix + 1] = row_pointers[y][x * 4 + 1];
                            manager->gfk[index].frame[pix + 2] = row_pointers[y][x * 4 + 2];
                            manager->gfk[index].frame[pix + 3] = row_pointers[y][x * 4 + 3];

                            /// RGBA(0, 0, 0, >0) -> RGBA(1, 1, 1, >0) for compatibility with OpenGL
                            if (manager->gfk[index].frame[pix]     == 0 && manager->gfk[index].frame[pix + 3]) manager->gfk[index].frame[pix]++;
                            if (manager->gfk[index].frame[pix + 1] == 0 && manager->gfk[index].frame[pix + 3]) manager->gfk[index].frame[pix + 1]++;
                            if (manager->gfk[index].frame[pix + 2] == 0 && manager->gfk[index].frame[pix + 3]) manager->gfk[index].frame[pix + 2]++;

                            pix += 4;
                        }
                        pix += (manager->gfk[index].width - frxd - frxo) * 4;
                    }
                }   break;
                /// APNG_BLEND_OP_OVER the frame should be composited onto the output
                /// buffer based on its alpha, using a simple OVER operation
                case PNG_BLEND_OP_OVER: {
                    unsigned long pix = (manager->gfk[index].width * fryo) * 4;
                    for (long y = 0; y < fryd; y++) {
                        pix += frxo * 4;
                        png_bytep row = row_pointers[y];

                        for (int x = 0; x < frxd; x++) {
                            png_bytep px = &(row[x * 4]);

                            float src_r = (float)px[0];
                            float src_g = (float)px[1];
                            float src_b = (float)px[2];
                            float src_a = (float)px[3] / 255.0f;

                            float dst_r = (float)manager->gfk[index].frame[pix];
                            float dst_g = (float)manager->gfk[index].frame[pix + 1];
                            float dst_b = (float)manager->gfk[index].frame[pix + 2];
                            float dst_a = (float)manager->gfk[index].frame[pix + 3] / 255.0f;

                            float out_a = src_a + dst_a * (1.0f - src_a);

                            if (out_a > 0.0f) {
                                unsigned char r = (unsigned char)((src_r * src_a + dst_r * dst_a * (1.0f - src_a)) / out_a);
                                unsigned char g = (unsigned char)((src_g * src_a + dst_g * dst_a * (1.0f - src_a)) / out_a);
                                unsigned char b = (unsigned char)((src_b * src_a + dst_b * dst_a * (1.0f - src_a)) / out_a);

                                if (r > 255) r = 255;
                                if (g > 255) g = 255;
                                if (b > 255) b = 255;

                                manager->gfk[index].frame[pix]     = r;
                                manager->gfk[index].frame[pix + 1] = g;
                                manager->gfk[index].frame[pix + 2] = b;
                            } else {
                                manager->gfk[index].frame[pix]     = 0;
                                manager->gfk[index].frame[pix + 1] = 0;
                                manager->gfk[index].frame[pix + 2] = 0;
                            }

                            manager->gfk[index].frame[pix + 3] = (unsigned char)(out_a * 255.0f);
                            if (manager->gfk[index].frame[pix]     == 0 && manager->gfk[index].frame[pix + 3] > 0) manager->gfk[index].frame[pix]++;
                            if (manager->gfk[index].frame[pix + 1] == 0 && manager->gfk[index].frame[pix + 3] > 0) manager->gfk[index].frame[pix + 1]++;
                            if (manager->gfk[index].frame[pix + 2] == 0 && manager->gfk[index].frame[pix + 3] > 0) manager->gfk[index].frame[pix + 2]++;

                            pix += 4;
                        }
                        pix += (manager->gfk[index].width - frxd - frxo) * 4;
                    }
                }   break;
            }

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

            /// Writing data into buff.
            /// It will be used to return last frame when current is PNG_DISPOSE_OP_PREVIOUS
            memcpy(manager->gfk[index].buff, manager->gfk[index].frame, pixcount);

            manager->gfk[index].frame_points[(manager->gfk[index].count) * 4] = ((float)frxo) / ((float)_GetCollisionSize(manager->gfk[index].width, 1)) * 2 - 0.9999f;
            manager->gfk[index].frame_points[(manager->gfk[index].count) * 4 + 1] = -( ((float)fryo) / ((float)_GetCollisionSize(manager->gfk[index].height, 1)) * 2 - 0.9999f );
            manager->gfk[index].frame_points[(manager->gfk[index].count) * 4 + 2] = (float)(frxo + frxd) / ((float)_GetCollisionSize(manager->gfk[index].width, 1)) * 2 - 1.0001f;
            manager->gfk[index].frame_points[(manager->gfk[index].count) * 4 + 3] = -( (float)(fryo + fryd) / ((float)_GetCollisionSize(manager->gfk[index].height, 1)) * 2 - 1.0001f );

            prev_mode = mode;
            manager->gfk[index].count++;

            _SetLoadRect(manager, 20, 20 + 20 * index, (uint16_t)(61 + (float)i / (float)num * (float)(198 - 61)), 40 + 20 * index);
        }
    }
    else {
        png_read_image(png_ptr, row_pointers);
        row_bytes = png_get_rowbytes(png_ptr, info_ptr);

        manager->gfk[index].delays[0] = 1;
        manager->gfk[index].lengths[0] = 1;

        unsigned long pix = 0;
        for (long y = 0; y < manager->gfk[index].height; y++) {
            png_bytep row = row_pointers[y];

            for (long x = 0; x < manager->gfk[index].width; x++) {
                manager->gfk[index].frame[pix]     = row[x * 4];
                manager->gfk[index].frame[pix + 1] = row[x * 4 + 1];
                manager->gfk[index].frame[pix + 2] = row[x * 4 + 2];
                manager->gfk[index].frame[pix + 3] = row[x * 4 + 3];

                if (manager->gfk[index].frame[pix]     == 0 && manager->gfk[index].frame[pix + 3] > 0) manager->gfk[index].frame[pix]++;
                if (manager->gfk[index].frame[pix + 1] == 0 && manager->gfk[index].frame[pix + 3] > 0) manager->gfk[index].frame[pix + 1]++;
                if (manager->gfk[index].frame[pix + 2] == 0 && manager->gfk[index].frame[pix + 3] > 0) manager->gfk[index].frame[pix + 2]++;

                pix += 4;
            }
        }

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

        manager->gfk[index].frame_points[0] = -0.9999f;
        manager->gfk[index].frame_points[1] = 0.9999f;
        manager->gfk[index].frame_points[2] = 1.0001f;
        manager->gfk[index].frame_points[3] = -1.0001f;
        manager->gfk[index].count++;
    }

    /** -------------------------------------------- **/

    if (manager->gfk[index].frame != NULL)          { free(manager->gfk[index].frame);          manager->gfk[index].frame = NULL; }
    if (manager->gfk[index].buff != NULL)           { free(manager->gfk[index].buff);           manager->gfk[index].buff = NULL; }

    for (int i = 0; i < manager->gfk[index].height; i++) {
        if (row_pointers[i]) { free(row_pointers[i]); row_pointers[i] = NULL; }
    }
    if (row_pointers) { free(row_pointers); row_pointers = NULL; }

    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    return 1;
}

SINLINE uint8_t _LoadWEBP(Manager* manager, uint16_t index) {
    WebPAnimDecoder* dec = NULL;

    WebPData webp_data = { manager->gfk[index].data, (size_t)manager->gfk[index].data_size };

    if (!WebPGetInfo(manager->gfk[index].data, (size_t)manager->gfk[index].data_size, NULL, NULL)) {
        manager->error = MANAGER_WARN_WEBP_INFO;
        ManagerHandleError(manager);
        return 0;
    }

    WebPAnimDecoderOptions dec_options;
    if (!WebPAnimDecoderOptionsInit(&dec_options)) {
        manager->error = MANAGER_WARN_WEBP_OPTIONS;
        ManagerHandleError(manager);
        return 0;
    }

    dec_options.color_mode = MODE_RGBA;
    dec = WebPAnimDecoderNew(&webp_data, &dec_options);

    if (dec == NULL) {
        VP8StatusCode status = WebPGetFeatures(manager->gfk[index].data, manager->gfk[index].size, NULL);
        switch (status) {
            case VP8_STATUS_OUT_OF_MEMORY:
                manager->error = MANAGER_WARN_WEBP_OUT_OF_MEMORY;
                break;
            case VP8_STATUS_INVALID_PARAM:
                manager->error = MANAGER_WARN_WEBP_INVALID_PARAM;
                break;
            case VP8_STATUS_BITSTREAM_ERROR:
                manager->error = MANAGER_WARN_WEBP_BITSTREAM_ERROR;
                break;
            case VP8_STATUS_UNSUPPORTED_FEATURE:
                manager->error = MANAGER_WARN_WEBP_UNSUPPORTED_FEATURE;
                break;
            case VP8_STATUS_SUSPENDED:
                manager->error = MANAGER_WARN_WEBP_SUSPENDED;
                break;
            case VP8_STATUS_USER_ABORT:
                manager->error = MANAGER_WARN_WEBP_USER_ABORT;
                break;
            case VP8_STATUS_NOT_ENOUGH_DATA:
                manager->error = MANAGER_WARN_WEBP_NOT_ENOUGH_DATA;
                break;
            default:
                manager->error = MANAGER_WARN_WEBP_UNKNOWN;
                break;
        }
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 46, 40 + 20 * index); /// = 13%

    WebPAnimInfo anim_info;
    if (!WebPAnimDecoderGetInfo(dec, &anim_info)) {
        manager->error = MANAGER_WARN_WEBP_DECODER;
        ManagerHandleError(manager);
        return 0;
    }
    WebPAnimDecoderGetInfo(dec, &anim_info);

    manager->gfk[index].width = anim_info.canvas_width;
    manager->gfk[index].height = anim_info.canvas_height;

    if (manager->gfk[index].glversion < 2) {
        manager->gfk[index].width = GetPOTSize(manager->gfk[index].width, manager->gfk[index].height);
        manager->gfk[index].height = manager->gfk[index].width;
    }

    manager->gfk[index].npotwidth = manager->gfk[index].width;
    manager->gfk[index].npotheight = manager->gfk[index].height;

    _SetLoadRect(manager, 20, 20 + 20 * index, 48, 40 + 20 * index); /// = 14%

    int num = anim_info.frame_count;
    long pixcount = manager->gfk[index].width * manager->gfk[index].height * 4;

    manager->gfk[index].frame = calloc(pixcount, 1);
    if (!manager->gfk[index].frame) {
        manager->error = MANAGER_WARN_FRAME_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 50, 40 + 20 * index); /// = 15%

    manager->gfk[index].buff = calloc(pixcount, 1);
    if (!manager->gfk[index].buff) {
        manager->error = MANAGER_WARN_BUFF_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 52, 40 + 20 * index); /// = 16%

    manager->gfk[index].delays = calloc(num, sizeof(float));
    if (!manager->gfk[index].delays) {
        manager->error = MANAGER_WARN_DELAYS_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 54, 40 + 20 * index); /// = 17%

    manager->gfk[index].lengths = calloc(num, sizeof(float));
    if (!manager->gfk[index].lengths) {
        manager->error = MANAGER_WARN_LENGTHS_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 56, 40 + 20 * index); /// = 18%

    manager->gfk[index].frame_points = calloc(num, sizeof(float) * 4);
    if (!manager->gfk[index].frame_points) {
        manager->error = MANAGER_WARN_FRAMEP_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 58, 40 + 20 * index); /// = 19%

    manager->gfk[index].textures = calloc(num, sizeof(GLuint));
    if (!manager->gfk[index].textures) {
        manager->error = MANAGER_WARN_TEXTURES_ALLOC;
        ManagerHandleError(manager);
        return 0;
    }

    _SetLoadRect(manager, 20, 20 + 20 * index, 60, 40 + 20 * index); /// = 20%

    int prev_stamp = 0, stamp = 0;
    while (WebPAnimDecoderHasMoreFrames(dec)) {
        if (!WebPAnimDecoderGetNext(dec, &manager->gfk[index].buff, &stamp)) {
            break;
        }

        manager->gfk[index].delays[manager->gfk[index].count] =
            (float)(stamp - prev_stamp) / 1000;
        if (!manager->gfk[index].delays[manager->gfk[index].count])
            manager->gfk[index].delays[manager->gfk[index].count] = 1;
        if (!manager->gfk[index].count) manager->gfk[index].lengths[0] =
            manager->gfk[index].delays[0];
        else manager->gfk[index].lengths[manager->gfk[index].count] =
            manager->gfk[index].lengths[manager->gfk[index].count - 1] +
            manager->gfk[index].delays[manager->gfk[index].count];

        for (int y = 0; y < manager->gfk[index].npotheight; y++) {
            for (int x = 0; x < manager->gfk[index].npotwidth; x++) {
                int src_idx = 4 * (y * manager->gfk[index].npotwidth + x);
                int dst_idx = 4 * (y * manager->gfk[index].width + x);
                memcpy(manager->gfk[index].frame + dst_idx, manager->gfk[index].buff + src_idx, 4);
            }
        }

        unsigned int pix = 0;
        for (long y = 0; y < manager->gfk[index].height; y++) {
            for (long x = 0; x < manager->gfk[index].width; x++) {
                if (manager->gfk[index].frame[pix + 3]) {
                    if (!manager->gfk[index].frame[pix]) manager->gfk[index].frame[pix]++;
                    if (!manager->gfk[index].frame[pix + 1]) manager->gfk[index].frame[pix + 1]++;
                    if (!manager->gfk[index].frame[pix + 2]) manager->gfk[index].frame[pix + 1]++;
                }
                pix += 4;
            }
        }

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

        manager->gfk[index].frame_points[0] = -0.9999f;
        manager->gfk[index].frame_points[1] = 0.9999f;
        manager->gfk[index].frame_points[2] = 1.0001f;
        manager->gfk[index].frame_points[3] = -1.0001f;
        manager->gfk[index].count++;
        prev_stamp = stamp;

        _SetLoadRect(manager, 20, 20 + 20 * index, (uint16_t)(61 + (float)manager->gfk[index].count / (float)num * (float)(198 - 61)), 40 + 20 * index); /// [21% ... 89%] [61 .. 198]
    }

    WebPAnimDecoderDelete(dec);

    manager->gfk[index].buff = NULL;
    if (manager->gfk[index].frame != NULL) { free(manager->gfk[index].frame); manager->gfk[index].frame = NULL; }

    return 1;
}

SINLINE uint8_t LoadFile(Manager* manager, uint16_t index, uint8_t filetype) {
    switch (filetype) {
        case GIF:
            return _LoadGIF(manager, index);
        case APNG:
            return _LoadAPNG(manager, index);
        case WEBP:
            return _LoadWEBP(manager, index);
        default:
            return NO_FORMAT;
    }
}

#endif // DATA_H_INCLUDED

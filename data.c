#include "data.h"

void DataInit(Data* dt)
{
    dt->textures = NULL;
    dt->data = NULL;
    dt->buff = NULL;
    dt->frame = NULL;
    dt->delays = NULL;
    dt->frame_points = NULL;

    dt->count = 0; dt->size = 0;
    dt->width = 0; dt->height = 0;
    dt->error = 0;
}

/// Just checking their headers
/// returns INCORRECT (0)
/// or >1 if FORMAT

uint8_t CheckFile(const char* filename, Data* dt)
{
    /** file, filesize, filedata **/
    FILE *file = fopen(filename, "rb");
    if (file == NULL) return 0;

    fseek(file, 0, SEEK_END);
    dt->size = ftell(file);

    fseek(file, 0, SEEK_SET);
    dt->data = malloc(dt->size);

    fread(dt->data, 1, dt->size, file);
    fclose(file); file = NULL;

    if (_CheckGIF(dt->data, dt->size)) return GIF;
    else if (_CheckAPNG(dt->data, dt->size)) return APNG;
    else if (_CheckWEBP(dt->data, dt->size)) return WEBP;
    else if (_CheckAVIF(dt->data, dt->size)) return AVIF;
    else if (_CheckMNG(dt->data, dt->size)) return MNG;
    else return INCORRECT;
}

uint8_t _CheckGIF(void* data, long size)
{
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

uint8_t _CheckAPNG(void* data, long size)
{
    if (png_sig_cmp((png_bytep)data, 0, 8)) return 0;

    return 1;
}

uint8_t _CheckWEBP(void* data, long size)
{
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

uint8_t _CheckAVIF(void* data, long size)
{
    avifDecoder* decoder = avifDecoderCreate();
    if (!decoder) return 0;

    avifResult result = avifDecoderSetIOMemory(decoder, (uint8_t *)data, size);
    if (result) return 0;

    result = avifDecoderParse(decoder);
    if (result) return 0;

    avifDecoderDestroy(decoder);

    return 1;
}

uint8_t _CheckMNG(void* data, long size) { return 0; }

/// sets to data->error

uint8_t LoadFile(Window* window, Settings* st, Data* dt, unsigned char filetype)
{
    switch (filetype)
    {
        case GIF:
            return _LoadGIF(window, st, dt);
        case APNG:
            return _LoadAPNG(window, st, dt);
        case WEBP:
            return _LoadWEBP(window, st, dt);
        case AVIF:
            return _LoadAVIF(window, st, dt);
        case MNG:
            return _LoadMNG(window, st, dt);
        default: return INCORRECT;
    }
}

typedef struct
{
    unsigned char* data;
    size_t size;
    size_t position;
} GIFReader;

int _gifr(GifFileType* gif, GifByteType* bytes, int size)
{
    GIFReader* reader = (GIFReader*)gif->UserData;
    size_t available = reader->size - reader->position;
    size_t to_read = (size < available) ? size : available;

    if (to_read > 0) {
        memcpy(bytes, reader->data + reader->position, to_read);
        reader->position += to_read;
    }
    return to_read;
}

uint8_t _LoadGIF(Window* window, Settings* st, Data* dt)
{
    int gif_error = 0;

    GIFReader reader = { (uint8_t*)dt->data, dt->size, 0 } ;

    GifFileType* gif = NULL;
    gif = DGifOpen(&reader, _gifr, &gif_error);

    if (gif == NULL) {
        return (uint8_t)(gif_error - 99);
    }

    if (DGifSlurp(gif) != GIF_OK) {
        DGifCloseFile(gif, &gif_error);
        return 1;
    }

    dt->width = gif->SWidth;
    dt->height = gif->SHeight;
    int num = gif->ImageCount;
    long pixcount = (dt->width + 1) * (dt->height + 1) * 4;
    if (dt->frame == NULL) {
        dt->frame = malloc(pixcount);
        if (dt->frame == NULL) { gif_error = 15; goto _loadgif_release; }
        memset(dt->frame, 0, pixcount);
    }
    if (dt->buff == NULL) {
        dt->buff = malloc(pixcount);
        if (dt->buff == NULL) { gif_error = 15; goto _loadgif_release; }
        memset(dt->buff, 0, pixcount);
    }

    for (int i = 0; i < num; i++) {
        dt->count++;

        SavedImage savedImage = gif->SavedImages[i];
        GifImageDesc imageDesc = savedImage.ImageDesc;

        ColorMapObject* colorMap = imageDesc.ColorMap;
        if (colorMap == NULL)
            colorMap = gif->SColorMap;

        if (colorMap == NULL) {
            continue;
        }

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

        unsigned long index = ((dt->width + 1) * fryo) * 4;
        for (int y = 0; y < fryd; y++) {
            index += frxo * 4;
            for (int x = 0; x < frxd; x++) {
                int colorIndex = savedImage.RasterBits[y * frxd + x];

                if (colorIndex == transparent) {
                    index += 4;
                    continue;
                } else {
                    GifColorType color = colorMap->Colors[colorIndex];
                    dt->frame[index] = color.Red;
                    dt->frame[index + 1] = color.Green;
                    dt->frame[index + 2] = color.Blue;
                    dt->frame[index + 3] = 255;
                }

                if (dt->frame[index] == 0 && dt->frame[index + 1] == 0 &&
                    dt->frame[index + 2] == 0 && dt->frame[index + 3] > 0) {
                    dt->frame[index] = 1;
                    dt->frame[index + 1] = 1;
                    dt->frame[index + 2] = 1;
                }
                index += 4;
            }
            index += ((dt->width + 1) - frxd - frxo) * 4;
        }

        if (gcb.DisposalMode != DISPOSE_PREVIOUS) memcpy(dt->buff, dt->frame, pixcount);

        dt->delays = (float *)realloc(dt->delays, sizeof(float) * dt->count);
        if (dt->delays == NULL) { gif_error = 15; goto _loadgif_release; }
        dt->delays[i] = (gcb.DelayTime == 0) ? 0.1 : (float)gcb.DelayTime / 100;

        _GLImage(window, dt);
        ShowLoadLine(window, dt, st, (float)dt->count / (float)num);

        switch (gcb.DisposalMode)
        {
            case DISPOSE_BACKGROUND:
            {
                /// Cleaning frame`s region
                unsigned long index = ((dt->width + 1) * fryo) * 4;
                for (int y = 0; y < fryd; y++) {
                    index += frxo * 4;
                    for (int x = 0; x < frxd; x++) {
                        memset(&dt->frame[index], 0, 4);
                        index += 4;
                    }
                    index += ((dt->width + 1) - frxd - frxo) * 4;
                }
            }   break;
            case DISPOSE_PREVIOUS:
            {
                memcpy(dt->frame, dt->buff, pixcount);
            }   break;
            case DISPOSE_DO_NOT:
            default : break;
        }

        dt->frame_points = realloc(dt->frame_points, sizeof(float) * dt->count * 4);
        if (dt->frame_points == NULL) { gif_error = 15; goto _loadgif_release; }
        dt->frame_points[(dt->count - 1) * 4] = ((float)frxo) / ((float)_GetCollisionSize(dt->width, 1) - 1) * 2 - 0.9999f;
        dt->frame_points[(dt->count - 1) * 4 + 1] = -( ((float)fryo) / ((float)_GetCollisionSize(dt->height, 1) - 1) * 2 - 0.9999f );
        dt->frame_points[(dt->count - 1) * 4 + 2] = (float)(frxo + frxd) / ((float)_GetCollisionSize(dt->width, 1) - 1) * 2 - 1.0001f;
        dt->frame_points[(dt->count - 1) * 4 + 3] = -( (float)(fryo + fryd) / ((float)_GetCollisionSize(dt->height, 1) - 1) * 2 - 1.0001f );
    }

    if (dt->frame != NULL)          { free(dt->frame);          dt->frame = NULL; }
    if (dt->buff != NULL)           { free(dt->buff);           dt->buff = NULL; }

    DGifCloseFile(gif, &gif_error);

    return (uint8_t)gif_error;

_loadgif_release:
    if (gif != NULL) DGifCloseFile(gif, NULL);

    ClearMedia(dt);

    return (uint8_t)gif_error;
}

static void _pngr(png_structp png_ptr, png_bytep data, png_size_t length)
{
    void** data_ptr = (void**)png_get_io_ptr(png_ptr);
    unsigned char* current_pos = (unsigned char*)*data_ptr;

    memcpy(data, current_pos, length);
    *data_ptr = current_pos + length;
}

uint8_t _LoadAPNG(Window* window, Settings* st, Data* dt) /// or PNG
{
    uint8_t png_error = 0;

    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep* row_pointers = NULL;
    int rpa = 0;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        png_error = 64;
        goto _loadpng_release;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_error = 65;
        goto _loadpng_release;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_error = 66;
        goto _loadpng_release;
    }

    png_set_read_fn(png_ptr, &dt->data, _pngr);
    png_read_info(png_ptr, info_ptr);

    dt->width = png_get_image_width(png_ptr, info_ptr);
    dt->height = png_get_image_height(png_ptr, info_ptr);

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

    long pixcount = (dt->width + 1) * (dt->height + 1) * 4;
    if (dt->frame == NULL) {
        dt->frame = malloc(pixcount);
        if (dt->frame == NULL) { png_error = 15; goto _loadpng_release; }
        memset(dt->frame, 0, pixcount);
    }
    if (dt->buff == NULL) {
        dt->buff = malloc(pixcount);
        if (dt->buff == NULL) { png_error = 15; goto _loadpng_release; }
        memset(dt->buff, 0, pixcount);
    }

    int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    int num = png_get_num_frames(png_ptr, info_ptr);
    row_pointers = malloc(dt->height * sizeof(png_bytep));
    if (!row_pointers) {
        png_error = 67;
        goto _loadpng_release;
    }

    for (rpa = 0; rpa < dt->height; rpa++) {
        row_pointers[rpa] = malloc(row_bytes);
        if (!row_pointers[rpa]) {
            png_error = 68;
            goto _loadpng_release;
        }
    }

    png_uint_16 delay, den;
    png_uint_32 frxo, fryo, frxd, fryd;
    png_byte mode, blend;

    png_byte prev_mode = 0;
    if (num > 1) {
        for (int i = 0; i < num; i++) {
            dt->count++;

            switch (prev_mode)
            {
                /// APNG_DISPOSE_OP_NONE: no disposal is done on this frame before
                /// rendering the next; the contents of the output buffer are left as is

                /// APNG_DISPOSE_OP_BACKGROUND:
                /// the frame's region of the output buffer is to be cleared
                /// to fully transparent black before rendering the next frame
                case PNG_DISPOSE_OP_BACKGROUND:
                {
                    unsigned long index = ((dt->width + 1) * fryo) * 4;
                    for (long y = 0; y < fryd; y++) {
                        index += frxo * 4;
                        for (long x = 0; x < frxd; x++) {
                            memset(&dt->frame[index], 0, 4);

                            index += 4;
                        }
                        index += ((dt->width + 1) - frxd - frxo) * 4;
                    }
                }   break;
                /// APNG_DISPOSE_OP_PREVIOUS: the frame's region of the output
                /// buffer is to be reverted to the previous contents before
                /// rendering the next frame
                case PNG_DISPOSE_OP_PREVIOUS:
                {
                    unsigned long index = ((dt->width + 1) * fryo) * 4;
                    for (long y = 0; y < fryd; y++) {
                        index += frxo * 4;
                        for (long x = 0; x < frxd; x++) {
                            memcpy(&dt->frame[index], &dt->buff[index], 4);

                            index += 4;
                        }
                        index += ((dt->width + 1) - frxd - frxo) * 4;
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

            if (!den) den = 100;
            dt->delays = (float *)realloc(dt->delays, sizeof(float) * dt->count);
            if (dt->delays == NULL) { png_error = 15; goto _loadpng_release; }
            dt->delays[i] = (den == 0) ? 0.1 : (float)delay / (float)den;

            switch (blend)
            {
                /// APNG_BLEND_OP_SOURCE all color components of the frame,
                /// including alpha, overwrite the current contents of the frame's
                /// output buffer region
                case PNG_BLEND_OP_SOURCE:
                {
                    unsigned long index = ((dt->width + 1) * fryo) * 4;
                    for (long y = 0; y < fryd; y++) {
                        index += frxo * 4;

                        for (long x = 0; x < frxd; x++) {
                            dt->frame[index]     = row_pointers[y][x * 4];
                            dt->frame[index + 1] = row_pointers[y][x * 4 + 1];
                            dt->frame[index + 2] = row_pointers[y][x * 4 + 2];
                            dt->frame[index + 3] = row_pointers[y][x * 4 + 3];

                            /// RGBA(0, 0, 0, >0) -> RGBA(1, 1, 1, >0) for compatibility with OpenGL
                            if (dt->frame[index]     == 0 && dt->frame[index + 3]) dt->frame[index]++;
                            if (dt->frame[index + 1] == 0 && dt->frame[index + 3]) dt->frame[index + 1]++;
                            if (dt->frame[index + 2] == 0 && dt->frame[index + 3]) dt->frame[index + 2]++;

                            index += 4;
                        }
                        index += ((dt->width + 1) - frxd - frxo) * 4;
                    }
                }   break;
                /// APNG_BLEND_OP_OVER the frame should be composited onto the output
                /// buffer based on its alpha, using a simple OVER operation
                case PNG_BLEND_OP_OVER:
                {
                    unsigned long index = ((dt->width + 1) * fryo) * 4;
                    for (long y = 0; y < fryd; y++) {
                        index += frxo * 4;
                        png_bytep row = row_pointers[y];

                        for (int x = 0; x < frxd; x++) {
                            png_bytep px = &(row[x * 4]);

                            float src_r = (float)px[0];
                            float src_g = (float)px[1];
                            float src_b = (float)px[2];
                            float src_a = (float)px[3] / 255.0f;

                            float dst_r = (float)dt->frame[index];
                            float dst_g = (float)dt->frame[index + 1];
                            float dst_b = (float)dt->frame[index + 2];
                            float dst_a = (float)dt->frame[index + 3] / 255.0f;

                            float out_a = src_a + dst_a * (1.0f - src_a);

                            if (out_a > 0.0f) {
                                unsigned char r = (unsigned char)((src_r * src_a + dst_r * dst_a * (1.0f - src_a)) / out_a);
                                unsigned char g = (unsigned char)((src_g * src_a + dst_g * dst_a * (1.0f - src_a)) / out_a);
                                unsigned char b = (unsigned char)((src_b * src_a + dst_b * dst_a * (1.0f - src_a)) / out_a);

                                if (r > 255) r = 255;
                                if (g > 255) g = 255;
                                if (b > 255) b = 255;

                                dt->frame[index]     = r;
                                dt->frame[index + 1] = g;
                                dt->frame[index + 2] = b;
                            } else {
                                dt->frame[index]     = 0;
                                dt->frame[index + 1] = 0;
                                dt->frame[index + 2] = 0;
                            }

                            dt->frame[index + 3] = (unsigned char)(out_a * 255.0f);

                            /// RGBA(0, 0, 0, >0) -> RGBA(1, 1, 1, >0) for compatibility with OpenGL
                            if (dt->frame[index]     == 0 && dt->frame[index + 3] > 0) dt->frame[index]++;
                            if (dt->frame[index + 1] == 0 && dt->frame[index + 3] > 0) dt->frame[index + 1]++;
                            if (dt->frame[index + 2] == 0 && dt->frame[index + 3] > 0) dt->frame[index + 2]++;

                            index += 4;
                        }
                        index += ((dt->width + 1) - frxd - frxo) * 4;
                    }
                }   break;
            }

            /// Writing data into buff.
            /// It will be used to return last frame when current is PNG_DISPOSE_OP_PREVIOUS
            memcpy(dt->buff, dt->frame, pixcount);

            dt->frame_points = realloc(dt->frame_points, sizeof(float) * dt->count * 4);
            if (dt->frame_points == NULL) { png_error = 15; goto _loadpng_release; }
            dt->frame_points[(dt->count - 1) * 4] = ((float)frxo) / ((float)_GetCollisionSize(dt->width, 1) - 1) * 2 - 0.9999f;
            dt->frame_points[(dt->count - 1) * 4 + 1] = -( ((float)fryo) / ((float)_GetCollisionSize(dt->height, 1) - 1) * 2 - 0.9999f );
            dt->frame_points[(dt->count - 1) * 4 + 2] = (float)(frxo + frxd) / ((float)_GetCollisionSize(dt->width, 1) - 1) * 2 - 1.0001f;
            dt->frame_points[(dt->count - 1) * 4 + 3] = -( (float)(fryo + fryd) / ((float)_GetCollisionSize(dt->height, 1) - 1) * 2 - 1.0001f );

            _GLImage(window, dt);
            ShowLoadLine(window, dt, st, (float)dt->count / (float)num);

            prev_mode = mode;
        }
    }
    else {
        dt->count++;

        png_read_image(png_ptr, row_pointers);
        row_bytes = png_get_rowbytes(png_ptr, info_ptr);

        den = 100;
        dt->delays = (float *)realloc(dt->delays, sizeof(float) * dt->count);
        if (dt->delays == NULL) { png_error = 15; goto _loadpng_release; }
        dt->delays[0] = 0.1;

        unsigned long index = 0;
        for (long y = 0; y < dt->height; y++) {
            png_bytep row = row_pointers[y];

            for (long x = 0; x < dt->width; x++) {
                dt->frame[index]     = row[x * 4];
                dt->frame[index + 1] = row[x * 4 + 1];
                dt->frame[index + 2] = row[x * 4 + 2];
                dt->frame[index + 3] = row[x * 4 + 3];

                if (dt->frame[index]     == 0 && dt->frame[index + 3] > 0) dt->frame[index]++;
                if (dt->frame[index + 1] == 0 && dt->frame[index + 3] > 0) dt->frame[index + 1]++;
                if (dt->frame[index + 2] == 0 && dt->frame[index + 3] > 0) dt->frame[index + 2]++;

                index += 4;
            }
            index += 4;
        }

        dt->frame_points = realloc(dt->frame_points, sizeof(float) * dt->count * 4);
        if (dt->frame_points == NULL) { png_error = 15; goto _loadpng_release; }
        dt->frame_points[0] = -0.9999f;
        dt->frame_points[1] = 0.9999f;
        dt->frame_points[2] = 1.0001f;
        dt->frame_points[3] = -1.0001f;

        _GLImage(window, dt);
        ShowLoadLine(window, dt, st, (float)dt->count / (float)num);
    }

    /// Free buffers
    for (int i = 0; i < dt->height; i++) {
        free(row_pointers[i]);
    }
    free(row_pointers);

    if (dt->frame != NULL)  { free(dt->frame);  dt->frame = NULL; }
    if (dt->buff != NULL)   { free(dt->buff);   dt->buff = NULL; }

    /// Free structures
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    return png_error;

_loadpng_release:
    if (row_pointers) {
        for (int j = 0; j < rpa; j++) {
            if (row_pointers[j]) free(row_pointers[j]);
        }
        free(row_pointers);
    }

    if (row_pointers != NULL) free(row_pointers);
    row_pointers = NULL;
    ClearMedia(dt);

    if (png_ptr || info_ptr) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    }

    return png_error;
}

uint8_t _LoadWEBP(Window* window, Settings* st, Data* dt)
{
    uint8_t webp_error = 0;
    WebPAnimDecoder* dec = NULL;

    WebPData webp_data = { dt->data, (size_t)dt->size };

    if (!WebPGetInfo(dt->data, dt->size, NULL, NULL)) {
        webp_error = 128;
        goto _loadwebp_release;
    }

    WebPAnimDecoderOptions dec_options;
    if (!WebPAnimDecoderOptionsInit(&dec_options)) {
        webp_error = 129;
        goto _loadwebp_release;
    }
    WebPAnimDecoderOptionsInit(&dec_options);
    dec_options.color_mode = MODE_RGBA;
    dec = WebPAnimDecoderNew(&webp_data, &dec_options);
    if (dec == NULL) {
        VP8StatusCode status = WebPGetFeatures(dt->data, dt->size, NULL);
        switch (status) {
            case VP8_STATUS_OUT_OF_MEMORY:
                webp_error = 130;
                break;
            case VP8_STATUS_INVALID_PARAM:
                webp_error = 131;
                break;
            case VP8_STATUS_BITSTREAM_ERROR:
                webp_error = 132;
                break;
            case VP8_STATUS_UNSUPPORTED_FEATURE:
                webp_error = 133;
                break;
            case VP8_STATUS_SUSPENDED:
                webp_error = 134;
                break;
            case VP8_STATUS_USER_ABORT:
                webp_error = 135;
                break;
            case VP8_STATUS_NOT_ENOUGH_DATA:
                webp_error = 136;
                break;
            default:
                webp_error = 137;
                break;
        }
        goto _loadwebp_release;
    }

    WebPAnimInfo anim_info;
    if (!WebPAnimDecoderGetInfo(dec, &anim_info)) {
        webp_error = 138;
        goto _loadwebp_release;
    }
    WebPAnimDecoderGetInfo(dec, &anim_info);

    dt->width = anim_info.canvas_width;
    dt->height = anim_info.canvas_height;
    int num = anim_info.frame_count;

    long pixcount = (dt->width + 1) * (dt->height + 1) * 4;
    if (dt->frame == NULL) {
        dt->frame = malloc(pixcount);
        if (dt->frame == NULL) { webp_error = 15; goto _loadwebp_release; }
        memset(dt->frame, 0, pixcount);
    }
    if (dt->buff == NULL) {
        dt->buff = malloc(pixcount);
        if (dt->buff == NULL) { webp_error = 15; goto _loadwebp_release; }
        memset(dt->buff, 0, pixcount);
    }

    memset(dt->frame, 0, pixcount);

    int prev_stamp = 0;
    while (WebPAnimDecoderHasMoreFrames(dec)) {
        dt->count++;

        int stamp;
        if (!WebPAnimDecoderGetNext(dec, &dt->buff, &stamp)) {
            break;
        }

        dt->delays = (float *)realloc(dt->delays, sizeof(float) * dt->count);
        if (dt->delays == NULL) { webp_error = 15; goto _loadwebp_release; }
        dt->delays[dt->count - 1] = (float)(stamp - prev_stamp) / 1000;

        for (int y = 0; y < dt->height; y++) {
            for (int x = 0; x < dt->width; x++) {
                int src_idx = 4 * (y * dt->width + x);
                int dst_idx = 4 * (y * (dt->width + 1) + x);
                memcpy(dt->frame + dst_idx, dt->buff + src_idx, 4);
            }
        }

        unsigned int index = 0;
        for (long y = 0; y < dt->height; y++) {
            for (long x = 0; x < dt->width; x++) {
                if (dt->frame[index + 3]) {
                    if (!dt->frame[index]) dt->frame[index]++;
                    if (!dt->frame[index + 1]) dt->frame[index]++;
                    if (!dt->frame[index + 2]) dt->frame[index]++;
                }
                index += 4;
            }
        }

        dt->frame_points = realloc(dt->frame_points, sizeof(float) * dt->count * 4);
        if (dt->frame_points == NULL) { webp_error = 15; goto _loadwebp_release; }
        dt->frame_points[(dt->count - 1) * 4] = -0.9999f;
        dt->frame_points[(dt->count - 1) * 4 + 1] = 0.9999f;
        dt->frame_points[(dt->count - 1) * 4 + 2] = 0.9999f;
        dt->frame_points[(dt->count - 1) * 4 + 3] = -0.9999f;

        prev_stamp = stamp;
        _GLImage(window, dt);
        ShowLoadLine(window, dt, st, (float)dt->count / (float)num);
    }

    WebPAnimDecoderDelete(dec);

    /** dt->buff is already cleaned by decoder**/
    dt->buff = NULL;
    if (dt->frame != NULL)          { free(dt->frame);          dt->frame = NULL; }

    return webp_error;

_loadwebp_release:
    if (dec != NULL) {
        WebPAnimDecoderDelete(dec);
    }

    ClearMedia(dt);

    return webp_error;
}

uint8_t _LoadAVIF(Window* window, Settings* st, Data* dt)
{
    uint8_t avif_error = 0;
    avifDecoder* decoder = NULL;
    decoder = avifDecoderCreate();
    decoder = avifDecoderCreate();
    if (decoder == NULL) {
        avif_error = 192;
        goto _loadavif_release;
    }

    avifResult result = avifDecoderSetIOMemory(decoder, (uint8_t *)dt->data, dt->size);
    if (result != AVIF_RESULT_OK) {
        avif_error = 193;
        goto _loadavif_release;
    }

    result = avifDecoderParse(decoder);
    if (result != AVIF_RESULT_OK) {
        switch (result) {
            case AVIF_RESULT_NO_CONTENT:
                avif_error = 194; break;
            case AVIF_RESULT_NO_YUV_FORMAT_SELECTED:
                avif_error = 195; break;
            case AVIF_RESULT_REFORMAT_FAILED:
                avif_error = 196; break;
            case AVIF_RESULT_UNSUPPORTED_DEPTH:
                avif_error = 197; break;
            case AVIF_RESULT_BMFF_PARSE_FAILED:
                avif_error = 198; break;
            case AVIF_RESULT_NO_AV1_ITEMS_FOUND:
                avif_error = 199; break;
            case AVIF_RESULT_DECODE_COLOR_FAILED:
                avif_error = 200; break;
            case AVIF_RESULT_DECODE_ALPHA_FAILED:
                avif_error = 201; break;
            case AVIF_RESULT_COLOR_ALPHA_SIZE_MISMATCH:
                avif_error = 202; break;
            case AVIF_RESULT_ISPE_SIZE_MISMATCH:
                avif_error = 203; break;
            case AVIF_RESULT_NO_CODEC_AVAILABLE:
                avif_error = 204; break;
            case AVIF_RESULT_NO_IMAGES_REMAINING:
                avif_error = 205; break;
            case AVIF_RESULT_INVALID_EXIF_PAYLOAD:
                avif_error = 206; break;
            case AVIF_RESULT_INVALID_IMAGE_GRID:
                avif_error = 207; break;
            case AVIF_RESULT_INVALID_CODEC_SPECIFIC_OPTION:
                avif_error = 208; break;
            case AVIF_RESULT_TRUNCATED_DATA:
                avif_error = 209; break;
            case AVIF_RESULT_IO_NOT_SET:
                avif_error = 210; break;
            case AVIF_RESULT_IO_ERROR:
                avif_error = 211; break;
            case AVIF_RESULT_WAITING_ON_IO:
                avif_error = 212; break;
            case AVIF_RESULT_INVALID_ARGUMENT:
                avif_error = 213; break;
            case AVIF_RESULT_NOT_IMPLEMENTED:
                avif_error = 214; break;
            case AVIF_RESULT_OUT_OF_MEMORY:
                avif_error = 215; break;
            case AVIF_RESULT_CANNOT_CHANGE_SETTING:
                avif_error = 216; break;
            case AVIF_RESULT_INCOMPATIBLE_IMAGE:
                avif_error = 217; break;
            default:
                avif_error = 219; break;
        }
        goto _loadavif_release;
    }


    avifRGBImage rgb;

    dt->width = decoder->image->width;
    dt->height = decoder->image->height;
    int num = decoder->imageCount;

    long pixcount = (dt->width + 1) * (dt->height + 1) * 4;
    if (dt->frame == NULL) {
        dt->frame = malloc(pixcount);
        if (dt->frame == NULL) { avif_error = 15; goto _loadavif_release; }
        memset(dt->frame, 0, pixcount);
    }
    memset(dt->frame, 0, pixcount);

    for (int i = 0; i < num; i++) {
        dt->count++;
        dt->delays = (float *)realloc(dt->delays, sizeof(float) * dt->count);
        if (dt->delays == NULL) { avif_error = 15; goto _loadavif_release; }
        dt->delays[i] = (float)decoder->imageTiming.duration;
        dt->frame_points = realloc(dt->frame_points, sizeof(float) * dt->count * 4);
        if (dt->delays == NULL) { avif_error = 15; goto _loadavif_release; }

        result = avifDecoderNextImage(decoder);
        avifRGBImageSetDefaults(&rgb, decoder->image);

        rgb.format = AVIF_RGB_FORMAT_RGBA;
        result = avifRGBImageAllocatePixels(&rgb);
        result = avifImageYUVToRGB(decoder->image, &rgb);
        if (result != AVIF_RESULT_OK) {
            avif_error = 218;
            goto _loadavif_release;
        }

        if (result != AVIF_RESULT_OK)
            break;

        for (int y = 0; y < dt->height; y++) {
            for (int x = 0; x < dt->width; x++) {
                int src_idx = 4 * (y * dt->width + x);
                int dst_idx = 4 * (y * (dt->width + 1) + x);

                dt->frame[dst_idx]     = rgb.pixels[src_idx]     ? rgb.pixels[src_idx] : 1;
                dt->frame[dst_idx + 1] = rgb.pixels[src_idx + 1] ? rgb.pixels[src_idx + 1] : 1;
                dt->frame[dst_idx + 2] = rgb.pixels[src_idx + 2] ? rgb.pixels[src_idx + 2] : 1;
                dt->frame[dst_idx + 3] = rgb.pixels[src_idx + 3] ? 255 : 0;
            }
        }

        dt->frame_points[(dt->count - 1) * 4] = -0.9999f;
        dt->frame_points[(dt->count - 1) * 4 + 1] = 0.9999f;
        dt->frame_points[(dt->count - 1) * 4 + 2] = 0.9999f;
        dt->frame_points[(dt->count - 1) * 4 + 3] = -0.9999f;

        _GLImage(window, dt);
        ShowLoadLine(window, dt, st, (float)dt->count / (float)num);
    }

    avifRGBImageFreePixels(&rgb);
    avifDecoderDestroy(decoder);

    if (dt->frame != NULL)          { free(dt->frame);          dt->frame = NULL; }

    return avif_error;

_loadavif_release:
    if (rgb.pixels != NULL)
        avifRGBImageFreePixels(&rgb);

    if (decoder != NULL)
        avifDecoderDestroy(decoder);

    ClearMedia(dt);

    return avif_error;
}

uint8_t _LoadMNG(Window* window, Settings* st, Data* dt) { return 0xE0; }


void _GLImage(Window* window, Data* dt)
{
    wglMakeCurrent(window->hdc, window->hrc);

    dt->textures = realloc(dt->textures, sizeof(GLuint) * dt->count);

    glGenTextures(1, &dt->textures[dt->count - 1]);
    glBindTexture(GL_TEXTURE_2D, dt->textures[dt->count - 1]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dt->width + 1, dt->height + 1,
                                0, GL_RGBA, GL_UNSIGNED_BYTE, dt->frame);

    glBindTexture(GL_TEXTURE_2D, 0);

    wglMakeCurrent(NULL, NULL);
}

void ClearMedia(Data* dt)
{
    if (dt->count > 0) glDeleteTextures(dt->count, dt->textures);

    if (dt->textures != NULL)       { free(dt->textures);       dt->textures = NULL; }
    if (dt->frame != NULL)          { free(dt->frame);          dt->frame = NULL; }
    if (dt->delays != NULL)         { free(dt->delays);         dt->delays = NULL; }
    if (dt->frame_points != NULL)   { free(dt->frame_points);   dt->frame_points = NULL; }

    dt->prev_mode = 0;
    dt->prev_frxo = 0;
    dt->prev_fryo = 0;
    dt->prev_frxd = 0;
    dt->prev_fryd = 0;
    dt->count = 0;
    dt->width = 0; dt->height = 0;
}

void ClearFileData(Data* dt)
{
    if (dt->data != NULL)       { free(dt->data);         dt->data = NULL; }

    dt->size = 0;
}

void ClearData(Data* dt)
{
    if (dt->count > 0) glDeleteTextures(dt->count, dt->textures);

    if (dt->data != NULL)           { free(dt->data);           dt->data = NULL; }

    if (dt->textures != NULL)       { free(dt->textures);       dt->textures = NULL; }
    if (dt->frame != NULL)          { free(dt->frame);          dt->frame = NULL; }
    if (dt->buff != NULL)           { free(dt->buff);           dt->buff = NULL; }
    if (dt->delays != NULL)         { free(dt->delays);         dt->delays = NULL; }
    if (dt->frame_points != NULL)   { free(dt->frame_points);   dt->frame_points = NULL; }

    dt->prev_mode = 0;
    dt->prev_frxo = 0;
    dt->prev_fryo = 0;
    dt->prev_frxd = 0;
    dt->prev_fryd = 0;
    dt->count = 0; dt->size = 0;
    dt->width = 0; dt->height = 0;
}

void ShowData(Data* dt)
{
    printf("Frames: %d, fs: %ld, Res: %dx%d\n", dt->count, dt->size, dt->width, dt->height);
}

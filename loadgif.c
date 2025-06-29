#include "loadgif.h"
#include "opengl_proc.h"
#include "window_proc.h"
#include "settings.h"

void *filedata = NULL;
long filesize = 0;
int filetype = 0;
GLuint *textures = NULL;
unsigned char *frame = NULL;
unsigned char *buff = NULL;
int width = 0;
int height = 0;
int checkwidth = 0;
int checkheight = 0;
int fc = 0;
double *delays = NULL;
int past_mode = 0;
int past_frxo = 0;
int past_fryo = 0;
int past_frxd = 0;
int past_fryd = 0;
int frames = 0;
WebPAnimDecoder* dec;

/**    Показывает полосу загрузки    **/

void LoadProgress() {
    BeginPaint(hwnd, &ps);

    HBRUSH hBrush = CreateSolidBrush(RGB(190, 190, 190));
    GetClientRect(hwnd, &rect);
    SelectObject(hdc, hBrush);

    Rectangle(hdc, -1, -1, CollisionWidth(), 30);
    hBrush = CreateSolidBrush(RGB(0, 255, 0));
    SelectObject(hdc, hBrush);
    Rectangle(hdc, -1, -1, (int)(((float)fc / (float)frames) * (float)CollisionWidth()), 30);

    DeleteObject(hBrush);
    EndPaint(hwnd, &ps);

    InvalidateRect(hwnd, NULL, FALSE);
}

/**    Генерирует текстуру для кадра    **/

void BindFrame() {
    glGenTextures(1, &textures[fc]);
    glBindTexture(GL_TEXTURE_2D, textures[fc]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width + 1, height + 1,
                                0, GL_RGBA, GL_UNSIGNED_BYTE, frame);

    glBindTexture(GL_TEXTURE_2D, 0);
}

/**    Записывает кадры в память    **/

void WriteGIFFrames(void *anim, struct GIF_WHDR *whdr) {
    textures = (GLuint *)realloc(textures, sizeof(GLuint) * (fc + 1));
    delays = (double *)realloc(delays, sizeof(double) * (fc + 1));
    *(delays + fc) = (whdr->time) ? (double)whdr->time / 100 : 0.1;

    unsigned int index = 0;

    /** GIF_NONE = 0, GIF_CURR = 1, GIF_BKGD = 2, GIF_PREV = 3 **/

    // printf("Frame: %d | mode: %d\n", fc, whdr->mode);

    switch (whdr->mode) {
        case GIF_NONE: {
            // memset(frame, 0, (width + 1) * (height * 1) * 4);
            for (long y = 0; y < whdr->fryd; y++) {
                index += whdr->frxo * 4;
                for (long x = 0; x < whdr->frxd; x++) {

                    uint8_t i = whdr->bptr[y * whdr->frxd + x];

                    if (whdr->tran != i) {
                        buff[index] = (whdr->cpal[i].R) ? whdr->cpal[i].R : whdr->cpal[i].R + 1;
                        buff[index + 1] = (whdr->cpal[i].G) ? whdr->cpal[i].G : whdr->cpal[i].G + 1;
                        buff[index + 2] = (whdr->cpal[i].B) ? whdr->cpal[i].B : whdr->cpal[i].B + 1;
                        buff[index + 3] = 255;
                    }

                    index += 4;
                }
                index += ((width + 1) - whdr->frxd - whdr->frxo) * 4;
            }
        }   break;
        case GIF_CURR: {
            for (long y = 0; y < whdr->fryd; y++) {
                index += whdr->frxo * 4;
                for (long x = 0; x < whdr->frxd; x++) {

                    uint8_t i = whdr->bptr[y * whdr->frxd + x];

                    if (whdr->tran != i) {
                        buff[index] = (whdr->cpal[i].R) ? whdr->cpal[i].R : whdr->cpal[i].R + 1;
                        buff[index + 1] = (whdr->cpal[i].G) ? whdr->cpal[i].G : whdr->cpal[i].G + 1;
                        buff[index + 2] = (whdr->cpal[i].B) ? whdr->cpal[i].B : whdr->cpal[i].B + 1;
                        buff[index + 3] = 255;
                    }

                    index += 4;
                }
                index += ((width + 1) - whdr->frxd - whdr->frxo) * 4;
            }
            if (past_mode == GIF_BKGD) {
                if (past_fryo + past_fryd == whdr->fryo + whdr->fryd ||
                    past_frxo + past_frxd > whdr->frxo + whdr->frxd) {
                    for (long y = 0; y < whdr->fryd; y++) {
                        memset(frame + (width + 1) * y * 4 + whdr->frxo * 4, 0, whdr->frxd * 4);
                    }
                }
            }
        }   break;
        case GIF_BKGD: {
            index = ((width + 1) * whdr->fryo) * 4;
            uint8_t bg_r = whdr->cpal[whdr->bkgd].R;
            uint8_t bg_g = whdr->cpal[whdr->bkgd].G;
            uint8_t bg_b = whdr->cpal[whdr->bkgd].B;

            if (past_mode == GIF_BKGD) {
                /** Очистка внутри кадра **/
                for (long y = 0; y < whdr->fryd; y++) {
                    memset(frame + (width + 1) * y * 4 + whdr->frxo * 4, 0, whdr->frxd * 4);
                }

                /** Очистка за кадром **/
                for (long y = 0; y < height + 1; y++) {
                    if (y < whdr->fryo)
                        /** Сверху **/
                        memset(frame + (width + 1) * y * 4, 0, (width + 1) * 4);
                    else if (y > whdr->fryo + whdr->fryd) {
                        /** Снизу **/
                        if (past_fryo + past_fryd != whdr->fryo + whdr->fryd)
                            memset(frame + (width + 1) * y * 4, 0, (width + 1) * 4);
                    }
                    else {
                        /** Слева **/
                        memset(frame + (width + 1) * y * 4, 0, whdr->frxo * 4);
                        /** Справа **/
                        if (past_frxo + past_frxd != whdr->frxo + whdr->frxd) {
                            memset(frame + (width + 1) * y * 4 + (whdr->frxo + whdr->frxd) * 4, 0, (width + 1 - whdr->frxo - whdr->frxd) * 4);
                        }
                    }
                }
            }

            for (long y = 0; y < whdr->fryd; y++) {
                index += whdr->frxo * 4;
                for (long x = 0; x < whdr->frxd; x++) {

                    uint8_t i = whdr->bptr[y * whdr->frxd + x];

                    if (whdr->tran != i) {
                        frame[index] =     bg_r;
                        frame[index + 1] = bg_g;
                        frame[index + 2] = bg_b;
                        frame[index + 3] = 255;
                    }
                    else if (past_mode == GIF_BKGD) frame[index + 3] = 0;

                    index += 4;
                }
                index += ((width + 1) - whdr->frxd - whdr->frxo) * 4;
            }
        }   break;
        case GIF_PREV: {
            if (buff != NULL) memcpy(frame, buff, (width + 1) * (height + 1) * 4);
            else memset(frame, 0, (width + 1) * (height * 1) * 4);
        }   break;
    }

    index = ((width + 1) * whdr->fryo) * 4;

    for (long y = 0; y < whdr->fryd; y++) {
        index += whdr->frxo * 4;
        for (long x = 0; x < whdr->frxd; x++) {

            uint8_t i = whdr->bptr[y * whdr->frxd + x];

            if (whdr->tran != i) {
                frame[index] = (whdr->cpal[i].R) ? whdr->cpal[i].R : whdr->cpal[i].R + 1;
                frame[index + 1] = (whdr->cpal[i].G) ? whdr->cpal[i].G : whdr->cpal[i].G + 1;
                frame[index + 2] = (whdr->cpal[i].B) ? whdr->cpal[i].B : whdr->cpal[i].B + 1;
                frame[index + 3] = 255;
            }

            index += 4;
        }
        index += ((width + 1) - whdr->frxd - whdr->frxo) * 4;
    }


    BindFrame();
    LoadProgress();

    past_mode = whdr->mode;
    past_frxo = whdr->frxo;
    past_fryo = whdr->fryo;
    past_frxd = whdr->frxd;
    past_fryd = whdr->fryd;
    fc++;
}

void WriteWEBPFrames() {
    WebPData webp_data = { filedata, (size_t)filesize };

    WebPAnimDecoderOptions dec_options;
    WebPAnimDecoderOptionsInit(&dec_options);
    dec_options.color_mode = MODE_RGBA;

    dec = WebPAnimDecoderNew(&webp_data, &dec_options);

    int prev_timestamp = 0;
    while (WebPAnimDecoderHasMoreFrames(dec)) {
        textures = (GLuint *)realloc(textures, sizeof(GLuint) * (fc + 1));
        delays = (double *)realloc(delays, sizeof(double) * (fc + 1));

        int timestamp;
        if (!WebPAnimDecoderGetNext(dec, &buff, &timestamp)) {
            printf("Error: Failed to decode frame\n");
            break;
        }

        *(delays + fc) = (double)(timestamp - prev_timestamp) / 1000;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int src_idx = 4 * (y * width + x);
                int dst_idx = 4 * (y * (width + 1) + x);
                memcpy(frame + dst_idx, buff + src_idx, 4);
            }
        }

        unsigned int index = 0;
        for (long y = 0; y < height; y++) {
            for (long x = 0; x < width; x++) {
                if (frame[index + 3]) {
                    if (!frame[index]) frame[index]++;
                    if (!frame[index + 1]) frame[index]++;
                    if (!frame[index + 2]) frame[index]++;
                }
                index += 4;
            }
        }


        BindFrame();
        LoadProgress();

        prev_timestamp = timestamp;
        fc++;
    }
    WebPAnimDecoderDelete(dec); dec = NULL;
}

void WritePNGFrame(const char *filename) {
    frames = 1;
    textures = (GLuint *)realloc(textures, sizeof(GLuint));
    delays = (double *)realloc(delays, sizeof(double));
    *(delays + fc) = 0.1;

    LoadProgress();

    FILE* fp = fopen(filename, "rb");

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);

    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
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

    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * (height));
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));
    }
    png_read_image(png_ptr, row_pointers);

    for (int y = 0; y < height; y++) {
        memcpy(frame + y * (width) * 4 + y * 4, row_pointers[y], (width) * 4);
    }

    for (int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }   free(row_pointers);

    BindFrame();

    png_read_end(png_ptr, NULL);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
}

void WriteAPNGFrames(const char *filename) {
    FILE* fp = fopen(filename, "rb");
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);

    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
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
    int num_frames = png_get_num_frames(png_ptr, info_ptr);
    frames = num_frames;

    png_uint_32 frame_width, frame_height;
    png_uint_32 x_offset, y_offset;
    png_uint_16 delay_num, delay_den;
    png_byte dispose_op, blend_op;

    png_uint_32 next_frame_width, next_frame_height;
    png_uint_32 next_x_offset, next_y_offset;
    png_uint_16 next_delay_num, next_delay_den;
    png_byte next_dispose_op, next_blend_op;

    png_uint_32 bkg_frame_width = 0;
    png_uint_32 bkg_x_offset = 0;

    int past_dispose_op = 0, past_blend_op = 0;

    for (int i = 0; i < num_frames; i++) {
        /**
            dispose_op:

            PNG_DISPOSE_OP_NONE (0)

            PNG_DISPOSE_OP_BACKGROUND (1)

            PNG_DISPOSE_OP_PREVIOUS (2)

            blend_op:

            PNG_BLEND_OP_SOURCE (0)

            PNG_BLEND_OP_OVER (1)
        **/

        textures = (GLuint *)realloc(textures, sizeof(GLuint) * (i + 1));
        delays = (double *)realloc(delays, sizeof(double) * (i + 1));

        if (i == 0) {
            png_read_frame_head(png_ptr, info_ptr);

            png_get_next_frame_fcTL(png_ptr, info_ptr, &frame_width, &frame_height,
                                    &x_offset, &y_offset, &delay_num, &delay_den,
                                    &dispose_op, &blend_op);
        }
        else {
            frame_width = next_frame_width; frame_height = next_frame_height;
            x_offset = next_x_offset; y_offset = next_y_offset;
            delay_num = next_delay_num; delay_den = next_delay_den;
            dispose_op = next_dispose_op; blend_op = next_blend_op;
        }

        *(delays + i) = (delay_den == 0) ? 0.1 : (double)delay_num / delay_den;

        png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
        size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
        for (int y = 0; y < height; y++) {
            row_pointers[y] = (png_byte*)malloc(rowbytes);
        }

        png_read_image(png_ptr, row_pointers);

        if (i < num_frames - 1) {
            png_read_frame_head(png_ptr, info_ptr);

            png_get_next_frame_fcTL(png_ptr, info_ptr, &next_frame_width, &next_frame_height,
                                    &next_x_offset, &next_y_offset, &next_delay_num, &next_delay_den,
                                    &next_dispose_op, &next_blend_op);
        }

        unsigned int index = 0;

        switch (dispose_op) {
            case PNG_DISPOSE_OP_NONE: {
                if (dispose_op == 0 && blend_op == 0 && next_dispose_op == 0 && next_blend_op == 0 && past_dispose_op == 0 && past_blend_op == 1) {
                    /** Очистка за кадром **/
                    for (long y = 0; y < height + 1; y++) {
                        if (y < y_offset)
                            /** Сверху **/
                            memset(frame + (width + 1) * y * 4, 0, (width + 1) * 4);
                        else if (y > y_offset + frame_height) {
                            /** Снизу **/
                            memset(frame + (width + 1) * y * 4, 0, (width + 1) * 4);
                        }
                        else {
                            /** Слева **/
                            memset(frame + (width + 1) * y * 4, 0, x_offset * 4);
                            /** Справа **/
                            memset(frame + (width + 1) * y * 4 + (x_offset + frame_width) * 4, 0, (width + 1 - x_offset - frame_width) * 4);
                        }
                    }
                }
            }   break;
            case PNG_DISPOSE_OP_BACKGROUND: {
                bkg_frame_width = frame_width;
                bkg_x_offset = x_offset;

                /** Очистка внутри кадра **/
                if (!(dispose_op == 1 && blend_op == 1 && next_dispose_op == 1 && next_blend_op == 0 && past_dispose_op == 2 && past_blend_op == 0)) {
                    index = ((width + 1) * y_offset) * 4;
                    for (long y = 0; y < frame_height; y++) {
                        index += x_offset * 4;
                        for (int x = 0; x < frame_width; x++) {
                            if (x + x_offset < next_x_offset) {
                                frame[index] = 0;
                                frame[index + 1] = 0;
                                frame[index + 2] = 0;
                                frame[index + 3] = 0;
                                index += 4;
                            }
                        }
                        index += ((width + 1) - frame_width - x_offset) * 4;
                    }
                }
            }   break;
        }

        index = ((width + 1) * y_offset) * 4;
        for (long y = 0; y < frame_height; y++) {
            index += x_offset * 4;
            png_bytep row = row_pointers[y];

            for (int x = 0; x < frame_width; x++) {
                png_bytep px = &(row[x * 4]);

                if (blend_op == PNG_BLEND_OP_SOURCE) {
                    frame[index]     = px[0];
                    frame[index + 1] = px[1];
                    frame[index + 2] = px[2];
                    frame[index + 3] = px[3];
                }
                else {
                    if (!frame[index + 3]) {
                        frame[index]     = px[0];
                        frame[index + 1] = px[1];
                        frame[index + 2] = px[2];
                        frame[index + 3] = px[3];
                    }
                    else {
                        float sa = px[3] / 255.0f;
                        float da = frame[index + 3] / 255.0f;
                        float oa = sa + da * (1.0f - sa);

                        frame[index]     = (px[0] * sa + frame[index]     * da * (1.0f - sa)) / oa;
                        frame[index + 1] = (px[1] * sa + frame[index + 1] * da * (1.0f - sa)) / oa;
                        frame[index + 2] = (px[2] * sa + frame[index + 2] * da * (1.0f - sa)) / oa;
                        frame[index + 3] = oa * 255.0f;
                    }
                }

                if (!frame[index]     && frame[index + 3]) frame[index]++;
                if (!frame[index + 1] && frame[index + 3]) frame[index + 1]++;
                if (!frame[index + 2] && frame[index + 3]) frame[index + 2]++;

                index += 4;
            }
            index += ((width + 1) - frame_width - x_offset) * 4;
        }

        LoadProgress();
        BindFrame();


        if (dispose_op == PNG_DISPOSE_OP_BACKGROUND) {
            /** Очистка внутри кадра **/
            index = ((width + 1) * y_offset) * 4;
            for (long y = 0; y < frame_height; y++) {
                index += x_offset * 4;
                for (int x = 0; x < frame_width; x++) {
                    frame[index] = 0;
                    frame[index + 1] = 0;
                    frame[index + 2] = 0;
                    frame[index + 3] = 0;
                    index += 4;
                }
                index += ((width + 1) - frame_width - x_offset) * 4;
            }
        }

        if ((dispose_op == 2 && blend_op == 1 && past_dispose_op == 1 && past_blend_op == 0 && next_dispose_op == 0 && next_blend_op == 1)) {
            /** Очистка внутри кадра **/
            index = ((width + 1) * y_offset) * 4;
            for (long y = 0; y < frame_height - 1; y++) {
                index += x_offset * 4;
                for (int x = 0; x < frame_width; x++) {
                    if (x_offset + x > bkg_x_offset && bkg_x_offset + bkg_frame_width > x_offset + frame_width) {
                        frame[index] = 0;
                        frame[index + 1] = 0;
                        frame[index + 2] = 0;
                        frame[index + 3] = 0;
                    }
                    index += 4;
                }
                index += ((width + 1) - frame_width - x_offset) * 4;
            }
        }

        if ((dispose_op == 0 && blend_op == 1 && past_dispose_op == 2 && past_blend_op == 1 && next_dispose_op == 1 && next_blend_op == 0)) {
            /** Очистка за кадром **/
            for (long y = 0; y < height + 1; y++) {
                if (y < y_offset)
                    /** Сверху **/
                    memset(frame + (width + 1) * y * 4, 0, (width + 1) * 4);
                else if (y > y_offset + frame_height) {
                    /** Снизу **/
                    memset(frame + (width + 1) * y * 4, 0, (width + 1) * 4);
                }
                else {
                    /** Слева **/
                    memset(frame + (width + 1) * y * 4, 0, x_offset * 4);
                    /** Справа **/
                    memset(frame + (width + 1) * y * 4 + (x_offset + frame_width) * 4, 0, (width + 1 - x_offset - frame_width) * 4);
                }
            }
        }


        for (int y = 0; y < height; y++) {
            free(row_pointers[y]);
        }   free(row_pointers);

        past_dispose_op = dispose_op; past_blend_op = blend_op;
        fc++;
    }

    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
}

void WriteAVIFFrames() {
    avifDecoder *decoder = avifDecoderCreate();
    avifResult result = avifDecoderSetIOMemory(decoder, (uint8_t *)filedata, filesize);
    result = avifDecoderParse(decoder);

    avifRGBImage rgb;

    for (int i = 0; i < decoder->imageCount; i++) {
        textures = (GLuint *)realloc(textures, sizeof(GLuint) * (i + 1));
        delays = (double *)realloc(delays, sizeof(double) * (i + 1));

        result = avifDecoderNextImage(decoder);

        avifRGBImageSetDefaults(&rgb, decoder->image);
        rgb.format = AVIF_RGB_FORMAT_RGBA;

        result = avifRGBImageAllocatePixels(&rgb);
        result = avifImageYUVToRGB(decoder->image, &rgb);
        if (result != AVIF_RESULT_OK) {
            break;
        }

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int src_idx = 4 * (y * width + x);
                int dst_idx = 4 * (y * (width + 1) + x);

                frame[dst_idx] = rgb.pixels[src_idx] ? rgb.pixels[src_idx] : 1;
                frame[dst_idx + 1] = rgb.pixels[src_idx + 1] ? rgb.pixels[src_idx + 1] : 1;
                frame[dst_idx + 2] = rgb.pixels[src_idx + 2] ? rgb.pixels[src_idx + 2] : 1;
                frame[dst_idx + 3] = rgb.pixels[src_idx + 3] ? 255 : 0;
            }
        }

        BindFrame();
        LoadProgress();
        *(delays + i) = (double)decoder->imageTiming.duration;
        fc++;
    }

    avifRGBImageFreePixels(&rgb);
    avifDecoderDestroy(decoder);
}

/**   Check frames functions   **/

void CheckGIFFrames(void *data, struct GIF_WHDR *whdr) {
    width = whdr->xdim;
    height = whdr->ydim;
    frames++;
}

int CheckWEBPFrames() {
    WebPData webp_data = { filedata, (size_t)filesize };
    WebPAnimDecoderOptions dec_options;
    WebPAnimDecoderOptionsInit(&dec_options);
    dec_options.color_mode = MODE_RGBA;

    dec = WebPAnimDecoderNew(&webp_data, &dec_options);
    if (!dec) {
        return 0;
    }

    WebPAnimInfo anim_info;
    if (!WebPAnimDecoderGetInfo(dec, &anim_info)) {
        WebPAnimDecoderDelete(dec);
        return 0;
    }

    width = anim_info.canvas_width;
    height = anim_info.canvas_height;
    frames = (int)anim_info.frame_count;

    WebPAnimDecoderDelete(dec); dec = NULL;
    return 1;
}

int CheckAPNGFrames(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return 0;
    }

    png_byte header[8];
    if (fread(header, 1, 8, fp) != 8 || png_sig_cmp(header, 0, 8)) {
        fclose(fp);
        return 0;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(fp);
        return 0;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        return 0;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    png_uint_32 full_width = 0, full_height = 0;
    png_get_IHDR(png_ptr, info_ptr, &full_width, &full_height, NULL, NULL, NULL, NULL, NULL);
    width = (int)full_width;
    height = (int)full_height;

    png_uint_32 num_frames = 0, num_plays = 0;
    int is_apng = png_get_acTL(png_ptr, info_ptr, &num_frames, &num_plays);
    frames = (int)num_frames;

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);

    return is_apng ? APNG_FORMAT : PNG_FORMAT;
}

int CheckAVIFFrames() {
    avifDecoder *decoder = avifDecoderCreate();
    if (decoder == NULL) {
        return 0;
    }

    avifResult result = avifDecoderSetIOMemory(decoder, (uint8_t *)filedata, filesize);
    if (result != AVIF_RESULT_OK) {
        return 0;
    }

    result = avifDecoderParse(decoder);
    if (result != AVIF_RESULT_OK) {
        return 0;
    }
    frames = decoder->imageCount;
    width = decoder->image->width;
    height = decoder->image->height;

    avifDecoderDestroy(decoder);
    return 1;
}

/**   Loads file to memory   **/

void LoadFile(const char *filename, int type) {
    if (fc) { glDeleteTextures(fc + 1, textures); } fc = 0;
    if (delays != NULL) { free(delays); delays = NULL; }
    if (textures != NULL) { free(textures); textures = NULL; }

    frame = (unsigned char *)malloc((width + 1) * (height + 1) * 4);
    buff = (unsigned char *)malloc((width + 1) * (height + 1) * 4);
    memset(frame, 0, (width + 1) * (height + 1) * 4);

    switch (type) {
        case GIF_FORMAT: {
            memset(buff, 0, (width + 1) * (height + 1) * 4);
            GIF_Load(filedata, filesize, WriteGIFFrames, NULL, NULL, 0);
        }   break;
        case WEBP_FORMAT: {
            WriteWEBPFrames();
        }   break;
        case PNG_FORMAT: {
            WritePNGFrame(filename);
        }   break;
        case APNG_FORMAT: {
            WriteAPNGFrames(filename);
        }   break;
        case AVIF_FORMAT: {
            WriteAVIFFrames();
        }   break;
    }

    if (filedata != NULL) { free(filedata); filedata = NULL; }
    if (frame != NULL) { free(frame); frame = NULL; }
    if (type != WEBP_FORMAT) { free(buff); buff = NULL; }
}

/**   Check if extension exists and is correct format   **/

int CheckFile(const char *filename) {
    int type = CheckAPNGFrames(filename);
    if (type) return type;

    frames = 0;

    FILE *file = fopen(filename, "rb");
    if (file == NULL) return 0;

    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    filedata = malloc(filesize);

    fread(filedata, 1, filesize, file);
    fclose(file); file = NULL;

    if (GIF_Load(filedata, filesize, CheckGIFFrames, NULL, NULL, 0)) {
        return GIF_FORMAT;
    }
    else if (CheckWEBPFrames()) {
        return WEBP_FORMAT;
    }
    else if (CheckAVIFFrames()) {
        return AVIF_FORMAT;
    }
    else if (0) { // MNG
        return 0;
    }
    else {
        free(filedata); filedata = NULL;
        return 0;
    }
}



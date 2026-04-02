#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include <stdint.h>
#include "tjpgd.h"

#include "images.h"

typedef struct {
    const uint8_t *data;
    size_t size;
    size_t index;
} InputMemory;

typedef struct {
    InputMemory in;
    RGBImage out;
} Context;

/* Input function (reads from file) */
static size_t in_func(JDEC *jd, uint8_t *buff, size_t nbyte) {
    Context *ctx = (Context*)jd->device;
    InputMemory *in = &ctx->in;
    printf("Read %u bytes (idx=%zu)\n", nbyte, in->index);

    if (in->index >= in->size) return 0;

    size_t remain = in->size - in->index;
    if (nbyte > remain) nbyte = remain;

    if (buff) {
        memcpy(buff, in->data + in->index, nbyte);
    }

    in->index += nbyte;

    return nbyte;
}

/* Output function (called for each decoded block) */
static int out_func(JDEC *jd, void *bitmap, JRECT *rect) {
    Context *ctx = (Context*)jd->device;
    RGBImage *img = &ctx->out;

    uint8_t *src = (uint8_t*)bitmap;

    for (int y = rect->top; y <= rect->bottom; y++) {
        for (int x = rect->left; x <= rect->right; x++) {

            int idx = y * SSTV_WIDTH + x;

            img->r[idx] = src[0];
            img->g[idx] = src[1];
            img->b[idx] = src[2];

            src += 3;
        }
    }
    return 1;
}

void decode_image(RGBImage *img, const uint8_t *encoded_buff, size_t size) {
    printf("decode start\n");
    /* Work buffer required by tjpgd */
    uint8_t work[4096];

    JDEC jd;
    JRESULT res;

    Context ctx;

    ctx.in.data = encoded_buff;
    ctx.in.size = size;
    ctx.in.index = 0;

    /* Prepare decoder */
    res = jd_prepare(&jd, in_func, work, sizeof(work), &ctx);

    printf("%d\n", res);
    hard_assert(res == JDR_OK);
    hard_assert(jd.width == SSTV_WIDTH);
    hard_assert(jd.height == SSTV_HEIGHT);

    /* Pass image struct to output function */
    jd.device = img;

    /* Decompress */
    printf("%d %d\n", jd.width, jd.height);
    res = jd_decomp(&jd, out_func, 0);

    printf("%d\n", res);
    hard_assert(res == JDR_OK);

    /* Example: print first pixel */
    printf("First pixel R=%d G=%d B=%d\n",
           img->r[0], img->g[0], img->b[0]);

}
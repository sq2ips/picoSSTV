#include "tjpgd.h"
#include <stdlib.h>
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>

#include "images.h"
#include "sstv.h"

const uint8_t *jpeg_buff = NULL;
size_t jpeg_size = 0;
uint8_t *buff_out = NULL;

extern const uint8_t _binary_image_bin_start[];
extern const uint8_t _binary_image_bin_end[];
size_t _binary_image_bin_len = 0;

// Input function (reads from memory)
size_t input_func(JDEC* jd, uint8_t* buff, unsigned int nbyte) {
    static size_t offset = 0;

    if (buff) {
        unsigned int i;
        for (i = 0; i < nbyte && offset < jpeg_size; i++) {
            buff[i] = jpeg_buff[offset++];
        }
        return i;
    } else {
        // Skip bytes
        offset += nbyte;
        if (offset > jpeg_size) offset = jpeg_size;
        return nbyte;
    }
}

// Output function (writes RGB888 pixels)
int output_func(JDEC* jd, void* bitmap, JRECT* rect) {
    uint8_t* src = (uint8_t*)bitmap;

    for (int y = rect->top; y <= rect->bottom; y++) {
        for (int x = rect->left; x <= rect->right; x++) {
            int idx = (y * SSTV_WIDTH + x) * 3;

            buff_out[idx + 0] = *src++; // R
            buff_out[idx + 1] = *src++; // G
            buff_out[idx + 2] = *src++; // B
        }
    }
    return 1; // Continue decoding
}

void decode_image(uint8_t *out_buff, const uint8_t *in_buff, size_t size) {
    jpeg_buff = in_buff;
    jpeg_size = size;
    buff_out = out_buff;
    
    JDEC jdec;
    uint8_t work[4096]; // Slightly larger for RGB888

    // Prepare decoder
    if (jd_prepare(&jdec, input_func, work, sizeof(work), NULL) != JDR_OK) {
        printf("Decoder init failed\n");
    }

    // Decode JPEG
    if (jd_decomp(&jdec, output_func, 0) != JDR_OK) {
        printf("Decode failed\n");
    }else{
        printf("Decoded successfully!\n");
    }
}

uint8_t get_images_count(void){
    return *(uint8_t *)_binary_image_bin_start;
}

size_t get_image_data(uint8_t num, const uint8_t *buff){
    hard_assert(num<get_images_count());

    size_t size = 0;
    for(uint8_t i = 0; i<=num; i++){
        printf("%d %d %d %d", *(uint8_t *)(_binary_image_bin_start+1), *(uint8_t *)(_binary_image_bin_start+2), *(uint8_t *)(_binary_image_bin_start+3), *(uint8_t *)(_binary_image_bin_start+4));
        size = *(uint32_t *)(_binary_image_bin_start+1+size+i*4);
        printf("size: %lu\n", size);
    }

    buff = (uint8_t *)(_binary_image_bin_start+1+size+num*4+4);

    return size;
}
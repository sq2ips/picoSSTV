#ifndef _IMAGES_H
#define _IMAGES_H

void decode_image(uint8_t *out_buff, const uint8_t *in_buff, size_t size);
uint8_t get_images_count(void);
size_t get_image_data(uint8_t num, const uint8_t *buff);

#endif
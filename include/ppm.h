#ifndef PPM_H
#define PPM_H

#include <stdint.h>
#include <stdbool.h>


typedef struct {
    uint16_t r, g, b;
} Pixel;


typedef struct {
    size_t rows;
    size_t cols;
    unsigned int maxval;
    Pixel **pixels;
} PpmImage;


typedef struct {
    size_t rows;
    size_t cols;
    unsigned int maxval;
    uint8_t *data;
} PpmImageGray;


PpmImage *ppm_readimage(const char *filename); // allocate in the heap
void ppm_destroy(PpmImage* image);             // deallocate heap memory

PpmImageGray *ppm_read_gray_image(const char *filename); // allocate in the heap
void ppm_destroy_gray_image(PpmImageGray* image);             // deallocate heap memory

bool ppm_writeimage(const char *filename, PpmImage* image);
bool ppm_write_gray_image(const char *filename, PpmImageGray* gray_image);

bool ppm_image_to_ppm_gray_image(PpmImage* image, PpmImageGray* gray_image);

#endif


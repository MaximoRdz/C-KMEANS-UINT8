#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../include/ppm.h"

#define VALUES_PER_PIXEL 3


PpmImage *ppm_readimage(const char *filename)
{
    FILE *image = fopen(filename, "rb");

    if (image == NULL) {
        perror("ERROR opening image");
        return NULL; 
    }
    // get the file size
    fseek(image, 0, SEEK_END);
    size_t imagesize = ftell(image); 
    printf("Image size: %zu\n", imagesize);
    
    // file descriptor
    int file_desc = fileno(image);
    printf("File descriptor: %d\n", file_desc);

 
    // cursor position reset
    rewind(image);

    PpmImage *result = malloc(sizeof *result);
    if (result == NULL) {
        perror("Could not allocate result");
    }
    // read the ppm header
    char type[3] = { 0 };
    fscanf(image, "%2s", type);

    if (strcmp(type, "P6") != 0) {
        printf("ERROR reading image, expected format <P6>. Found format %s\n", type);
        fclose(image);
        free(result);
        return NULL;
    }

    // P6
    // #Compressed by jpeg-recompress
    // 1800 1200
    // 255

    // fscanf: reads formatted input from a stream. Extracting data from a file according to 
    // specified format.
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), image)) {
        if (buffer[0] == '#') continue;
        if (sscanf(buffer, "%zu %zu", &result->cols, &result->rows) == 2) break;
    }
    fscanf(image, "%u", &result->maxval);
    fgetc(image); // careful with not reading the \n char after the maxval.

    printf("INFO: image size -> rows: %zu cols: %zu\n", result->rows, result->cols);
    printf("INFO: image max val: %u\n", result->maxval);

    // mapping files into memory in C:
    // request blocks of memry and fill them with our file contents
    // off_t offset = ftell(image);
    // uint8_t *rawdata = mmap(NULL, imagesize, PROT_READ, MAP_PRIVATE, file_desc, offset);

    uint8_t bytes_per_value = (result->maxval > 256 ? 2 : 1);
    if (bytes_per_value != 1) {
        perror("Only support for values of 1 byte");
        free(result);
        fclose(image);
        return NULL;
    }
    size_t pixel_bytes = result->cols * result->rows * VALUES_PER_PIXEL * bytes_per_value;
    uint8_t *rawdata = malloc(pixel_bytes);
    if (!rawdata) {
        perror("Memory allocation failed");
        free(rawdata);
        free(result);
        fclose(image);
        return NULL;
    }

    fread(rawdata, 1, pixel_bytes, image); // read everything into a buffer


    uint8_t *pixeldata = rawdata;
    result->pixels = calloc(result->rows, sizeof(Pixel*));

    for (size_t i=0; i<result->rows; i++) {
        result->pixels[i] = calloc(result->cols, sizeof(Pixel));
        for (size_t j=0; j<result->cols; j++) {
            uint8_t *pixel_start = pixeldata + 
                (i * result->cols + j) * VALUES_PER_PIXEL * bytes_per_value;
            if (bytes_per_value == 1) {
               result->pixels[i][j].r = pixel_start[0];
               result->pixels[i][j].g = pixel_start[1];
               result->pixels[i][j].b = pixel_start[2];
            } else break;
        }
    }
    
    free(rawdata);
    fclose(image);
    return result;
};


void ppm_destroy(PpmImage* image)
{
    for (size_t i=0; i<image->rows; i++) {
        free(image->pixels[i]);
    }
    free(image->pixels);
    free(image);
}


bool ppm_writeimage(const char* filename, PpmImage* image)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("ERROR: opening file");
        return false;
    }
    fprintf(file, "P6\n%zu %zu\n%u\n", image->cols, image->rows, image->maxval);

    uint8_t bytes_per_value = (image->maxval > 256 ? 2 : 1);
    for (size_t i=0; i<image->rows; i++) {
        for (size_t j=0; j<image->cols; j++) {
            fwrite(&(image->pixels[i][j].r), bytes_per_value, 1, file);
            fwrite(&(image->pixels[i][j].g), bytes_per_value, 1, file);
            fwrite(&(image->pixels[i][j].b), bytes_per_value, 1, file);
        }
    }

    return true;
}


bool ppm_image_to_ppm_gray_image(PpmImage* image, PpmImageGray* gray_image)
{
    gray_image->rows = image->rows;
    gray_image->cols = image->cols;
    gray_image->maxval = image->maxval;
    gray_image->data = calloc(gray_image->rows * gray_image->cols, sizeof(uint8_t)); 

    if (gray_image->data == NULL) {
        perror("Could not allocate data elements in memory");
        return false;
    }

    for (size_t i=0; i<image->rows; i++) {
        for (size_t j=0; j<image->cols; j++) {
            gray_image->data[i*gray_image->cols + j] = (uint8_t)(
                    0.2125 * image->pixels[i][j].r +
                    0.7154 * image->pixels[i][j].g +
                    0.0721 * image->pixels[i][j].b 
                    );
        }
    }

    return true;
}

bool ppm_write_gray_image(const char* filename, PpmImageGray* gray_image)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error writing gray image to file");
        return false;
    }

    fprintf(file, "P5\n%zu %zu\n%u\n", gray_image->cols, gray_image->rows, gray_image->maxval);
    for (size_t i=0; i<gray_image->cols*gray_image->rows; i++) {
        fwrite(&(gray_image->data[i]), 1, 1, file);
    }
    fclose(file);
    return true;
}

void ppm_destroy_gray_image(PpmImageGray* gray_image)
{
    free(gray_image->data);
    free(gray_image);
}



#ifndef KMEANS_H
#define KMEANS_H

#include <stdint.h>


void kmeans_uint8(const uint8_t *data, size_t length, size_t k, uint8_t *output);

#endif

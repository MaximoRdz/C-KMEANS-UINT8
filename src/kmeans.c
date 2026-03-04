#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define KMEANS_MAX_ITERATIONS 1000


int squared_distance(uint8_t pointA, uint8_t pointB) {
    return ((int)pointA - (int)pointB) * ((int)pointA - (int)pointB);
}


double now_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void kmeans_uint8(const uint8_t *data, size_t length, size_t k, uint8_t *output)
{
    uint8_t *centroids = malloc(k * sizeof(uint8_t));
    if (centroids == NULL) {
        perror("Failed allocation for centroids array");
        return;
    }

    for (size_t i=0; i<k; ++i) centroids[i] = data[rand() % length];

    size_t *cluster_elements_counters = calloc(k, sizeof(size_t));
    if (cluster_elements_counters== NULL) {
        perror("Failed allocation for elements in clusters array");
        free(centroids);
        return;
    }
    
    size_t *cluster_elements_sums = calloc(k, sizeof(size_t));
    if (cluster_elements_sums== NULL) {
        perror("Failed allocation for elements clusters sum array");
        free(centroids);
        free(cluster_elements_counters);
        return;
    }

    uint8_t pixel_color, new_centroid_value;
    size_t closest_cluster, changed_centroids_count, iter=0;
    int current_min_distance, candidate_min_distance;
    double start = now_seconds(), end = 0;
    while (true) {
        memset(cluster_elements_counters, 0, k * sizeof(size_t));
        memset(cluster_elements_sums, 0, k * sizeof(size_t));

        // assign each pixel to nearest color (uint8) cluster.
        for (size_t i=0; i<length; ++i) {
            pixel_color = data[i];
            closest_cluster = 0;
            current_min_distance = squared_distance(pixel_color, centroids[closest_cluster]);
            
            for (size_t j=1; j<k; ++j) {
                candidate_min_distance = squared_distance(pixel_color, centroids[j]);
                if (candidate_min_distance < current_min_distance) {
                    current_min_distance = candidate_min_distance;
                    closest_cluster = j;
                }
            }
        
            // udpate counters: assigned cluster and cummulative sum.
            cluster_elements_counters[closest_cluster]++;
            cluster_elements_sums[closest_cluster] += pixel_color;
            output[i] = closest_cluster;
        }

        // recalculate centroids as the mean of each cluster.
        changed_centroids_count = 0;
        for (size_t i=0; i<k; ++i) {
            if (cluster_elements_counters[i] == 0) {
                centroids[i] = data[rand() % length];
                continue;
            }

            new_centroid_value = cluster_elements_sums[i] / cluster_elements_counters[i];

            if (centroids[i] != new_centroid_value) {
                changed_centroids_count++;
                centroids[i] = new_centroid_value;
            }
        }

        if (changed_centroids_count == 0) {
            end = now_seconds();
            break;
        }

        iter++;
        if (iter >= KMEANS_MAX_ITERATIONS) {
            end = now_seconds();
            printf("Loop terminated, max iterations exceeded!\n");
            break;
        }
    }
    printf("INFO: KMeans algorithm converged after %zu interations. Elapsed: %10.2f ms\n", iter, (end -start) * 1000.0);
    // reconstruct image with centroid value.
   for (size_t i=0; i<length; ++i) {
        output[i] = centroids[output[i]];
    }
        free(centroids);
        free(cluster_elements_counters);
        free(cluster_elements_sums);
}



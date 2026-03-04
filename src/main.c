#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>


#include "../include/raylib.h"
#include "../include/ppm.h"
#include "../include/kmeans.h"


/*Configuration structs for kmeans program subcommands.*/
typedef struct {
    int verbose;
    int k;
    char *input_path;
    char *output_path;
} ComputeConfig;

typedef struct {
    int verbose;
    char *input_path;
} InteractiveConfig;

/*Main Mode Execution Functions*/
typedef struct {
    PpmImageGray *original;
    PpmImageGray *clustered;

    Texture2D texture;

    int current_k;
    int requested_k;
    int show_raw;
} InteractiveApp;

void run_interactive_mode(InteractiveConfig cfg)
{

    InteractiveApp app = {0};

    PpmImage *image = ppm_readimage(cfg.input_path);
    if (!image) {
        fprintf(stderr, "Could not read input ppm image %s\n", cfg.input_path);
        return;
    }

    app.original = malloc(sizeof(*app.original));
    if (!app.original) {
        fprintf(stderr, "Allocation failed for original image\n");
        ppm_destroy(image);
        return;
    }

    if (!ppm_image_to_ppm_gray_image(image, app.original)) {
        ppm_destroy(image);
        free(app.original);
        return;
    }

    ppm_destroy(image);

    app.clustered = malloc(sizeof(*app.clustered));
    if (!app.clustered) {
        fprintf(stderr, "Allocation failed for clustered image\n");
        ppm_destroy_gray_image(app.original);
        return;
    }

    app.clustered->rows = app.original->rows;
    app.clustered->cols = app.original->cols;
    app.clustered->maxval = app.original->maxval;

    size_t total_pixels = app.original->rows * app.original->cols;

    app.clustered->data = malloc(total_pixels);
    if (!app.clustered->data) {
        fprintf(stderr, "Allocation failed for clustered data\n");
        ppm_destroy_gray_image(app.original);
        free(app.clustered);
        return;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1200, 800, "Interactive KMeans");
    SetTargetFPS(60);

    Image img = {
        .data = app.original->data,
        .width = app.original->cols,
        .height = app.original->rows,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE
    };

    app.texture = LoadTextureFromImage(img);

    app.current_k = 0;
    app.requested_k = 1;
    app.show_raw = 1;

    char title[64];

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_SPACE) && !app.show_raw) {
            app.current_k = 0;
            app.requested_k = 1; 
            app.show_raw = 1;
            UpdateTexture(app.texture, app.original->data);
        } else {
            if (IsKeyPressed(KEY_RIGHT)) {
                app.requested_k++;
                app.show_raw = 0;
            }

            if (IsKeyPressed(KEY_LEFT) && app.requested_k > 2) {
                app.requested_k--;
                app.show_raw = 0;
            }
            if (!app.show_raw && app.current_k != app.requested_k) {
                kmeans_uint8(
                    app.original->data,
                    total_pixels,
                    app.requested_k,
                    app.clustered->data
                );
                UpdateTexture(app.texture, app.clustered->data);
                app.current_k = app.requested_k;
            }
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawTexture(app.texture, 0, 0, WHITE);

            if (app.show_raw)
                snprintf(title, sizeof(title), "KMeans Raw");
            else
                snprintf(title, sizeof(title), "KMeans n=%d", app.current_k);

            DrawText(title, 10, 10, 30, ORANGE);

        EndDrawing();
    }
    UnloadTexture(app.texture);
    CloseWindow();

    ppm_destroy_gray_image(app.original);
    ppm_destroy_gray_image(app.clustered);
}


int compute_and_save_kmeans(ComputeConfig cfg)
{
    PpmImage *image = ppm_readimage(cfg.input_path);
     if (!image) {
        fprintf(stderr, "Could not read input ppm image %s\n", cfg.input_path);
        return 1;
    }   

    PpmImageGray *gray_image = malloc(sizeof(*gray_image));
    if (gray_image == NULL) {
        fprintf(stderr, "Could allocate gray_image");
        ppm_destroy(image);
        return 1;
    }

    if (!ppm_image_to_ppm_gray_image(image, gray_image)) {
        ppm_destroy(image);
        free(gray_image);
        return 1;
    }

    ppm_destroy(image);

    PpmImageGray* clustered_image = malloc(sizeof(*clustered_image));
    if (clustered_image == NULL) {
        fprintf(stderr, "Could allocate clustered_image");
        return 1;
    }

    clustered_image->rows = gray_image->rows;
    clustered_image->cols = gray_image->cols;
    clustered_image->maxval = gray_image->maxval;
    clustered_image->data = calloc(clustered_image->rows * clustered_image->cols, sizeof(uint8_t)); 
    if (clustered_image->data == NULL) {
        fprintf(stderr, "Could calloc data for clustered_image");
        free(clustered_image);
        return 1;
    }

    kmeans_uint8(gray_image->data, gray_image->cols * gray_image->rows, cfg.k, clustered_image->data);

    if (!ppm_write_gray_image(cfg.output_path, clustered_image)) {
        fprintf(stderr, "Failed to write clustered_image to output_path [%s]", cfg.output_path);
        ppm_destroy_gray_image(clustered_image);
        ppm_destroy_gray_image(gray_image);
        return 1;
    }
    ppm_destroy_gray_image(clustered_image);
    ppm_destroy_gray_image(gray_image);
    return 0;
}

/*Program man print functions.*/
void print_main_help(void) {
    printf("Usage:\n");
    printf("  kmeans compute [options] <input.ppm>\n");
    printf("  kmeans interactive [options] <input.ppm>\n\n");
    printf("Commands:\n");
    printf("  compute       Run k-means and save output image\n");
    printf("  interactive   Run interactive k-means viewer\n");
}

void print_compute_help(void) {
    printf("Usage: kmeans compute -k <clusters> -o <output.ppm> [options] <input.ppm>\n");
    printf("Options:\n");
    printf("  -k <int>   Number of clusters (>=2)\n");
    printf("  -o <path>  Output PPM image path\n");
    printf("  -v         Verbose mode\n");
    printf("  -h         Show this help\n");
}

void print_interactive_help(void) {
    printf("Usage: kmeans interactive [options] <input.ppm>\n");
    printf("Options:\n");
    printf("  -v    Verbose mode\n");
    printf("  -h    Show this help\n");
}

/*Subcommand parsing and redirection to mode execution functions.*/
int run_compute(int argc, char **argv) {
    ComputeConfig cfg = {0};
    cfg.k = 0;
    cfg.output_path = NULL;
    cfg.input_path = NULL;

    int opt;
    optind = 1;

    while ((opt = getopt(argc, argv, "k:o:vh")) != -1) {
        switch (opt) {
            case 'k': {
                char *end;
                long value = strtol(optarg, &end, 10);
                if (*end != '\0' || value < 2) {
                    fprintf(stderr, "Error: invalid k value\n");
                    return 1;
                }
                cfg.k = (int)value;
                break;
            }
            case 'o':
                cfg.output_path = optarg;
                break;
            case 'v':
                cfg.verbose = 1;
                break;
            case 'h':
                print_compute_help();
                return 0;
            default:
                print_compute_help();
                return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: missing input ppm image\n");
        return 1;
    }

    cfg.input_path = argv[optind];

    if (optind + 1 < argc) {
        fprintf(stderr, "Error: too many arguments\n");
        return 1;
    }

    if (cfg.k < 2) {
        fprintf(stderr, "Error: -k must be >= 2\n");
        return 1;
    }

    if (!cfg.output_path) {
        fprintf(stderr, "Error: -o is required\n");
        return 1;
    }

    if (cfg.verbose) {
        printf("[Compute Mode]\n");
        printf("  Input:  %s\n", cfg.input_path);
        printf("  Output: %s\n", cfg.output_path);
        printf("  K:      %d\n", cfg.k);
    }

    return compute_and_save_kmeans(cfg);
}

int run_interactive(int argc, char **argv) {
    InteractiveConfig cfg = {0};
    cfg.input_path = NULL;

    int opt;
    optind = 1;

    while ((opt = getopt(argc, argv, "vh")) != -1) {
        switch (opt) {
            case 'v':
                cfg.verbose = 1;
                break;
            case 'h':
                print_interactive_help();
                return 0;
            default:
                print_interactive_help();
                return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: missing input ppm image\n");
        return 1;
    }

    cfg.input_path = argv[optind];

    if (optind + 1 < argc) {
        fprintf(stderr, "Error: too many arguments\n");
        return 1;
    }

    if (cfg.verbose) {
        printf("[Interactive Mode]\n");
        printf("  Input: %s\n", cfg.input_path);
    }

    run_interactive_mode(cfg);

    return 0;
}


int main(int argc, char **argv)
{
    if (argc < 2) {
        print_main_help();
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0 ||
        strcmp(argv[1], "-h") == 0) {
        print_main_help();
        return 0;
    }

    srand((unsigned int)time(NULL));

    if (strcmp(argv[1], "compute") == 0) {
        return run_compute(argc - 1, argv + 1);
    }

    if (strcmp(argv[1], "interactive") == 0) {
        return run_interactive(argc - 1, argv + 1);
    }

    fprintf(stderr, "Unknown command: %s\n\n", argv[1]);
    print_main_help();
    return 1;
}





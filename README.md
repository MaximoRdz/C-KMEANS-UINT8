# Interactive KMeans

2D grayscale image KMeans implementation in C with an interactive visualization tool built using [Raylib](https://www.raylib.com/).

This project focuses on implementing KMeans from scratch, keeping the algorithm independent from the UI layer, and exploring algorithmic and performance improvements.

---

## Features

- Grayscale (`uint8`) KMeans clustering
- Interactive cluster adjustment
- Real-time texture updates
- PPM image support
- Compute mode and interactive mode
- Structured CLI design

---

## Why PPM?

PPM is used because:

- It is simple and easy to parse manually
- It avoids external image decoding libraries
- It is ideal for learning and experimentation

> It is not intended for production image handling.

---

## Requirements (Windows)

- GCC (MinGW or MSYS2)
- Raylib (provided in `lib/`)
- OpenGL / Win32 system libraries

**Linked libraries:** `raylib` · `opengl32` · `gdi32` · `winmm`

---

## Build

Using the provided Makefile:

```sh
make
```

Produces:

```
kmeans.exe
```

To clean:

```sh
make clean
```

---

## Usage

### Interactive Mode

```sh
kmeans interactive input.ppm
```

**Controls:**

| Key | Action |
|-----|--------|
| `→` RIGHT arrow | Increase cluster count |
| `←` LEFT arrow | Decrease cluster count (minimum 2) |
| `SPACE` | Show original image |

The image updates when the cluster count changes.

### Compute Mode

```sh
kmeans compute -k <clusters> -o output.ppm input.ppm
```

**Options:**

| Flag | Description |
|------|-------------|
| `-k` | Number of clusters (>= 2) |
| `-o` | Output path |
| `-v` | Verbose mode |

---

## Implementation Notes

- Lloyd's algorithm
- Random centroid initialization
- Convergence when centroids stop changing
- Maximum iteration safeguard
- Time complexity per iteration: `O(n × k)`

Where:
- `n` = number of pixels
- `k` = number of clusters

Since we are working with gray uint8 images histogram-based implementation [here](https://github.com/MaximoRdz/C-KMEANS-UINT8/blob/main/histogram-based-kmeans.md)
allowed to push time complexity down to `O(256 x k)`

---

## Future Improvements

- [x] Histogram-based KMeans (reduce complexity to `O(256 × k)`)
- [ ] Lookup table (0–255 → centroid value)
- [ ] Single-pass image reconstruction
- [ ] KMeans++ initialization
- [ ] Multi-threaded computation


# Histogram-Based KMeans Optimization

> If the domain size is smaller than the dataset size, operate on the domain — not on the dataset.

---

## KMeans in a Nutshell

1. Pick `k` random centroids
2. For each pixel, find the closest centroid (using grayscale distance)
3. Recompute each centroid as the mean of its assigned pixels
4. If centroids do not change (or change within a threshold), stop
5. Otherwise, repeat

---

## Current Bottleneck

For each pixel, the algorithm compares against all `k` centroids:

$$O(n \times k)$$

| Symbol | Meaning |
|--------|---------|
| `n` | Number of pixels |
| `k` | Number of clusters |

For a 1 megapixel image with `k = 8`, that's **8,000,000 comparisons per iteration**.

---

## Optimization Insight

Grayscale images (`uint8`) have a fixed domain:

$$|\mathcal{D}| = 256 \quad \text{unique possible values}$$

Even if the image contains $n \gg 256$ pixels, the number of *distinct* inputs never exceeds 256.

All pixels sharing the same gray value are always assigned to the same cluster in a given iteration. There is no need to evaluate each one individually.

## Optimization Strategy

Instead of iterating over every pixel, build a **histogram** first:

```
gray_value (0–255) │ pixel_count
───────────────────┼────────────
        0          │    1024
        1          │     839
      ...          │     ...
      255          │     512
```

Then iterate over the **256 histogram entries** rather than `n` pixels:

- For each gray value `g`:
  - Find the closest centroid
  - Update cluster statistics weighted by `histogram[g]`

### New Complexity

$$O(256 \times k)$$

---

## Code-Level Changes

### Naive `kmeans_uint8`

The original implementation tracks per cluster:

$$S_j = \sum_{p \in C_j} p \qquad N_j = |C_j|$$

Centroid update:

$$\mu_j = \frac{S_j}{N_j}$$

---

### Histogram-Based KMeans

Clusters are now composed of **weighted gray values**. Let $h[g]$ denote the histogram count for gray value $g$.

For cluster $C_j$ containing $m$ unique gray values:

$$\mu_j = \frac{\displaystyle\sum_{g \in C_j} h[g] \cdot g}{\displaystyle\sum_{g \in C_j} h[g]}$$

To keep this efficient, maintain two accumulators per cluster:

| Accumulator | Formula |
|-------------|---------|
| Weighted sum | $W_j = \sum_{g \in C_j} h[g] \cdot g$ |
| Weighted count | $N_j = \sum_{g \in C_j} h[g]$ |

Centroid update remains:

$$\mu_j = \frac{W_j}{N_j}$$

This preserves the same update structure as the naive implementation — only the loop iterates over histogram buckets instead of raw pixels.


#include "ProcTex.h"
#include <cmath>
#include <algorithm>
#include <vector>

namespace hm {

// Hash-based 2D pseudo-random gradient noise
static float hash2D(int x, int y) {
    int n = x * 374761393 + y * 668265263;
    n = (n ^ (n >> 13)) * 1274126177;
    return (n & 0x7fffffff) / float(0x7fffffff);
}

float ProcTex::perlinNoise(float x, float y) {
    int x0 = static_cast<int>(std::floor(x));
    int y0 = static_cast<int>(std::floor(y));
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float sx = x - static_cast<float>(x0);
    float sy = y - static_cast<float>(y0);

    // Quintic Hermite interpolation
    float fx = sx * sx * sx * (sx * (sx * 6.0f - 15.0f) + 10.0f);
    float fy = sy * sy * sy * (sy * (sy * 6.0f - 15.0f) + 10.0f);

    float n00 = hash2D(x0, y0);
    float n10 = hash2D(x1, y0);
    float n01 = hash2D(x0, y1);
    float n11 = hash2D(x1, y1);

    float ix0 = n00 + fx * (n10 - n00);
    float ix1 = n01 + fx * (n11 - n01);
    return ix0 + fy * (ix1 - ix0);
}

float ProcTex::fbm(float x, float y, int octaves) {
    float val = 0.0f;
    float amp = 0.5f;
    float freq = 1.0f;
    for (int i = 0; i < octaves; ++i) {
        val += amp * perlinNoise(x * freq, y * freq);
        freq *= 2.0f;
        amp *= 0.5f;
    }
    return val;
}

float ProcTex::worley(float x, float y) {
    int xi = static_cast<int>(std::floor(x));
    int yi = static_cast<int>(std::floor(y));

    float minDist = 10.0f;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int cx = xi + dx;
            int cy = yi + dy;
            float px = cx + hash2D(cx, cy);
            float py = cy + hash2D(cy + 19, cx + 73);
            float d = std::sqrt((px - x) * (px - x) + (py - y) * (py - y));
            minDist = std::min(minDist, d);
        }
    }
    return minDist;
}

QImage ProcTex::generateStoneBrick(int size, uint32_t seed) {
    QImage img(size, size, QImage::Format_RGBA8888);
    int brickH = size / 16;
    int brickW = size / 8;

    for (int y = 0; y < size; ++y) {
        int row = y / brickH;
        int rowY = y % brickH;
        int xOffset = (row % 2 == 1) ? (brickW / 2) : 0;

        for (int x = 0; x < size; ++x) {
            int colX = (x + xOffset) % size;
            int col = colX / brickW;
            int colXRem = colX % brickW;

            // Mortar detection
            bool isMortar = (rowY <= 2 || rowY >= brickH - 2 || colXRem <= 2 || colXRem >= brickW - 2);

            float n = fbm(x * 0.035f + seed, y * 0.035f + seed, 4);
            float crack = perlinNoise(x * 0.1f, y * 0.1f);

            int r, g, b;
            if (isMortar) {
                int base = 35 + static_cast<int>(n * 25);
                r = base; g = base; b = base + 5;
            } else {
                int brickVariance = static_cast<int>(hash2D(row, col) * 40.0f - 20.0f);
                int baseR = 75 + brickVariance + static_cast<int>(n * 45);
                int baseG = 70 + brickVariance + static_cast<int>(n * 40);
                int baseB = 68 + brickVariance + static_cast<int>(n * 40);

                if (crack > 0.82f) {
                    baseR -= 30; baseG -= 30; baseB -= 30;
                }

                r = std::clamp(baseR, 15, 230);
                g = std::clamp(baseG, 15, 230);
                b = std::clamp(baseB, 15, 230);
            }

            img.setPixelColor(x, y, QColor(r, g, b, 255));
        }
    }
    return img;
}

QImage ProcTex::generateWoodPlanks(int size, uint32_t seed) {
    QImage img(size, size, QImage::Format_RGBA8888);
    int plankH = size / 10;

    for (int y = 0; y < size; ++y) {
        int plankIdx = y / plankH;
        int plankY = y % plankH;
        bool isGap = (plankY <= 2 || plankY >= plankH - 2);

        float plankVariance = hash2D(plankIdx, seed) * 30.0f - 15.0f;

        for (int x = 0; x < size; ++x) {
            if (isGap) {
                img.setPixelColor(x, y, QColor(15, 10, 8, 255));
                continue;
            }

            // Wood grain stretch along X
            float grain = fbm(x * 0.008f + seed, y * 0.08f, 4);
            float fineGrain = perlinNoise(x * 0.02f, y * 0.35f);

            int baseR = static_cast<int>(70 + plankVariance + grain * 50.0f + fineGrain * 15.0f);
            int baseG = static_cast<int>(45 + plankVariance * 0.8f + grain * 35.0f + fineGrain * 10.0f);
            int baseB = static_cast<int>(25 + plankVariance * 0.5f + grain * 20.0f + fineGrain * 5.0f);

            img.setPixelColor(x, y, QColor(
                std::clamp(baseR, 20, 220),
                std::clamp(baseG, 15, 180),
                std::clamp(baseB, 8, 140),
                255
            ));
        }
    }
    return img;
}

QImage ProcTex::generatePeelingPlaster(int size, uint32_t seed) {
    QImage img(size, size, QImage::Format_RGBA8888);

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float n = fbm(x * 0.02f + seed, y * 0.02f + seed, 4);
            float w = worley(x * 0.015f + seed, y * 0.015f + seed);

            bool peeled = (w < 0.22f);
            int r, g, b;

            if (peeled) {
                // Exposed rotten lath / damp dark brick beneath
                r = 45 + static_cast<int>(n * 30);
                g = 35 + static_cast<int>(n * 20);
                b = 28 + static_cast<int>(n * 15);
            } else {
                // Faded damask / aged stained wallpaper
                float stain = fbm(x * 0.008f + 50, y * 0.008f + 50, 3);
                int base = static_cast<int>(95 + n * 35 - stain * 40);
                r = std::clamp(base + 8, 20, 240);
                g = std::clamp(base + 5, 20, 240);
                b = std::clamp(base - 10, 15, 240);
            }

            img.setPixelColor(x, y, QColor(r, g, b, 255));
        }
    }
    return img;
}

QImage ProcTex::generateCheckerTile(int size, uint32_t seed) {
    QImage img(size, size, QImage::Format_RGBA8888);
    int tileSize = size / 8;

    for (int y = 0; y < size; ++y) {
        int ty = y / tileSize;
        int remY = y % tileSize;

        for (int x = 0; x < size; ++x) {
            int tx = x / tileSize;
            int remX = x % tileSize;

            bool isBorder = (remX <= 1 || remX >= tileSize - 1 || remY <= 1 || remY >= tileSize - 1);
            bool isWhite = ((tx + ty) % 2 == 0);

            float dirt = fbm(x * 0.025f + seed, y * 0.025f + seed, 3);

            int r, g, b;
            if (isBorder) {
                r = 20; g = 18; b = 18;
            } else if (isWhite) {
                int val = static_cast<int>(140 - dirt * 70);
                r = val; g = val - 5; b = val - 10;
            } else {
                int val = static_cast<int>(35 + dirt * 25);
                r = val; g = val; b = val + 5;
            }

            img.setPixelColor(x, y, QColor(
                std::clamp(r, 10, 250),
                std::clamp(g, 10, 250),
                std::clamp(b, 10, 250),
                255
            ));
        }
    }
    return img;
}

QImage ProcTex::generateRustMetal(int size, uint32_t seed) {
    QImage img(size, size, QImage::Format_RGBA8888);

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float n = fbm(x * 0.03f + seed, y * 0.03f + seed, 4);
            float rustAmount = fbm(x * 0.012f + seed + 200, y * 0.012f + seed + 200, 3);

            int r, g, b;
            if (rustAmount > 0.45f) {
                // Corroded rust
                r = static_cast<int>(95 + n * 65);
                g = static_cast<int>(45 + n * 35);
                b = static_cast<int>(20 + n * 20);
            } else {
                // Dark cold iron
                int val = static_cast<int>(50 + n * 30);
                r = val; g = val + 2; b = val + 6;
            }

            img.setPixelColor(x, y, QColor(
                std::clamp(r, 15, 230),
                std::clamp(g, 15, 230),
                std::clamp(b, 15, 230),
                255
            ));
        }
    }
    return img;
}

QImage ProcTex::generateBloodSplatter(int size, uint32_t seed) {
    QImage img(size, size, QImage::Format_RGBA8888);
    img.fill(QColor(0, 0, 0, 0)); // transparent background

    float cx = size * 0.5f;
    float cy = size * 0.5f;
    float baseRadius = size * 0.28f;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float dx = x - cx;
            float dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float angle = std::atan2(dy, dx);

            // Tendrils & droplet noise
            float splatterWarp = fbm(std::cos(angle * 4.0f) * 2.0f + seed, std::sin(angle * 4.0f) * 2.0f + seed, 3);
            float threshold = baseRadius * (0.6f + splatterWarp * 0.8f);

            // Satellite drops
            float dropNoise = perlinNoise(x * 0.08f + seed, y * 0.08f + seed);
            bool isDrop = (dropNoise > 0.82f && dist < size * 0.45f);

            if (dist < threshold || isDrop) {
                float centerFade = std::clamp(1.0f - (dist / (size * 0.5f)), 0.0f, 1.0f);
                int r = static_cast<int>(120 + centerFade * 50);
                int g = static_cast<int>(8 + centerFade * 10);
                int b = static_cast<int>(8 + centerFade * 8);
                int a = static_cast<int>(200 + centerFade * 55);

                img.setPixelColor(x, y, QColor(r, g, b, a));
            }
        }
    }
    return img;
}

QImage ProcTex::generateNormalMap(const QImage& heightMap, float strength) {
    int w = heightMap.width();
    int h = heightMap.height();
    QImage normalMap(w, h, QImage::Format_RGBA8888);

    auto getHeight = [&](int x, int y) -> float {
        int px = (x + w) % w;
        int py = (y + h) % h;
        QColor c = heightMap.pixelColor(px, py);
        return (c.red() * 0.299f + c.green() * 0.587f + c.blue() * 0.114f) / 255.0f;
    };

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            // Sobel operator
            float tl = getHeight(x - 1, y - 1);
            float l  = getHeight(x - 1, y);
            float bl = getHeight(x - 1, y + 1);
            float t  = getHeight(x,     y - 1);
            float b  = getHeight(x,     y + 1);
            float tr = getHeight(x + 1, y - 1);
            float r  = getHeight(x + 1, y);
            float br = getHeight(x + 1, y + 1);

            float dx = (tr + 2.0f * r + br) - (tl + 2.0f * l + bl);
            float dy = (bl + 2.0f * b + br) - (tl + 2.0f * t + tr);

            Vec3 norm = normalize(Vec3(-dx * strength, -dy * strength, 1.0f));

            int red   = static_cast<int>((norm.x * 0.5f + 0.5f) * 255.0f);
            int green = static_cast<int>((norm.y * 0.5f + 0.5f) * 255.0f);
            int blue  = static_cast<int>((norm.z * 0.5f + 0.5f) * 255.0f);

            normalMap.setPixelColor(x, y, QColor(
                std::clamp(red, 0, 255),
                std::clamp(green, 0, 255),
                std::clamp(blue, 0, 255),
                255
            ));
        }
    }
    return normalMap;
}

} // namespace hm

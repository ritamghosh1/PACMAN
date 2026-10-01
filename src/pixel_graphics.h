#pragma once

#include <QColor>
#include <QImage>
#include <QString>
#include "entity.h"

namespace pixel {

// 3x5 Classic Arcade Pixel Font
// Glyphs are 3 cols wide, 5 rows tall.
inline const char* getGlyph(char c) {
    switch (c) {
        case '0': return "####.##.##.####";
        case '1': return ".#.##..#..#.###";
        case '2': return "###..#####..###";
        case '3': return "###..####..####";
        case '4': return "#.##.####..#..#";
        case '5': return "####..###..####";
        case '6': return "####..####.####";
        case '7': return "###..#..#..#..#";
        case '8': return "####.#####.####";
        case '9': return "####.####..####";
        case 'A': return "####.#####.##.#";
        case 'B': return "##.#.###.#.###.";
        case 'C': return "####..#..#..###";
        case 'D': return "##.#.##.##.###.";
        case 'E': return "####..####..###";
        case 'F': return "####..####..#..";
        case 'G': return "####..#.##.####";
        case 'H': return "#.##.#####.##.#";
        case 'I': return "###.#..#..#.###";
        case 'J': return "..#..#..##.####";
        case 'K': return "#.##.###.#.##.#";
        case 'L': return "#..#..#..#..###";
        case 'M': return "#.#####.##.##.#";
        case 'N': return "####.##.##.##.#";
        case 'O': return "####.##.##.####";
        case 'P': return "####.#####..#..";
        case 'Q': return "####.##.####..#";
        case 'R': return "####.######.#.#";
        case 'S': return "####..###..####";
        case 'T': return "###.#..#..#..#.";
        case 'U': return "#.##.##.##.####";
        case 'V': return "#.##.##.##.#.#.";
        case 'W': return "#.##.##.#####.#";
        case 'X': return "#.##.#.#.#.##.#";
        case 'Y': return "#.##.####.#..#.";
        case 'Z': return "###..#.#.#..###";
        case '!': return ".#..#..#.....#.";
        case '-': return "......###......";
        case '/': return "..#..#.#.#..#..";
        case ':': return "....#.....#....";
        default:  return "...............";
    }
}

inline void setPixelSafe(QImage& img, int x, int y, QRgb color) {
    if (x >= 0 && x < img.width() && y >= 0 && y < img.height()) {
        img.setPixel(x, y, color);
    }
}

inline void drawText(QImage& img, int startX, int startY, const QString& text, const QColor& color) {
    QRgb rgb = color.rgba();
    int curX = startX;
    QByteArray ba = text.toUpper().toLatin1();
    for (int i = 0; i < ba.size(); i++) {
        char ch = ba.at(i);
        if (ch == ' ') {
            curX += 4; // 3px glyph + 1px spacing
            continue;
        }
        const char* glyph = getGlyph(ch);
        for (int r = 0; r < 5; r++) {
            for (int c = 0; c < 3; c++) {
                if (glyph[r * 3 + c] == '#') {
                    setPixelSafe(img, curX + c, startY + r, rgb);
                }
            }
        }
        curX += 4;
    }
}

// 7x7 Pac-Man Sprite Matrices
// 0 = closed, 1 = half open, 2 = wide open
inline void drawPacman(QImage& img, int cx, int cy, Dir dir, int frame, const QColor& color, int pixelSize = 1) {
    QRgb rgb = color.rgba();
    int x0 = cx - 3 * pixelSize;
    int y0 = cy - 3 * pixelSize;

    // Base 7x7 matrices for Facing Right
    static const char* kClosed[7] = {
        "..###..",
        ".#####.",
        "#######",
        "#######",
        "#######",
        ".#####.",
        "..###.."
    };

    static const char* kHalfRight[7] = {
        "..###..",
        ".#####.",
        "###....",
        "##.....",
        "###....",
        ".#####.",
        "..###.."
    };

    static const char* kWideRight[7] = {
        "..###..",
        ".###...",
        "##.....",
        "#......",
        "##.....",
        ".###...",
        "..###.."
    };

    const char* const* baseSprite = kClosed;
    if (frame == 1) baseSprite = kHalfRight;
    else if (frame == 2) baseSprite = kWideRight;

    for (int r = 0; r < 7; r++) {
        for (int c = 0; c < 7; c++) {
            int srcR = r, srcC = c;
            // Rotate coordinates based on facing direction
            if (frame > 0) {
                switch (dir) {
                    case Dir::Left:  srcR = r; srcC = 6 - c; break;
                    case Dir::Up:    srcR = c; srcC = 6 - r; break;
                    case Dir::Down:  srcR = 6 - c; srcC = r; break;
                    default: break; // Right is base
                }
            }
            if (baseSprite[srcR][srcC] == '#') {
                if (pixelSize == 1) {
                    setPixelSafe(img, x0 + c, y0 + r, rgb);
                } else {
                    for (int pr = 0; pr < pixelSize; pr++) {
                        for (int pc = 0; pc < pixelSize; pc++) {
                            setPixelSafe(img, x0 + c * pixelSize + pc, y0 + r * pixelSize + pr, rgb);
                        }
                    }
                }
            }
        }
    }
}

// Dying animation (11 steps)
inline void drawPacmanDying(QImage& img, int cx, int cy, float progress) {
    int x0 = cx - 3;
    int y0 = cy - 3;
    QRgb yellow = qRgb(255, 255, 0);

    int step = std::clamp(static_cast<int>(progress * 11.0f), 0, 10);

    if (step < 8) {
        // Mouth widens all the way to 360 degrees
        static const char* kClosed[7] = {
            "..###..",
            ".#####.",
            "#######",
            "#######",
            "#######",
            ".#####.",
            "..###.."
        };
        for (int r = 0; r < 7; r++) {
            for (int c = 0; c < 7; c++) {
                if (kClosed[r][c] != '#') continue;
                // Cut wedge towards top
                int dx = c - 3;
                int dy = 3 - r; // positive upwards
                float angle = std::atan2(static_cast<float>(dx), static_cast<float>(dy)); // 0 is straight up
                float cutHalf = (step / 7.0f) * 3.14159f;
                if (std::abs(angle) >= cutHalf) {
                    setPixelSafe(img, x0 + c, y0 + r, yellow);
                }
            }
        }
    } else if (step == 8 || step == 9) {
        // Disappearing line / dot
        setPixelSafe(img, cx, cy, yellow);
        setPixelSafe(img, cx - 1, cy, yellow);
        setPixelSafe(img, cx + 1, cy, yellow);
    } else {
        // Final pop sparks
        QRgb sparkCol = qRgb(255, 183, 174);
        setPixelSafe(img, cx - 3, cy - 3, sparkCol);
        setPixelSafe(img, cx + 3, cy - 3, sparkCol);
        setPixelSafe(img, cx - 3, cy + 3, sparkCol);
        setPixelSafe(img, cx + 3, cy + 3, sparkCol);
    }
}

// 7x7 Ghost Sprite
inline void drawGhost(QImage& img, int cx, int cy, Dir facing, int animFrame,
                      const QColor& bodyColor, bool frightened, bool frightFlash, bool eyesOnly) {
    int x0 = cx - 3;
    int y0 = cy - 3;

    QRgb bodyRgb = bodyColor.rgba();
    if (frightened) {
        bodyRgb = frightFlash ? qRgb(255, 255, 255) : qRgb(33, 33, 222);
    }

    // 1. Draw Body & Wavy Skirt
    if (!eyesOnly) {
        static const char* kDome[6] = {
            "..###..",
            ".#####.",
            "#######",
            "#######",
            "#######",
            "#######"
        };
        for (int r = 0; r < 6; r++) {
            for (int c = 0; c < 7; c++) {
                if (kDome[r][c] == '#') {
                    setPixelSafe(img, x0 + c, y0 + r, bodyRgb);
                }
            }
        }
        // Skirt row 6 alternates with animFrame
        static const char* kSkirt1 = "#.#.#.#";
        static const char* kSkirt2 = ".##.##.";
        const char* skirt = (animFrame % 2 == 0) ? kSkirt1 : kSkirt2;
        for (int c = 0; c < 7; c++) {
            if (skirt[c] == '#') {
                setPixelSafe(img, x0 + c, y0 + 6, bodyRgb);
            }
        }
    }

    // 2. Eyes & Pupils
    if (frightened && !eyesOnly) {
        // Scared white dot eyes & zigzag mouth
        QRgb eyeCol = frightFlash ? qRgb(33, 33, 222) : qRgb(255, 255, 255);
        setPixelSafe(img, x0 + 2, y0 + 2, eyeCol);
        setPixelSafe(img, x0 + 4, y0 + 2, eyeCol);

        // Wavy mouth
        setPixelSafe(img, x0 + 1, y0 + 4, eyeCol);
        setPixelSafe(img, x0 + 3, y0 + 4, eyeCol);
        setPixelSafe(img, x0 + 5, y0 + 4, eyeCol);
        setPixelSafe(img, x0 + 2, y0 + 5, eyeCol);
        setPixelSafe(img, x0 + 4, y0 + 5, eyeCol);
    } else {
        // Normal eyes or Eyes-Only
        QRgb white = qRgb(255, 255, 255);
        QRgb pupil = qRgb(33, 33, 222);

        // Sclera: two 2x3 white blocks
        // Left eye: cols 1, 2, rows 1, 2, 3
        // Right eye: cols 4, 5, rows 1, 2, 3
        for (int r = 1; r <= 3; r++) {
            setPixelSafe(img, x0 + 1, y0 + r, white);
            setPixelSafe(img, x0 + 2, y0 + r, white);
            setPixelSafe(img, x0 + 4, y0 + r, white);
            setPixelSafe(img, x0 + 5, y0 + r, white);
        }

        // Pupils shifted by facing
        int pox = 0, poy = 0;
        switch (facing) {
            case Dir::Left:  pox = 0; poy = 2; break; // left col
            case Dir::Right: pox = 1; poy = 2; break; // right col
            case Dir::Up:    pox = 0; poy = 1; break; // top row
            case Dir::Down:  pox = 0; poy = 3; break; // bottom row
            default:         pox = 0; poy = 2; break;
        }

        if (facing == Dir::Up || facing == Dir::Down) {
            setPixelSafe(img, x0 + 1, y0 + poy, pupil);
            setPixelSafe(img, x0 + 2, y0 + poy, pupil);
            setPixelSafe(img, x0 + 4, y0 + poy, pupil);
            setPixelSafe(img, x0 + 5, y0 + poy, pupil);
        } else {
            setPixelSafe(img, x0 + 1 + pox, y0 + 2, pupil);
            setPixelSafe(img, x0 + 1 + pox, y0 + 3, pupil);
            setPixelSafe(img, x0 + 4 + pox, y0 + 2, pupil);
            setPixelSafe(img, x0 + 4 + pox, y0 + 3, pupil);
        }
    }
}

// 7x7 Bonus Fruit (Cherry)
inline void drawCherry(QImage& img, int cx, int cy) {
    int x0 = cx - 3;
    int y0 = cy - 3;
    QRgb green = qRgb(74, 222, 53);
    QRgb red = qRgb(255, 0, 0);
    QRgb white = qRgb(255, 255, 255);

    // Stems
    setPixelSafe(img, x0 + 3, y0 + 0, green);
    setPixelSafe(img, x0 + 4, y0 + 0, green);
    setPixelSafe(img, x0 + 2, y0 + 1, green);
    setPixelSafe(img, x0 + 4, y0 + 1, green);
    setPixelSafe(img, x0 + 1, y0 + 2, green);
    setPixelSafe(img, x0 + 4, y0 + 2, green);

    // Left cherry
    setPixelSafe(img, x0 + 0, y0 + 3, red);
    setPixelSafe(img, x0 + 1, y0 + 3, red);
    setPixelSafe(img, x0 + 2, y0 + 3, red);
    setPixelSafe(img, x0 + 0, y0 + 4, red);
    setPixelSafe(img, x0 + 1, y0 + 4, white); // shine
    setPixelSafe(img, x0 + 2, y0 + 4, red);
    setPixelSafe(img, x0 + 1, y0 + 5, red);

    // Right cherry
    setPixelSafe(img, x0 + 4, y0 + 3, red);
    setPixelSafe(img, x0 + 5, y0 + 3, red);
    setPixelSafe(img, x0 + 6, y0 + 3, red);
    setPixelSafe(img, x0 + 4, y0 + 4, red);
    setPixelSafe(img, x0 + 5, y0 + 4, white); // shine
    setPixelSafe(img, x0 + 6, y0 + 4, red);
    setPixelSafe(img, x0 + 5, y0 + 5, red);
}

} // namespace pixel

#pragma once

#include <cstdint>

namespace hm {

// Deterministic fast PRNG using xoshiro128**
class Rng {
public:
    explicit Rng(uint64_t seed = 0x853c49e6748fea9bULL) {
        setSeed(seed);
    }

    void setSeed(uint64_t seed) {
        uint64_t z = seed + 0x9e3779b97f4a7c15ULL;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        s[0] = static_cast<uint32_t>(z);
        s[1] = static_cast<uint32_t>(z >> 32);

        z += 0x9e3779b97f4a7c15ULL;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        s[2] = static_cast<uint32_t>(z);
        s[3] = static_cast<uint32_t>(z >> 32);

        if ((s[0] | s[1] | s[2] | s[3]) == 0) {
            s[0] = 1;
        }
    }

    uint32_t nextU32() {
        const uint32_t result = rotl(s[1] * 5, 7) * 9;
        const uint32_t t = s[1] << 9;

        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];

        s[2] ^= t;
        s[3] = rotl(s[3], 11);

        return result;
    }

    // Returns float in [0, 1)
    float nextFloat() {
        return (nextU32() >> 8) * (1.0f / 16777216.0f);
    }

    // Returns float in [minVal, maxVal]
    float rangeFloat(float minVal, float maxVal) {
        return minVal + (maxVal - minVal) * nextFloat();
    }

    // Returns integer in [minVal, maxVal]
    int rangeInt(int minVal, int maxVal) {
        if (minVal >= maxVal) return minVal;
        return minVal + static_cast<int>(nextU32() % (maxVal - minVal + 1));
    }

private:
    static inline uint32_t rotl(const uint32_t x, int k) {
        return (x << k) | (x >> (32 - k));
    }

    uint32_t s[4];
};

} // namespace hm

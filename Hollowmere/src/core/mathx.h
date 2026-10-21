#pragma once

#include <cmath>
#include <algorithm>
#include <cstring>

namespace hm {

constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 6.28318530717958647692f;
constexpr float DEG2RAD = PI / 180.0f;
constexpr float RAD2DEG = 180.0f / PI;

inline float radians(float deg) { return deg * DEG2RAD; }
inline float degrees(float rad) { return rad * RAD2DEG; }
inline float clamp(float v, float lo, float hi) { return std::max(lo, std::min(hi, v)); }
inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
inline float smoothstep(float edge0, float edge1, float x) {
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
    Vec2 operator/(float s) const { return Vec2(x / s, y / s); }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }
};

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3() = default;
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }

    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    Vec3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }
};

inline float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

inline float lengthSq(const Vec3& v) {
    return dot(v, v);
}

inline float length(const Vec3& v) {
    return std::sqrt(lengthSq(v));
}

inline float distance(const Vec3& a, const Vec3& b) {
    return length(a - b);
}

inline Vec3 normalize(const Vec3& v) {
    float l = length(v);
    return l > 1e-6f ? v / l : Vec3(0.0f, 0.0f, 0.0f);
}

inline Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
    return Vec3(lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t));
}

struct Vec4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    Vec4() = default;
    Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    Vec4(const Vec3& v, float w_) : x(v.x), y(v.y), z(v.z), w(w_) {}
};

// 4x4 Column-Major Matrix for OpenGL
struct Mat4 {
    float m[16] = {0.0f};

    Mat4() {
        m[0] = 1.0f; m[5] = 1.0f; m[10] = 1.0f; m[15] = 1.0f;
    }

    const float* data() const { return m; }
    float* data() { return m; }

    static Mat4 identity() {
        return Mat4();
    }

    static Mat4 translate(const Vec3& v) {
        Mat4 res;
        res.m[12] = v.x;
        res.m[13] = v.y;
        res.m[14] = v.z;
        return res;
    }

    static Mat4 scale(const Vec3& v) {
        Mat4 res;
        res.m[0] = v.x;
        res.m[5] = v.y;
        res.m[10] = v.z;
        return res;
    }

    static Mat4 rotateX(float angleRad) {
        Mat4 res;
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        res.m[5] = c;
        res.m[6] = s;
        res.m[9] = -s;
        res.m[10] = c;
        return res;
    }

    static Mat4 rotateY(float angleRad) {
        Mat4 res;
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        res.m[0] = c;
        res.m[2] = -s;
        res.m[8] = s;
        res.m[10] = c;
        return res;
    }

    static Mat4 rotateZ(float angleRad) {
        Mat4 res;
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        res.m[0] = c;
        res.m[1] = s;
        res.m[4] = -s;
        res.m[5] = c;
        return res;
    }

    static Mat4 perspective(float fovRad, float aspect, float zNear, float zFar) {
        Mat4 res;
        std::memset(res.m, 0, sizeof(res.m));
        float tanHalfFov = std::tan(fovRad * 0.5f);
        res.m[0] = 1.0f / (aspect * tanHalfFov);
        res.m[5] = 1.0f / tanHalfFov;
        res.m[10] = -(zFar + zNear) / (zFar - zNear);
        res.m[11] = -1.0f;
        res.m[14] = -(2.0f * zFar * zNear) / (zFar - zNear);
        return res;
    }

    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 f = normalize(target - eye);
        Vec3 s = normalize(cross(f, up));
        Vec3 u = cross(s, f);

        Mat4 res;
        res.m[0] = s.x;
        res.m[4] = s.y;
        res.m[8] = s.z;
        res.m[1] = u.x;
        res.m[5] = u.y;
        res.m[9] = u.z;
        res.m[2] = -f.x;
        res.m[6] = -f.y;
        res.m[10] = -f.z;
        res.m[12] = -dot(s, eye);
        res.m[13] = -dot(u, eye);
        res.m[14] = dot(f, eye);
        return res;
    }

    Mat4 operator*(const Mat4& b) const {
        Mat4 out;
        for (int c = 0; c < 4; ++c) {
            for (int r = 0; r < 4; ++r) {
                out.m[c * 4 + r] =
                    m[0 * 4 + r] * b.m[c * 4 + 0] +
                    m[1 * 4 + r] * b.m[c * 4 + 1] +
                    m[2 * 4 + r] * b.m[c * 4 + 2] +
                    m[3 * 4 + r] * b.m[c * 4 + 3];
            }
        }
        return out;
    }

    Vec3 transformPoint(const Vec3& p) const {
        float x = m[0] * p.x + m[4] * p.y + m[8]  * p.z + m[12];
        float y = m[1] * p.x + m[5] * p.y + m[9]  * p.z + m[13];
        float z = m[2] * p.x + m[6] * p.y + m[10] * p.z + m[14];
        float w = m[3] * p.x + m[7] * p.y + m[11] * p.z + m[15];
        return (std::abs(w) > 1e-6f) ? Vec3(x / w, y / w, z / w) : Vec3(x, y, z);
    }

    Vec3 transformVector(const Vec3& v) const {
        return Vec3(
            m[0] * v.x + m[4] * v.y + m[8]  * v.z,
            m[1] * v.x + m[5] * v.y + m[9]  * v.z,
            m[2] * v.x + m[6] * v.y + m[10] * v.z
        );
    }
};

struct AABB {
    Vec3 min = Vec3(0, 0, 0);
    Vec3 max = Vec3(0, 0, 0);

    AABB() = default;
    AABB(const Vec3& min_, const Vec3& max_) : min(min_), max(max_) {}

    bool intersects(const AABB& o) const {
        return (min.x <= o.max.x && max.x >= o.min.x) &&
               (min.y <= o.max.y && max.y >= o.min.y) &&
               (min.z <= o.max.z && max.z >= o.min.z);
    }

    bool contains(const Vec3& p) const {
        return p.x >= min.x && p.x <= max.x &&
               p.y >= min.y && p.y <= max.y &&
               p.z >= min.z && p.z <= max.z;
    }

    Vec3 center() const {
        return (min + max) * 0.5f;
    }

    Vec3 size() const {
        return max - min;
    }
};

struct Ray {
    Vec3 origin;
    Vec3 dir;

    Ray() = default;
    Ray(const Vec3& o, const Vec3& d) : origin(o), dir(normalize(d)) {}

    bool intersectAABB(const AABB& box, float& tMinOut) const {
        float tmin = 0.0f;
        float tmax = 100000.0f;

        for (int i = 0; i < 3; ++i) {
            float o = (i == 0) ? origin.x : (i == 1 ? origin.y : origin.z);
            float d = (i == 0) ? dir.x : (i == 1 ? dir.y : dir.z);
            float bmin = (i == 0) ? box.min.x : (i == 1 ? box.min.y : box.min.z);
            float bmax = (i == 0) ? box.max.x : (i == 1 ? box.max.y : box.max.z);

            if (std::abs(d) < 1e-6f) {
                if (o < bmin || o > bmax) return false;
            } else {
                float invD = 1.0f / d;
                float t1 = (bmin - o) * invD;
                float t2 = (bmax - o) * invD;
                if (t1 > t2) std::swap(t1, t2);
                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);
                if (tmin > tmax) return false;
            }
        }

        tMinOut = tmin;
        return true;
    }
};

} // namespace hm

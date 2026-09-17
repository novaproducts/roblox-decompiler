#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <cmath>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <mutex>
#include <queue>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "windows.h"

using std::min;
using std::max;

namespace rbx {
    constexpr float PI = 3.14159265358979323846f;
    constexpr float TWO_PI = 2.0f * PI;
    constexpr float HALF_PI = PI * 0.5f;
    constexpr float DEG_TO_RAD = PI / 180.0f;
    constexpr float RAD_TO_DEG = 180.0f / PI;
    constexpr float EPSILON = 1e-6f;

    inline float degToRad(float degrees) noexcept { return degrees * DEG_TO_RAD; }
    inline float radToDeg(float radians) noexcept { return radians * RAD_TO_DEG; }
    inline bool isZero(float value, float epsilon = EPSILON) noexcept { return std::abs(value) < epsilon; }
    inline float lerp(float a, float b, float t) noexcept { return a + t * (b - a); }
    inline float clamp(float value, float min_, float max_) noexcept { return min(max(value, min_), max_); }

    struct vector2;
    struct vector3;
    struct vector4;
    struct matrix3;
    struct matrix4;
    struct cframe;


    struct vector2 final {
        float x, y;

        vector2() noexcept : x(0), y(0) {}
        vector2(float x, float y) noexcept : x(x), y(y) {}
        explicit vector2(float value) noexcept : x(value), y(value) {}

        inline const float& operator[](int i) const noexcept {
            return reinterpret_cast<const float*>(this)[i];
        }
        inline float& operator[](int i) noexcept {
            return reinterpret_cast<float*>(this)[i];
        }

        vector2 operator+(const vector2& other) const noexcept {
            return { x + other.x, y + other.y };
        }
        vector2 operator-(const vector2& other) const noexcept {
            return { x - other.x, y - other.y };
        }
        vector2 operator*(float scalar) const noexcept {
            return { x * scalar, y * scalar };
        }
        vector2 operator*(const vector2& other) const noexcept {
            return { x * other.x, y * other.y };
        }
        vector2 operator/(float scalar) const noexcept {
            return { x / scalar, y / scalar };
        }
        vector2 operator/(const vector2& other) const noexcept {
            return { x / other.x, y / other.y };
        }
        vector2 operator-() const noexcept {
            return { -x, -y };
        }

        vector2& operator+=(const vector2& other) noexcept {
            x += other.x; y += other.y; return *this;
        }
        vector2& operator-=(const vector2& other) noexcept {
            x -= other.x; y -= other.y; return *this;
        }
        vector2& operator*=(float scalar) noexcept {
            x *= scalar; y *= scalar; return *this;
        }
        vector2& operator/=(float scalar) noexcept {
            x /= scalar; y /= scalar; return *this;
        }

        bool operator==(const vector2& other) const noexcept {
            return isZero(x - other.x) && isZero(y - other.y);
        }
        bool operator!=(const vector2& other) const noexcept {
            return !(*this == other);
        }

        float magnitude() const noexcept {
            return std::sqrt(x * x + y * y);
        }
        float magnitudeSquared() const noexcept {
            return x * x + y * y;
        }
        float dot(const vector2& other) const noexcept {
            return x * other.x + y * other.y;
        }
        float distance(const vector2& other) const noexcept {
            return (*this - other).magnitude();
        }
        float distanceSquared(const vector2& other) const noexcept {
            return (*this - other).magnitudeSquared();
        }
        vector2 normalize() const noexcept {
            float mag = magnitude();
            return isZero(mag) ? vector2(0, 0) : *this / mag;
        }
        vector2 perpendicular() const noexcept {
            return { -y, x };
        }

        static const vector2& zero() noexcept {
            static const vector2 v(0, 0); return v;
        }
        static const vector2& one() noexcept {
            static const vector2 v(1, 1); return v;
        }
        static const vector2& unitX() noexcept {
            static const vector2 v(1, 0); return v;
        }
        static const vector2& unitY() noexcept {
            static const vector2 v(0, 1); return v;
        }
    };

    struct vector3 final {
        float x, y, z;

        vector3() noexcept : x(0), y(0), z(0) {}
        vector3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}
        explicit vector3(float value) noexcept : x(value), y(value), z(value) {}
        vector3(const vector2& v, float z) noexcept : x(v.x), y(v.y), z(z) {}
        float length() const { return std::sqrt(dot(*this)); }
        inline const float& operator[](int i) const noexcept {
            return reinterpret_cast<const float*>(this)[i];
        }
        inline float& operator[](int i) noexcept {
            return reinterpret_cast<float*>(this)[i];
        }



        vector3 operator+(const vector3& other) const noexcept {
            return { x + other.x, y + other.y, z + other.z };
        }
        vector3 operator-(const vector3& other) const noexcept {
            return { x - other.x, y - other.y, z - other.z };
        }
        vector3 operator*(float scalar) const noexcept {
            return { x * scalar, y * scalar, z * scalar };
        }
        vector3 operator*(const vector3& other) const noexcept {
            return { x * other.x, y * other.y, z * other.z };
        }
        vector3 operator/(float scalar) const noexcept {
            return *this * (1.0f / scalar);
        }
        vector3 operator/(const vector3& other) const noexcept {
            return { x / other.x, y / other.y, z / other.z };
        }
        vector3 operator-() const noexcept {
            return { -x, -y, -z };
        }

        vector3& operator+=(const vector3& other) noexcept {
            x += other.x; y += other.y; z += other.z; return *this;
        }
        vector3& operator-=(const vector3& other) noexcept {
            x -= other.x; y -= other.y; z -= other.z; return *this;
        }
        vector3& operator*=(float scalar) noexcept {
            x *= scalar; y *= scalar; z *= scalar; return *this;
        }
        vector3& operator/=(float scalar) noexcept {
            return *this *= (1.0f / scalar);
        }
        vector3& operator/=(const vector3& other) noexcept {
            x /= other.x; y /= other.y; z /= other.z; return *this;
        }

        bool operator==(const vector3& other) const noexcept {
            return isZero(x - other.x) && isZero(y - other.y) && isZero(z - other.z);
        }
        bool operator!=(const vector3& other) const noexcept {
            return !(*this == other);
        }

        float magnitude() const noexcept {
            return std::sqrt(x * x + y * y + z * z);
        }
        float magnitudeSquared() const noexcept {
            return x * x + y * y + z * z;
        }
        float dot(const vector3& other) const noexcept {
            return x * other.x + y * other.y + z * other.z;
        }
        vector3 cross(const vector3& other) const noexcept {
            return {
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            };
        }
        float distance(const vector3& other) const noexcept {
            return (*this - other).magnitude();
        }
        float distanceSquared(const vector3& other) const noexcept {
            return (*this - other).magnitudeSquared();
        }
        vector3 normalize() const noexcept {
            float mag = magnitude();
            return isZero(mag) ? vector3(0, 0, 0) : *this / mag;
        }
        vector3 direction() const noexcept {
            return normalize();
        }
        vector3 reflect(const vector3& normal) const noexcept {
            return *this - normal * (2.0f * dot(normal));
        }
        vector3 project(const vector3& other) const noexcept {
            float otherMagSq = other.magnitudeSquared();
            return isZero(otherMagSq) ? vector3::zero() : other * (dot(other) / otherMagSq);
        }
        vector3 lerp(const vector3& other, float t) const noexcept {
            return *this + (other - *this) * t;
        }


        float length_squared() const {
            return x * x + y * y + z * z;
        };

        static const vector3& zero() noexcept {
            static const vector3 v(0, 0, 0); return v;
        }
        static const vector3& one() noexcept {
            static const vector3 v(1, 1, 1); return v;
        }
        static const vector3& unitX() noexcept {
            static const vector3 v(1, 0, 0); return v;
        }
        static const vector3& unitY() noexcept {
            static const vector3 v(0, 1, 0); return v;
        }
        static const vector3& unitZ() noexcept {
            static const vector3 v(0, 0, 1); return v;
        }
        static const vector3& forward() noexcept {
            static const vector3 v(0, 0, -1); return v;
        }
        static const vector3& back() noexcept {
            static const vector3 v(0, 0, 1); return v;
        }
        static const vector3& up() noexcept {
            static const vector3 v(0, 1, 0); return v;
        }
        static const vector3& down() noexcept {
            static const vector3 v(0, -1, 0); return v;
        }
        static const vector3& left() noexcept {
            static const vector3 v(-1, 0, 0); return v;
        }
        static const vector3& right() noexcept {
            static const vector3 v(1, 0, 0); return v;
        }
    };

    struct vector4 final {
        float x, y, z, w;

        vector4() noexcept : x(0), y(0), z(0), w(0) {}
        vector4(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w) {}
        explicit vector4(float value) noexcept : x(value), y(value), z(value), w(value) {}
        vector4(const vector3& v, float w) noexcept : x(v.x), y(v.y), z(v.z), w(w) {}

        inline const float& operator[](int i) const noexcept {
            return reinterpret_cast<const float*>(this)[i];
        }
        inline float& operator[](int i) noexcept {
            return reinterpret_cast<float*>(this)[i];
        }

        vector4 operator+(const vector4& other) const noexcept {
            return { x + other.x, y + other.y, z + other.z, w + other.w };
        }
        vector4 operator-(const vector4& other) const noexcept {
            return { x - other.x, y - other.y, z - other.z, w - other.w };
        }
        vector4 operator*(float scalar) const noexcept {
            return { x * scalar, y * scalar, z * scalar, w * scalar };
        }
        vector4 operator/(float scalar) const noexcept {
            return *this * (1.0f / scalar);
        }

        float magnitude() const noexcept {
            return std::sqrt(x * x + y * y + z * z + w * w);
        }
        float dot(const vector4& other) const noexcept {
            return x * other.x + y * other.y + z * other.z + w * other.w;
        }
        vector4 normalize() const noexcept {
            float mag = magnitude();
            return isZero(mag) ? vector4(0, 0, 0, 0) : *this / mag;
        }

        vector3 xyz() const noexcept { return { x, y, z }; }
        vector2 xy() const noexcept { return { x, y }; }

        static const vector4& zero() noexcept {
            static const vector4 v(0, 0, 0, 0); return v;
        }
        static const vector4& one() noexcept {
            static const vector4 v(1, 1, 1, 1); return v;
        }
    };

    struct matrix3 final {
        float data[9];

        matrix3() noexcept {
            std::fill(data, data + 9, 0.0f);
        }

        matrix3(float m00, float m01, float m02,
            float m10, float m11, float m12,
            float m20, float m21, float m22) noexcept {
            data[0] = m00; data[1] = m01; data[2] = m02;
            data[3] = m10; data[4] = m11; data[5] = m12;
            data[6] = m20; data[7] = m21; data[8] = m22;
        }

        inline const float& operator[](int i) const noexcept { return data[i]; }
        inline float& operator[](int i) noexcept { return data[i]; }

        float& operator()(int row, int col) noexcept { return data[row * 3 + col]; }
        const float& operator()(int row, int col) const noexcept { return data[row * 3 + col]; }

        matrix3 operator*(const matrix3& other) const noexcept {
            matrix3 result;
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    result(i, j) = 0;
                    for (int k = 0; k < 3; ++k) {
                        result(i, j) += (*this)(i, k) * other(k, j);
                    }
                }
            }
            return result;
        }

        matrix3 operator+(const matrix3& other) const noexcept {
            matrix3 result;
            for (int i = 0; i < 9; ++i)
                result.data[i] = data[i] + other.data[i];
            return result;
        }

        matrix3 operator-(const matrix3& other) const noexcept {
            matrix3 result;
            for (int i = 0; i < 9; ++i)
                result.data[i] = data[i] - other.data[i];
            return result;
        }

        matrix3 operator*(float scalar) const noexcept {
            matrix3 result;
            for (int i = 0; i < 9; ++i)
                result.data[i] = data[i] * scalar;
            return result;
        }

        vector3 operator*(const vector3& vec) const noexcept {
            return {
                data[0] * vec.x + data[1] * vec.y + data[2] * vec.z,
                data[3] * vec.x + data[4] * vec.y + data[5] * vec.z,
                data[6] * vec.x + data[7] * vec.y + data[8] * vec.z
            };
        }

        matrix3 transpose() const noexcept {
            matrix3 result;
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j)
                    result(i, j) = (*this)(j, i);
            return result;
        }

        float determinant() const noexcept {
            return data[0] * (data[4] * data[8] - data[5] * data[7]) -
                data[1] * (data[3] * data[8] - data[5] * data[6]) +
                data[2] * (data[3] * data[7] - data[4] * data[6]);
        }

        matrix3 inverse() const noexcept {
            float det = determinant();
            if (isZero(det)) return matrix3{};

            matrix3 result;
            result.data[0] = (data[4] * data[8] - data[5] * data[7]) / det;
            result.data[1] = (data[2] * data[7] - data[1] * data[8]) / det;
            result.data[2] = (data[1] * data[5] - data[2] * data[4]) / det;
            result.data[3] = (data[5] * data[6] - data[3] * data[8]) / det;
            result.data[4] = (data[0] * data[8] - data[2] * data[6]) / det;
            result.data[5] = (data[2] * data[3] - data[0] * data[5]) / det;
            result.data[6] = (data[3] * data[7] - data[4] * data[6]) / det;
            result.data[7] = (data[1] * data[6] - data[0] * data[7]) / det;
            result.data[8] = (data[0] * data[4] - data[1] * data[3]) / det;
            return result;
        }

        vector3 getColumn(int index) const noexcept {
            return { data[index], data[index + 3], data[index + 6] };
        }
        vector3 getRow(int index) const noexcept {
            return { data[index * 3], data[index * 3 + 1], data[index * 3 + 2] };
        }

        vector3 MatrixToEulerAngles() const {
            vector3 angles;
            angles.y = std::asin(clamp(data[6], -1.0f, 1.0f));

            if (std::abs(data[6]) < 0.9999f) {
                angles.x = std::atan2(-data[7], data[8]);
                angles.z = std::atan2(-data[3], data[0]);
            }
            else {
                angles.x = 0.0f;
                angles.z = std::atan2(data[1], data[4]);
            }

            return angles * RAD_TO_DEG;
        }

        static matrix3 EulerAnglesToMatrix(const vector3& angles) {
            vector3 rad = angles * DEG_TO_RAD;
            float cx = std::cos(rad.x), sx = std::sin(rad.x);
            float cy = std::cos(rad.y), sy = std::sin(rad.y);
            float cz = std::cos(rad.z), sz = std::sin(rad.z);

            return matrix3{
                cy * cz, cy * sz, -sy,
                sx * sy * cz - cx * sz, sx * sy * sz + cx * cz, sx * cy,
                cx * sy * cz + sx * sz, cx * sy * sz - sx * cz, cx * cy
            };
        }

        vector3 GetForwardVector() const noexcept { return { data[2], data[5], data[8] }; }
        vector3 GetRightVector() const noexcept { return { data[0], data[3], data[6] }; }
        vector3 GetUpVector() const noexcept { return { data[1], data[4], data[7] }; }

        static const matrix3& identity() noexcept {
            static const matrix3 m(1, 0, 0, 0, 1, 0, 0, 0, 1); return m;
        }
    };

    struct matrix4 final {
        float data[16];

        matrix4() noexcept {
            std::fill(data, data + 16, 0.0f);
        }

        matrix4(float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33) noexcept {
            data[0] = m00; data[1] = m01; data[2] = m02; data[3] = m03;
            data[4] = m10; data[5] = m11; data[6] = m12; data[7] = m13;
            data[8] = m20; data[9] = m21; data[10] = m22; data[11] = m23;
            data[12] = m30; data[13] = m31; data[14] = m32; data[15] = m33;
        }

        inline const float& operator[](int i) const noexcept { return data[i]; }
        inline float& operator[](int i) noexcept { return data[i]; }

        float& operator()(int row, int col) noexcept { return data[row * 4 + col]; }
        const float& operator()(int row, int col) const noexcept { return data[row * 4 + col]; }

        matrix4 operator*(const matrix4& other) const noexcept {
            matrix4 result;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    result(i, j) = 0;
                    for (int k = 0; k < 4; ++k) {
                        result(i, j) += (*this)(i, k) * other(k, j);
                    }
                }
            }
            return result;
        }

        vector4 operator*(const vector4& vec) const noexcept {
            return {
                data[0] * vec.x + data[1] * vec.y + data[2] * vec.z + data[3] * vec.w,
                data[4] * vec.x + data[5] * vec.y + data[6] * vec.z + data[7] * vec.w,
                data[8] * vec.x + data[9] * vec.y + data[10] * vec.z + data[11] * vec.w,
                data[12] * vec.x + data[13] * vec.y + data[14] * vec.z + data[15] * vec.w
            };
        }

        static const matrix4& identity() noexcept {
            static const matrix4 m(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
            return m;
        }
    };

    struct cframe final {
        vector3 right_vector = { 1, 0, 0 };
        vector3 up_vector = { 0, 1, 0 };
        vector3 back_vector = { 0, 0, 1 };
        vector3 position = { 0, 0, 0 };

        cframe() = default;
        cframe(vector3 position) : position{ position } {}
        cframe(vector3 right, vector3 up, vector3 back, vector3 pos)
            : right_vector{ right }, up_vector{ up }, back_vector{ back }, position{ pos } {
        }

        void lookAtLocked(vector3 point) noexcept {
            vector3 look_vector = (position - point).normalize();
            right_vector = vector3::unitY().cross(look_vector).normalize();
            up_vector = look_vector.cross(right_vector).normalize();
            back_vector = -look_vector;
        }

        cframe lookAt(vector3 point) const noexcept {
            vector3 look_vector = (position - point).normalize();
            vector3 right = vector3::unitY().cross(look_vector).normalize();
            vector3 up = look_vector.cross(right).normalize();
            return cframe{ right, up, look_vector, position };
        }

        cframe operator*(const cframe& other) const noexcept {
            cframe result;
            result.right_vector = right_vector * other.right_vector;
            result.up_vector = up_vector * other.up_vector;
            result.back_vector = back_vector * other.back_vector;
            result.position = transformPoint(other.position);
            return result;
        }

        vector3 operator*(const vector3& vec) const noexcept {
            return transformPoint(vec);
        }

        vector3 transformPoint(const vector3& point) const noexcept {
            return {
                right_vector.x * point.x + up_vector.x * point.y + back_vector.x * point.z + position.x,
                right_vector.y * point.x + up_vector.y * point.y + back_vector.y * point.z + position.y,
                right_vector.z * point.x + up_vector.z * point.y + back_vector.z * point.z + position.z
            };
        }

        vector3 transformVector(const vector3& vec) const noexcept {
            return {
                right_vector.x * vec.x + up_vector.x * vec.y + back_vector.x * vec.z,
                right_vector.y * vec.x + up_vector.y * vec.y + back_vector.y * vec.z,
                right_vector.z * vec.x + up_vector.z * vec.y + back_vector.z * vec.z
            };
        }

        cframe inverse() const noexcept {
            vector3 inv_right = { right_vector.x, up_vector.x, back_vector.x };
            vector3 inv_up = { right_vector.y, up_vector.y, back_vector.y };
            vector3 inv_back = { right_vector.z, up_vector.z, back_vector.z };
            vector3 inv_pos = {
                -position.dot(inv_right),
                -position.dot(inv_up),
                -position.dot(inv_back)
            };
            return cframe{ inv_right, inv_up, inv_back, inv_pos };
        }

        matrix4 toMatrix4x4() const noexcept {
            return matrix4{
                right_vector.x, up_vector.x, back_vector.x, position.x,
                right_vector.y, up_vector.y, back_vector.y, position.y,
                right_vector.z, up_vector.z, back_vector.z, position.z,
                0, 0, 0, 1
            };
        }

        static const cframe& identity() noexcept {
            static const cframe cf; return cf;
        }
    };

    inline vector2 operator*(float scalar, const vector2& vec) noexcept {
        return vec * scalar;
    }
    inline vector3 operator*(float scalar, const vector3& vec) noexcept {
        return vec * scalar;
    }
    inline vector4 operator*(float scalar, const vector4& vec) noexcept {
        return vec * scalar;
    }

    static vector3 lookvec(const matrix3& rotationMatrix) noexcept {
        return rotationMatrix.getColumn(2);
    }
    static vector3 rightvec(const matrix3& rotationMatrix) noexcept {
        return rotationMatrix.getColumn(0);
    }
    static vector3 upvec(const matrix3& rotationMatrix) noexcept {
        return rotationMatrix.getColumn(1);
    }
}

using namespace rbx;
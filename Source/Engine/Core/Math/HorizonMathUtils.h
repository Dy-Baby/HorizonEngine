#pragma once

#include "Core/Math/HorizonMathCommon.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Matrix.h"
#include "Core/Math/Vector.h"

namespace HE
{

namespace Math
{

FORCEINLINE double Cos(double radians)
{
    return cos(radians);
}

FORCEINLINE float Cos(float radians)
{
    return cos(radians);
}

template <typename T>
FORCEINLINE T Max(const T& a, const T& b)
{
    return (a >= b) ? a : b;
}

template <typename T>
FORCEINLINE T Min(const T& a, const T& b)
{
    return (a <= b) ? a : b;
}

FORCEINLINE float Lerp(float x, float y, float t)
{
    return x + (y - x) * t;
}

FORCEINLINE Vector3 Lerp(const Vector3& x, const Vector3& y, float t)
{
    return x + (y - x) * t;
}

FORCEINLINE float Abs(float x)
{
    return abs(x);
}

FORCEINLINE float Fmod(float x, float y)
{
    return fmod(x, y);
}

FORCEINLINE float Square(float x)
{
    return x * x;
}

FORCEINLINE Vector3 Normalize(const Vector3& v)
{
    return glm::normalize(v);
}

FORCEINLINE float Length(const Vector3& v)
{
    return glm::length(v);
}

FORCEINLINE float LengthSquared(const Vector3& v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

FORCEINLINE bool IsPowerOfTwo(uint32_t value)
{
    return (value > 0) && ((value & (value - 1)) == 0);
}

FORCEINLINE Matrix4x4 Transpose(const Matrix4x4& matrix)
{
    return glm::transpose(matrix);
}

FORCEINLINE Matrix4x4 Compose(const Vector3& translation, const Quaternion& rotation, const Vector3& scale)
{
    return glm::translate(glm::mat4(1), translation) * glm::mat4_cast(glm::normalize(rotation)) * glm::scale(glm::mat4(1.0f), scale);
}

FORCEINLINE void Decompose(const Matrix4x4& matrix, Vector3& outTranslation, Quaternion& outRotation, Vector3& outScale)
{
    Vector3 skew;
    Vector4 perspective;
    glm::decompose(matrix, outScale, outRotation, outTranslation, skew, perspective);
}

FORCEINLINE uint32_t MaxMipLevelCount(uint32_t size)
{
    return 1 + uint32_t(std::floor(std::log2(size)));
}

FORCEINLINE uint32_t MaxMipLevelCount(uint32_t width, uint32_t height)
{
    return 1 + uint32_t(std::floor(std::log2(glm::min(width, height))));
}

FORCEINLINE float Clamp(float x, float min = 0, float max = 1)
{
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

FORCEINLINE uint32 Clamp(uint32 x, uint32 min = 0, uint32 max = 1)
{
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

FORCEINLINE Vector3 Clamp(const Vector3& vec, float min = 0, float max = 1)
{
    return Vector3(Clamp(vec[0]), Clamp(vec[1]), Clamp(vec[2]));
}

FORCEINLINE Vector3 Bezier3(float t, Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3)
{
    t = Math::Clamp(t, 0.0f, 1.0f);
    float d = 1.0f - t;
    return d * d * d * p0 + 3.0f * d * d * t * p1 + 3.0f * d * t * t * p2 + t * t * t * p3;
}

FORCEINLINE float DegreesToRadians(float x)
{
    return glm::radians(x);
}

FORCEINLINE Vector3 DegreesToRadians(const Vector3& v)
{
    return glm::radians(v);
}

FORCEINLINE float RadiansToDegrees(float x)
{
    return glm::radians(x);
}

FORCEINLINE Quaternion AngleAxis(float angle, const Vector3& axis)
{
    return glm::angleAxis(angle, axis);
}

FORCEINLINE Matrix4x4 Inverse(const Matrix4x4& matrix)
{
    return glm::inverse(matrix);
}

}
}
#pragma once
#include "raymath.h"

Vector3 operator+(Vector3 a, Vector3 b);
Vector3& operator+=(Vector3& a, Vector3 b);
Vector3 operator-(Vector3 a, Vector3 b);
Vector3& operator-=(Vector3& a, Vector3 b);
Vector3 operator*(float a, Vector3 v);
Vector3& operator*=(Vector3& v, float a);
Vector3 operator/(Vector3 v, float a);

Vector2 operator+(Vector2 a, Vector2 b);
Vector2& operator+=(Vector2& a, Vector2 b);
Vector2 operator-(Vector2 a, Vector2 b);
Vector2& operator-=(Vector2& a, Vector2 b);
Vector2 operator*(float a, Vector2 v);
Vector2& operator*=(Vector2& v, float a);
Vector2 operator/(Vector2 v, float a);

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

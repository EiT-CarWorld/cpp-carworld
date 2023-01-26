#pragma once
#include "raymath.h"

Vector3 operator+(Vector3 a, Vector3 b);
Vector3& operator+=(Vector3& a, Vector3 b);
Vector3 operator-(Vector3 a, Vector3 b);
Vector3& operator-=(Vector3& a, Vector3 b);
Vector3 operator*(float a, Vector3 v);
Vector3& operator*=(Vector3& v, float a);
Vector3 operator/(Vector3 v, float a);

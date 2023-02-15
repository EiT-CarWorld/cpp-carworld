#define RAYMATH_IMPLEMENTATION
#include "raymath.h"
#include "carMath.h"

// Vector3
Vector3 operator+(Vector3 a, Vector3 b) {
    return {a.x+b.x, a.y+b.y, a.z+b.z};
}
Vector3& operator+=(Vector3& a, Vector3 b) {
    a = a+b;
    return a;
}

Vector3 operator-(Vector3 a, Vector3 b) {
    return {a.x-b.x, a.y-b.y, a.z-b.z};
}
Vector3& operator-=(Vector3& a, Vector3 b) {
    a = a-b;
    return a;
}

Vector3 operator*(float a, Vector3 v) {
    return {a*v.x, a*v.y, a*v.z};
}
Vector3& operator*=(Vector3& v, float a) {
    v = a*v;
    return v;
}

Vector3 operator/(Vector3 v, float a) {
    return {v.x/a, v.y/a, v.z/a};
}

// Vector2
Vector2 operator+(Vector2 a, Vector2 b) {
    return {a.x+b.x, a.y+b.y};
}
Vector2& operator+=(Vector2& a, Vector2 b) {
    a = a+b;
    return a;
}

Vector2 operator-(Vector2 a, Vector2 b) {
    return {a.x-b.x, a.y-b.y};
}
Vector2& operator-=(Vector2& a, Vector2 b) {
    a = a-b;
    return a;
}

Vector2 operator*(float a, Vector2 v) {
    return {a*v.x, a*v.y};
}
Vector2& operator*=(Vector2& v, float a) {
    v = a*v;
    return v;
}

Vector2 operator/(Vector2 v, float a) {
    return {v.x/a, v.y/a};
}
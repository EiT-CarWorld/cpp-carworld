#define RAYMATH_IMPLEMENTATION
#include "raymath.h"
#include "carMath.h"

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
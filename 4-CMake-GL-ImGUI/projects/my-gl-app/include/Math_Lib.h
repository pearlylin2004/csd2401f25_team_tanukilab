#pragma once
#ifndef MATHLIB_H
#define MATHLIB_H

#include <cmath>
#include <algorithm> //for std::min and std::max

typedef struct {
    float x;
    float y;
} Vector2D;

typedef struct {
    float m[3][3];
} Mat33;

Vector2D Vec_Add(const Vector2D* a, const Vector2D* b);
Vector2D Vec_Sub(const Vector2D* a, const Vector2D* b);
Vector2D Vec_Normalize(const Vector2D* v);
Vector2D Vec_Scale(const Vector2D* v, float scale);
Vector2D Vec_Rotate(const Vector2D* v, float rad);
Vector2D Vec_Transform(const Vector2D* v, const Mat33* mat);

inline float Math_Sqrt(float v) { return std::sqrt(v); }
inline float Math_Fabs(float v) { return std::fabs(v); }
inline float Math_Fmax(float a, float b) { return std::fmax(a, b); }
inline float Math_Min(float a, float b) { return std::min(a, b); }
inline float Math_Max(float a, float b) { return std::max(a, b); }

void Mat33_Scale(Mat33* mat, float x, float y);
void Mat33_Rot(Mat33* mat, float rad);
void Mat33_Trans(Mat33* mat, float x, float y);
void Mat33_Con_cat(Mat33* con_cat, const Mat33* a, const Mat33* b);

#endif // MATHLIB_H

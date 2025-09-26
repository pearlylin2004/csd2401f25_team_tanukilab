#pragma once
#ifndef MATHLIB_H
#define MATHLIB_H
#include <cmath>

typedef struct {
    float x;
    float y;
} Vector2D;

typedef struct {
    float m[3][3];
} Mat33;

typedef struct {
    float x;
    float y;
    float z;
} Vector3D;

typedef struct {
    float m[4][4];
}Mat44;

float Math_Sqrt(float v); 
float Math_Fabs(float v);
float Math_Fmax(float a, float b);
float Math_Min(float a, float b);
float Math_Max(float a, float b);


Vector2D Vec_Add(const Vector2D* a, const Vector2D* b);
Vector2D Vec_Sub(const Vector2D* a, const Vector2D* b);
Vector2D Vec_Normalize(const Vector2D* v);
Vector2D Vec_Scale(const Vector2D* v, float scale);
Vector2D Vec_Rotate(const Vector2D* v, float rad);
Vector2D Vec_Transform(const Vector2D* v, const Mat33* mat);

void Mat33_Scale(Mat33* mat, float x, float y);
void Mat33_Rot(Mat33* mat, float rad);
void Mat33_Trans(Mat33* mat, float x, float y);
void Mat33_Con_cat(Mat33* con_cat, const Mat33* a, const Mat33* b);

Vector3D Vec3_Add(const Vector3D* a, const Vector3D* b);
Vector3D Vec3_Sub(const Vector3D* a, const Vector3D* b);
Vector3D Vec3_Normalize(const Vector3D* v);
Vector3D Vec3_Scale(const Vector3D* v, float scale);
Vector3D Vec3_Transform(const Vector3D* v, const Mat44* mat);


void Mat44_Scale(Mat44* mat, float x, float y, float z);
void Mat44_RotX(Mat44* mat, float rad);
void Mat44_RotY(Mat44* mat, float rad);
void Mat44_RotZ(Mat44* mat, float rad);
void Mat44_Trans(Mat44* mat, float x, float y, float z);
void Mat44_Con_cat(Mat44* con_cat, const Mat44* a, const Mat44* b);


#endif
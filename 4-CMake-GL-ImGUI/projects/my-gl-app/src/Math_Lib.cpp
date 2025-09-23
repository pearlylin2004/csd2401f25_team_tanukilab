#include "Math_Lib.h"
#include <cmath>

//----------------------------------------
// Vector Operations
//----------------------------------------

// Vector addition: sum = a + b
// Formula: 
//    sum.x = a.x + b.x
//    sum.y = a.y + b.y
Vector2D Vec_Add(const Vector2D* a, const Vector2D* b) {
    Vector2D sum = { a->x + b->x,
                     a->y + b->y };
    return sum;
}

// Vector subtraction: sub = a - b
// Formula: 
//    sub.x = a.x - b.x
//    sub.y = a.y - b.y
Vector2D Vec_Sub(const Vector2D* a, const Vector2D* b) {
    Vector2D sub = { a->x - b->x,
                     a->y - b->y };
    return sub;
}

// Normalize a vector: Norm = v / |v|
// Formula:
//    |v| = sqrt(v.x^2 + v.y^2)
//    Norm.x = v.x / |v|
//    Norm.y = v.y / |v|
Vector2D Vec_Normalize(const Vector2D* v) {
    Vector2D Norm = { 0.0f, 0.0f };
    float dis = sqrtf(v->x * v->x + v->y * v->y);
    if (dis > 0.00001f) { // avoid division by zero
        Norm.x = v->x / dis;
        Norm.y = v->y / dis;
    }
    return Norm;
}

// Scale a vector: scaled = v * scale
// Formula:
//    scaled.x = v.x * scale
//    scaled.y = v.y * scale
Vector2D Vec_Scale(const Vector2D* v, float scale) {
    Vector2D scaled = { v->x * scale, v->y * scale };
    return scaled;
}

// Rotate a vector by rad radians
// Formula (2D rotation matrix):
//    [x']   [ cosθ  -sinθ ] [x]
//    [y'] = [ sinθ   cosθ ] [y]
//    rotated.x = v.x * cos(rad) - v.y * sin(rad)
//    rotated.y = v.x * sin(rad) + v.y * cos(rad)
Vector2D Vec_Rotate(const Vector2D* v, float rad) {
    float c = cosf(rad);
    float s = sinf(rad);
    Vector2D rotated = { v->x * c - v->y * s,
                         v->x * s + v->y * c };
    return rotated;
}

// Transform a vector by a 3x3 matrix (affine transformation)
// Formula:
//    [x']   [ m00 m01 m02 ] [x]
//    [y'] = [ m10 m11 m12 ] [y]
//    x' = x * m00 + y * m01 + m02
//    y' = x * m10 + y * m11 + m12
Vector2D Vec_Transform(const Vector2D* v, const Mat33* mat) {
    Vector2D transformed;
    transformed.x = v->x * mat->m[0][0] + v->y * mat->m[0][1] + mat->m[0][2];
    transformed.y = v->x * mat->m[1][0] + v->y * mat->m[1][1] + mat->m[1][2];
    return transformed;
}

//----------------------------------------
// Matrix Operations (3x3)
//----------------------------------------

// Create a scaling matrix
// Formula:
//    [ x  0  0 ]
//    [ 0  y  0 ]
//    [ 0  0  1 ]
void Mat33_Scale(Mat33* mat, float x, float y) {
    mat->m[0][0] = x; mat->m[0][1] = 0; mat->m[0][2] = 0;
    mat->m[1][0] = 0; mat->m[1][1] = y; mat->m[1][2] = 0;
    mat->m[2][0] = 0; mat->m[2][1] = 0; mat->m[2][2] = 1;
}

// Create a rotation matrix (radians)
// Formula:
//    [ cosθ  -sinθ  0 ]
//    [ sinθ   cosθ  0 ]
//    [ 0      0     1 ]
void Mat33_Rot(Mat33* mat, float rad) {
    float cos_value = cosf(rad);
    float sin_value = sinf(rad);

    mat->m[0][0] = cos_value; mat->m[0][1] = -sin_value; mat->m[0][2] = 0;
    mat->m[1][0] = sin_value; mat->m[1][1] = cos_value; mat->m[1][2] = 0;
    mat->m[2][0] = 0; mat->m[2][1] = 0; mat->m[2][2] = 1;
}

// Create a translation matrix
// Formula:
//    [ 1  0  x ]
//    [ 0  1  y ]
//    [ 0  0  1 ]
void Mat33_Trans(Mat33* mat, float x, float y) {
    mat->m[0][0] = 1; mat->m[0][1] = 0; mat->m[0][2] = x;
    mat->m[1][0] = 0; mat->m[1][1] = 1; mat->m[1][2] = y;
    mat->m[2][0] = 0; mat->m[2][1] = 0; mat->m[2][2] = 1;
}

// Concatenate two matrices: con_cat = a * b
// Formula: standard matrix multiplication
//    con_cat[i][j] = sum(a[i][k] * b[k][j]) for k = 0..2
void Mat33_Con_cat(Mat33* con_cat, const Mat33* a, const Mat33* b) {
    Mat33 temp;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            temp.m[i][j] = 0.0f;
            for (int k = 0; k < 3; k++)
            {
                temp.m[i][j] += a->m[i][k] * b->m[k][j];
            }
        }
    }
    *con_cat = temp;
}

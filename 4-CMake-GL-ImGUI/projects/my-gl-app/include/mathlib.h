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
} Mat44;

//------------------------------------------
// **GLM Conversions for graphics coders only**
// usage: Before including your mathlib.h, 
// define this #define USE_GLM_CONVERSION
// 
// Normal matrix to GLM 
// eg. Vector2D myVec2{1.0f, 2.0f};
// glm::vec2 glmVec2 = myVec2; 
//
// GLM to Normal Matrix
// eg. glm::vec2 gVec2(5.0f, 6.0f);
//  Vector2D myVec2FromGLM(gVec2);
// 
//----------------------------------------
#ifdef GLM_CONVERSION
#include <glm/glm.hpp>

//---------------- Vector2D ----------------
struct Vector2D {
    float x, y;

    operator glm::vec2() const { return glm::vec2(x, y); }
    Vector2D(const glm::vec2& v) : x(v.x), y(v.y) {}
    Vector2D() : x(0), y(0) {}
};

//---------------- Vector3D ----------------
struct Vector3D {
    float x, y, z;

    operator glm::vec3() const { return glm::vec3(x, y, z); }
    Vector3D(const glm::vec3& v) : x(v.x), y(v.y), z(v.z) {}
    Vector3D() : x(0), y(0), z(0) {}
};

//---------------- Mat33 ----------------
struct Mat33 {
    float m[3][3];

    operator glm::mat3() const {
        return glm::mat3(
            m[0][0], m[0][1], m[0][2],
            m[1][0], m[1][1], m[1][2],
            m[2][0], m[2][1], m[2][2]
        );
    }

    Mat33(const glm::mat3& g) {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                m[i][j] = g[i][j];
    }
    Mat33() { for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) m[i][j] = 0; }
};

//---------------- Mat44 ----------------
struct Mat44 {
    float m[4][4];

    operator glm::mat4() const {
        return glm::mat4(
            m[0][0], m[0][1], m[0][2], m[0][3],
            m[1][0], m[1][1], m[1][2], m[1][3],
            m[2][0], m[2][1], m[2][2], m[2][3],
            m[3][0], m[3][1], m[3][2], m[3][3]
        );
    }

    Mat44(const glm::mat4& g) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                m[i][j] = g[i][j];
    }
    Mat44() { for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) m[i][j] = 0; }
};

#endif //GLM_CONVERSION

//----------------------------------------
// Math Functions
//----------------------------------------

float Math_Sqrt(float v);
float Math_Fabs(float v);
float Math_Fmax(float a, float b);
float Math_Min(float a, float b);
float Math_Max(float a, float b);

// Angle conversion
float DegToRad(float deg);
float RadToDeg(float rad);

// Radian trigonometry
float SinRad(float rad);
float CosRad(float rad);
float TanRad(float rad);
float ASinRad(float val);
float ACosRad(float val);
float ATanRad(float val);

// Degree trigonometry
float SinDeg(float deg);
float CosDeg(float deg);
float TanDeg(float deg);
float ASinDeg(float val);
float ACosDeg(float val);
float ATanDeg(float val);

// Vector2D
Vector2D Vec_Add(const Vector2D* a, const Vector2D* b);
Vector2D Vec_Sub(const Vector2D* a, const Vector2D* b);
Vector2D Vec_Normalize(const Vector2D* v);
float Vec_Length(const Vector2D* v);
float Vec_Distance(const Vector2D* a, const Vector2D* b);
float Vec_Dot(const Vector2D* a, const Vector2D* b);
float Vec_Cross(const Vector2D* a, const Vector2D* b);
Vector2D Vec_Negate(const Vector2D* v);
Vector2D Vec_FromAngle(float rad);
Vector2D Vec_Set(float x, float y);
Vector2D Vec_Zero();
Vector2D Vec_Scale(const Vector2D* v, float scale);
Vector2D Vec_Rotate(const Vector2D* v, float rad);
Vector2D Vec_Transform(const Vector2D* v, const Mat33* mat);

// Mat33
void Mat33_Identity(Mat33* mat);
void Mat33_Transpose(Mat33* out, const Mat33* mat);
float Mat33_Det(const Mat33* mat);
void Mat33_Scale(Mat33* mat, float x, float y);
void Mat33_Rot(Mat33* mat, float rad);
void Mat33_Trans(Mat33* mat, float x, float y);
void Mat33_Con_cat(Mat33* con_cat, const Mat33* a, const Mat33* b);

// Vector3D
Vector3D Vec3_Add(const Vector3D* a, const Vector3D* b);
Vector3D Vec3_Sub(const Vector3D* a, const Vector3D* b);
Vector3D Vec3_Negate(const Vector3D* v);
Vector3D Vec3_Set(float x, float y, float z);
Vector3D Vec3_Zero();
float Vec3_Length(const Vector3D* v);
float Vec3_Distance(const Vector3D* a, const Vector3D* b);
float Vec3_Dot(const Vector3D* a, const Vector3D* b);
Vector3D Vec3_Cross(const Vector3D* a, const Vector3D* b);
Vector3D Vec3_Normalize(const Vector3D* v);
Vector3D Vec3_Scale(const Vector3D* v, float scale);
Vector3D Vec3_Transform(const Vector3D* v, const Mat44* mat);

// Mat44
void Mat44_Identity(Mat44* mat);
void Mat44_Transpose(Mat44* out, const Mat44* mat);
void Mat44_Scale(Mat44* mat, float x, float y, float z);
void Mat44_RotX(Mat44* mat, float rad);
void Mat44_RotY(Mat44* mat, float rad);
void Mat44_RotZ(Mat44* mat, float rad);
void Mat44_Trans(Mat44* mat, float x, float y, float z);
void Mat44_Con_cat(Mat44* con_cat, const Mat44* a, const Mat44* b);

#endif 


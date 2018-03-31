#ifndef MATH_H
#define MATH_H

#include <stdint.h>
#include <tgmath.h>

//avx
#include <immintrin.h> 

#define PI 3.14159265


typedef struct Vec4
{
    float x,y,z,w;
} Vec4;

#define VEC4(X,Y,Z,W) (Vec4){.x=(X),.y=(Y),.z=(Z),.w=(W)}

Vec4 crossProduct( Vec4 v1, Vec4 v2);
float dotProduct( Vec4 v1, Vec4 v2);
float dotProductV3(Vec4 v1,Vec4 v2);
Vec4 apply_matrix4x4(float matrix[4][4], Vec4 v);
void matrixProduct(float matrix1[4][4],float matrix2[4][4]);

int32_t max(int32_t a,int32_t b);
int32_t min(int32_t a,int32_t b);
int32_t clamp(int32_t to_clamp,int32_t floor,int32_t roof);
int32_t interpolate(float i,int32_t floor,int32_t roof);

float maxf(float a,float b);
float minf(float a,float b);


Vec4 add( Vec4 v1, Vec4 v2);
Vec4 scale(float s, Vec4 v);


float length(Vec4 v);
float lengthV3(Vec4 v);
Vec4 normalizeV3(Vec4 v);

#endif
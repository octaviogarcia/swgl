#ifndef MATH_H
#define MATH_H

#include <stdint.h>


#define PI 3.14159265

struct Vec2
{
    float x,y;
};

struct Vec3
{
    float x,y,z;
};

struct Vec4
{
    float x,y,z,w;
};



#define VEC2(X,Y)     (struct Vec2){.Ox=(X),.y=(Y)}
#define VEC3(X,Y,Z)   (struct Vec3){.x=(X),.y=(Y),.z=(Z)}
#define VEC4(X,Y,Z,W) (struct Vec4){.x=(X),.y=(Y),.z=(Z),.w=(W)}

struct Vec4 crossProduct(struct Vec4 v1,struct Vec4 v2);
float dotProduct(struct Vec4 v1,struct Vec4 v2);
struct Vec4 apply_matrix4x4(float matrix[4][4],struct Vec4 v);

int32_t max(int32_t a,int32_t b);
int32_t min(int32_t a,int32_t b);
int32_t clamp(int32_t to_clamp,int32_t floor,int32_t roof);
int32_t interpolate(float i,int32_t floor,int32_t roof);

float maxf(float a,float b);
float minf(float a,float b);


struct Vec4 add(struct Vec4 v1,struct Vec4 v2);
struct Vec4 scale(float s,struct Vec4 v);
#endif
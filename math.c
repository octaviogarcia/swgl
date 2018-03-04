#include "math.h"

float maxf(float a,float b)
{
    if(a>b) return a;
    return b;
}
float minf(float a,float b)
{
    if(a>b) return b;
    return a;
}

float dotProduct(struct Vec4 v1,struct Vec4 v2)
{
    return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z+v1.w*v2.w;
}

struct Vec4 add(struct Vec4 v1,struct Vec4 v2)
{
    return VEC4(v1.x+v2.x,v1.y+v2.y,v1.z+v2.z,v1.w+v2.w);
}
struct Vec4 scale(float s,struct Vec4 v)
{
    return VEC4(s*v.x,s*v.y,s*v.z,s*v.w);
}

struct Vec4 apply_matrix4x4(float matrix[4][4],struct Vec4 v)
{
    float x = matrix[0][0]*v.x+matrix[0][1]*v.y+matrix[0][2]*v.z+matrix[0][3]*v.w;
    float y = matrix[1][0]*v.x+matrix[1][1]*v.y+matrix[1][2]*v.z+matrix[1][3]*v.w;
    float z = matrix[2][0]*v.x+matrix[2][1]*v.y+matrix[2][2]*v.z+matrix[2][3]*v.w;
    float w = matrix[3][0]*v.x+matrix[3][1]*v.y+matrix[3][2]*v.z+matrix[3][3]*v.w;
    
    return VEC4(x,y,z,w);
}

struct Vec4 crossProduct(struct Vec4 v1,struct Vec4 v2)
{
    /*
    | i   j  k |
    | x1 y1 z1 |
    | x2 y2 z2 |
    
    y1*z2 - y2*z1
    x2*z1 - x1*z2
    x1*y2 - x2*y1
    */
    
    float x1=v1.x;
    float y1=v1.y;
    float z1=v1.z;
    float x2=v2.x;
    float y2=v2.y;
    float z2=v2.z;
    
    return VEC4(y1*z2-y2*z1,x2*z1-x1*z2,x1*y2-x2*y1,1.0);
}


int32_t max(int32_t a,int32_t b)
{
    if(a>=b) return a;
    return b;
}

int32_t min(int32_t a,int32_t b)
{
    if(a<=b) return a;
    return b;
}

int32_t clamp(int32_t to_clamp,int32_t floor,int32_t roof)
{
    return max(floor,min(roof,to_clamp));
}

int32_t interpolate(float i,int32_t floor,int32_t roof)
{
    if(i>=1.0) return roof;
    if(i<=0.0) return floor;
    
    return roof*i+floor*(1.0-i);
}
#include "math.h"
//https://stackoverflow.com/questions/427477/fastest-way-to-clamp-a-real-fixed-floating-point-value
float maxf(float a,float b)
{
    __m128 a4 =  _mm_load_ss(&a);
    __m128 b4 =  _mm_load_ss(&b);
    a4 = _mm_max_ss(a4,b4);
    _mm_store_ss(&a,a4);
    return a;
}

float minf(float a,float b)
{
    __m128 a4 =  _mm_load_ss(&a);
    __m128 b4 =  _mm_load_ss(&b);
    a4 = _mm_min_ss(a4,b4);
    
    _mm_store_ss(&a,a4);
    return a;
    
}
float clampf(float f,float floor,float roof)
{
    __m128 f4     =  _mm_load_ss(&f);
    __m128 floor4 =  _mm_load_ss(&floor);
    __m128 roof4  =  _mm_load_ss(&roof);
    
    f4 = _mm_max_ss(floor4,_mm_min_ss(roof4,f4));
    _mm_store_ss(&f,f4);
    
    return f;
}

float dotProduct( Vec4 v1, Vec4 v2)
{
    return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z+v1.w*v2.w;
}

float dotProductV3(Vec4 v1,Vec4 v2)
{
    v1.w = 0;
    return dotProduct(v1,v2);
}


float length(Vec4 v)
{
    return sqrt(dotProduct(v,v));
}

float lengthV3(Vec4 v)
{
    v.w = 0;
    return length(v);
}

Vec4 normalizeV3(Vec4 v)
{
    float length = lengthV3(v);
    return VEC4(v.x/length,v.y/length,v.z/length,1);
}
Vec4 add( Vec4 v1, Vec4 v2)
{
    return VEC4(v1.x+v2.x,v1.y+v2.y,v1.z+v2.z,v1.w+v2.w);
}
Vec4 scale(float s, Vec4 v)
{
    return VEC4(s*v.x,s*v.y,s*v.z,s*v.w);
}

Vec4 apply_matrix4x4(float matrix[4][4], Vec4 v)
{
    float x = matrix[0][0]*v.x+matrix[0][1]*v.y+matrix[0][2]*v.z+matrix[0][3]*v.w;
    float y = matrix[1][0]*v.x+matrix[1][1]*v.y+matrix[1][2]*v.z+matrix[1][3]*v.w;
    float z = matrix[2][0]*v.x+matrix[2][1]*v.y+matrix[2][2]*v.z+matrix[2][3]*v.w;
    float w = matrix[3][0]*v.x+matrix[3][1]*v.y+matrix[3][2]*v.z+matrix[3][3]*v.w;
    
    return VEC4(x,y,z,w);
}

Vec4 crossProduct( Vec4 v1, Vec4 v2)
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
    /*
    __m128i a4 = _mm_set_epi32(a,a,a,a);
    __m128i b4 = _mm_set_epi32(b,b,b,b);
    a4 = _mm_max_epi32(a4,b4);
    return _mm_extract_epi32(a4,0);
    */
    if(a>=b) return a;
    return b;
}

int32_t min(int32_t a,int32_t b)
{
    /*
    __m128i a4 = _mm_set_epi32(a,a,a,a);
    __m128i b4 = _mm_set_epi32(b,b,b,b);
    a4 = _mm_min_epi32(a4,b4);
    return _mm_extract_epi32(a4,0);
    */
    if(a<=b) return a;
    return b;
}

int32_t clamp(int32_t n,int32_t floor,int32_t roof)
{
    /*
    __m128i n4 = _mm_set_epi32(n,n,n,n);
    __m128i floor4 = _mm_set_epi32(floor,floor,floor,floor);
    __m128i roof4 = _mm_set_epi32(roof,roof,roof,roof);
    
    n4 = _mm_max_epi32(floor4,_mm_min_epi32(roof4,n4));
    return _mm_extract_epi32(n4,0);
    */
    return max(floor,min(roof,n));
}

int32_t interpolate(float i,int32_t floor,int32_t roof)
{
    if(i>=1.0) return roof;
    if(i<=0.0) return floor;
    
    return roof*i+floor*(1.0-i);
}

void matrixProduct(float b[4][4],float a[4][4])
{
    /*
    
    
    ................ a00 a01 a02 a03
    ................ a10 a11 a12 a13
    ................ a20 a21 a22 a23
     ................ a30 a31 a32 a33
     
    b00 b01 b02 b03  b00*a00+b01*a10+b02*a20+b03*b30
    b10 b11 b12 b13
    b20 b21 b22 b23
    b30 b31 b32 b33
    
    */
    
    float result[4][4];
    
    for(int j=0;j<4;j++)
    {
        for(int k=0;k<4;k++)
        {
            result[j][k]=0;
            for(int i = 0;i<4;i++)
            {
                result[j][k]+=b[j][i]*a[i][k];
            }
        }
    }
    for(int j=0;j<4;j++)
    {
        for(int k=0;k<4;k++) a[j][k]=result[j][k];
    }
}
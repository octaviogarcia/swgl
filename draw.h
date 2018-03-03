#ifndef DRAW_H
#define DRAW_H

#include <stdlib.h>
//http://math.msu.su/~vvb/2course/Borisenko/CppProjects/GWindow/hi.c
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

extern Display *dis;
extern int screen;
extern Window win;
extern GC gc;

extern float scale_y;
extern float scale_x;
extern float scale_z;

extern int window_width_px;
extern int window_height_px;
extern float deltax;
extern float deltay;

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

#define VEC2(X,Y)     (struct Vec2){.x=(X),.y=(Y)}
#define VEC3(X,Y,Z)   (struct Vec3){.x=(X),.y=(Y),.z=(Z)}
#define VEC4(X,Y,Z,W) (struct Vec4){.x=(X),.y=(Y),.z=(Z),.w=(W)}

/* here are our X routines declared! */
void init_x();
void close_x();
void redraw();


int32_t colori_set_red(int32_t color,char red);
int32_t colori_set_green(int32_t color,char green);
int32_t colori_set_blue(int32_t color,char blue);
int32_t colori_get_red(int32_t color);
int32_t colori_get_green(int32_t color);
int32_t colori_get_blue(int32_t color);
int32_t colori(char red,char green,char blue);
int32_t max(int32_t a,int32_t b);
int32_t min(int32_t a,int32_t b);
int32_t clamp(int32_t to_clamp,int32_t floor,int32_t roof);
int32_t interpolate(float i,int32_t floor,int32_t roof);
int32_t colorf(float red,float green,float blue);
int32_t colori_delta_red(int32_t color,int32_t delta);
int32_t colori_delta_green(int32_t color,int32_t delta);
int32_t colori_delta_blue(int32_t color,int32_t delta);
void get_window_size(int* width,int* height);
struct Vec4 crossProduct(struct Vec4 v1,struct Vec4 v2);
float dotProduct(struct Vec4 v1,struct Vec4 v2);

//Deprecated?
void draw_triangle(struct Vec4* points,int index1,int index2,int index3);

float maxf(float a,float b);
float minf(float a,float b);
XPoint to_screen_coords(float x,float y);

void pipeline(struct Vec4* points,int index0,int index1,int index2,
              void* attributes,int attributes_size,int aindex0,int aindex1,int aindex2);

//TODO: convert these to function pointers ;)
void* vertexShader(struct Vec4* vertex,void* attribute);
int32_t fragmentShader(float fragx,float fragy,struct Vec4 triangle[3],float lambda0,float lambda1, float lambda2,void * vertexOut[3]);
#endif
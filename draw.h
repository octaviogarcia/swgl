#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>

#include <stdlib.h>
//http://math.msu.su/~vvb/2course/Borisenko/CppProjects/GWindow/hi.c
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "math.h"

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

#define IS_BIG_ENDIAN (!*(unsigned char *)&(uint16_t){1})

#if IS_BIG_ENDIAND
//guess this depends on endiannes
typedef
union
{
    struct
    {
        uint8_t a;
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } rgb;
    
    int32_t integer;
} Color;
#else
typedef
union
{
    struct
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    } rgb;
    
    int32_t integer;
    
} Color;
#endif

/* here are our X routines declared! */
void init_x();
void close_x();
void redraw();

Color colori(uint8_t red,uint8_t green,uint8_t blue);
Color colorf(float red,float green,float blue);
Color colori_delta_red(Color color,int32_t delta);
Color colori_delta_green(Color color,int32_t delta);
Color colori_delta_blue(Color color,int32_t delta);

void get_window_size(int* width,int* height);

//Deprecated?
void draw_triangle(struct Vec4* points,int index1,int index2,int index3);

XPoint to_screen_coords(float x,float y);

void pipeline(struct Vec4* points,int index0,int index1,int index2,
              void* attributes,int attributes_size,int aindex0,int aindex1,int aindex2);

//TODO: convert these to function pointers ;)
void* vertexShader(struct Vec4* vertex,void* attribute);
struct Vec4 fragmentShader(float fragx,float fragy,struct Vec4 triangle[3],float lambda0,
                           float lambda1, float lambda2,void * vertexOut[3]);
#endif
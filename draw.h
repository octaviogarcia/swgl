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

extern double scale_y;
extern double scale_x;

#define W_WIDTH 300
#define W_HEIGHT 300

struct Vec2
{
    double x,y;
};

struct Vec3
{
    double x,y,z;
};

struct Vec4
{
    double x,y,z,w;
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
int32_t interpolate(double i,int32_t floor,int32_t roof);
int32_t colorf(double red,double green,double blue);
int32_t colori_delta_red(int32_t color,int32_t delta);
int32_t colori_delta_green(int32_t color,int32_t delta);
int32_t colori_delta_blue(int32_t color,int32_t delta);
void get_window_size(int* width,int* height);
void draw_triangle(struct Vec4* points,int index1,int index2,int index3);
struct Vec4 crossProduct(struct Vec4 v1,struct Vec4 v2);
double dotProduct(struct Vec4 v1,struct Vec4 v2);
double division(struct Vec4 v1,struct Vec4 v2);
#endif
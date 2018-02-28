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

#define W_WIDTH 300
#define W_HEIGHT 300


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

#endif
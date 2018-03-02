
#include "draw.h"


Display *dis;
int screen;
Window win;
GC gc;

double scale_x = 1.0;
double scale_y = 1.0;

int32_t colori_set_red(int32_t color,char red)
{
    color &= 0x0000FFFF;
    color |= ((red<<16) & 0x00FF0000);
    return color;
}
int32_t colori_set_green(int32_t color,char green)
{
    color &= 0x00FF00FF;
    color |= ((green<<8) & 0x0000FF00);
    return color;
}
int32_t colori_set_blue(int32_t color,char blue)
{
    color &= 0x00FFFF00;
    color |= (blue & 0x000000FF);
    return color;
}

int32_t colori_get_red(int32_t color)
{
    return (color>>16) & 0x000000FF;
}

int32_t colori_get_green(int32_t color)
{
    return (color>>8)  & 0x000000FF;
}

int32_t colori_get_blue(int32_t color)
{
    return (color) & 0x000000FF;
}

int32_t colori(char red,char green,char blue)
{
    //  00000000 00000000 00000000 00000000
    //  00000000 rrrrrrrr  gggggggg bbbbbbbb
    
    int32_t result=0;
    
    result = colori_set_red(result,red);
    result = colori_set_green(result,green);
    result = colori_set_blue(result,blue);
    
    return result;
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

int32_t interpolate(double i,int32_t floor,int32_t roof)
{
    if(i>=1.0) return roof;
    if(i<=0.0) return floor;
    
    return roof*i+floor*(1.0-i);
}
int32_t colorf(double red,double green,double blue)
{
    char redi = clamp(red*255,0,255);
    char greeni = clamp(green*255,0,255);
    char bluei = clamp(blue*255,0,255);
    return colori(redi,greeni,bluei);
}

int32_t colori_delta_red(int32_t color,int32_t delta)
{
    int32_t red = colori_get_red(color);
    red = clamp(red+delta,0,255);
    return colori_set_red(color,red);
}

int32_t colori_delta_green(int32_t color,int32_t delta)
{
    int32_t green = colori_get_green(color);
    green = clamp(green+delta,0,255);
    return colori_set_green(color,green);
}

int32_t colori_delta_blue(int32_t color,int32_t delta)
{
    int32_t blue = colori_get_blue(color);
    blue = clamp(blue+delta,0,255);
    return colori_set_blue(color,blue);
}



void init_x() {
    /* get the colors black and white (see section for details) */        
    
    
    dis=XOpenDisplay((char *)0);
    screen=DefaultScreen(dis);
    
    //unsigned long black,white;
    //black=BlackPixel(dis,screen),
    //white=WhitePixel(dis, screen);
    //printf("black = %lu, white = %lu, sizeof = %lu\n",black,white,sizeof(long));
    
    win=XCreateSimpleWindow(dis,DefaultRootWindow(dis),0,0,	
                            W_WIDTH, W_HEIGHT, 0, colori(255,255,255),colori(0,0,0));
    XSetStandardProperties(dis,win,"swgl","swgl",None,NULL,0,NULL);
    //https://tronche.com/gui/x/xlib/events/mask.html#NoEventMask
    XSelectInput(dis, win, ExposureMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|KeyPressMask | ResizeRequest);
    gc=XCreateGC(dis, win, 0,0);    
    
    XClearWindow(dis, win);
    XMapRaised(dis, win);
};

void close_x() {
    XFreeGC(dis, gc);
    XDestroyWindow(dis,win);
    XCloseDisplay(dis);	
    exit(1);				
};

void redraw() {
    XClearWindow(dis, win);
};


void get_window_size(int* width,int* height)
{
    
    Window root_return;
    int x_return, y_return;
    unsigned int border_width_return;
    unsigned int depth_return;
    XGetGeometry(dis,win,&root_return,
                 &x_return,&y_return,
                 width,height,
                 &border_width_return,
                 &depth_return);
    
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
    
    double x1=v1.x;
    double y1=v1.y;
    double z1=v1.z;
    double x2=v2.x;
    double y2=v2.y;
    double z2=v2.z;
    
    return VEC4(y1*z2-y2*z1,x2*z1-x1*z2,x1*y2-x2*y1,1.0);
}
double dotProduct(struct Vec4 v1,struct Vec4 v2)
{
    return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z+v1.w*v2.w;
}
double division(struct Vec4 v1,struct Vec4 v2)
{//https://physics.stackexchange.com/questions/111652/can-we-divide-two-vectors
    double num = dotProduct(v1,v2);
    double den = dotProduct(v1,v1);
    return num/den;
}

void draw_triangle(struct Vec4* points,int index1,int index2,int index3)
{
    unsigned int width=0,height=0;
    
    get_window_size(&width,&height);
    
    
    
    struct Vec4 p1 = points[index1];
    struct Vec4 p2 = points[index2];
    struct Vec4 p3 = points[index3];
    
    double factorx = width/scale_x;
    double factory = height/scale_y;
    
    XPoint v1 = (XPoint){.x=factorx*p1.x,.y=factory*(scale_y-p1.y)};
    XPoint v2 = (XPoint){.x=factorx*p2.x,.y=factory*(scale_y-p2.y)};
    XPoint v3 = (XPoint){.x=factorx*p3.x,.y=factory*(scale_y-p3.y)};
    XPoint v4 = v1;
    
    XPoint v[4] = {v1,v2,v3,v4};
    
    XFillPolygon(dis,win,gc,v,4,Convex,CoordModeOrigin);
}

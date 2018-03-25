
#include "draw.h"


Display *dis;
int screen;
Window win;
GC gc;

float scale_x = 1.0;
float scale_y = 1.0;
float scale_z = 1.0;

#define DEFAULTWINDOWWIDTH 300
#define DEFAULTWINDOWHEIGHT 300 
XVisualInfo visinfo = {};
XImage * screen_img = NULL;

Color colori(uint8_t red,uint8_t green,uint8_t blue)
{
    //  00000000 00000000 00000000 00000000
    //  00000000 rrrrrrrr  gggggggg bbbbbbbb
    
    Color result;
    
    result.rgb.a=0;
    result.rgb.r=red;
    result.rgb.g=green;
    result.rgb.b=blue;
    
    return result;
}


Color colorf(float red,float green,float blue)
{
    char redi = clamp(red*255,0,255);
    char greeni = clamp(green*255,0,255);
    char bluei = clamp(blue*255,0,255);
    return colori(redi,greeni,bluei);
}

Color colori_delta_red(Color color,int32_t delta)
{
    int32_t red = color.rgb.r;
    color.rgb.r = clamp(red+delta,0,255);
    return color;
}

Color colori_delta_green(Color color,int32_t delta)
{
    int32_t green = color.rgb.g;
    color.rgb.g = clamp(green+delta,0,255);
    return color;
}

Color colori_delta_blue(Color color,int32_t delta)
{
    int32_t blue = color.rgb.b;
    color.rgb.b = clamp(blue+delta,0,255);
    return color;
}

void UpdateScreenData(int width,int height)
{
    XImage* old_screen_img = screen_img;
    char* mem = malloc(width*height*4);//4B per pixel
    
    screen_img = XCreateImage(dis, visinfo.visual, visinfo.depth,
                              ZPixmap, 0, mem, width, height,
                              32, 0);
    
    if(old_screen_img) XDestroyImage(old_screen_img);
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
                            DEFAULTWINDOWWIDTH, DEFAULTWINDOWHEIGHT, 0, colori(255,255,255).integer,colori(0,0,0).integer);
    XSetStandardProperties(dis,win,"swgl","swgl",None,NULL,0,NULL);
    //https://tronche.com/gui/x/xlib/events/mask.html#NoEventMask
    XSelectInput(dis, win, ExposureMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|KeyPressMask | ResizeRequest);
    gc=XCreateGC(dis, win, 0,0);    
    
    XClearWindow(dis, win);
    XMapRaised(dis, win);
    {
        int screenbitdepth = 24;
        int screen = DefaultScreen(dis);
        if(!XMatchVisualInfo(dis, screen, screenbitdepth, TrueColor, &visinfo)) {
            printf("Couldnt match visual info\n");
        }
    }
    UpdateScreenData(DEFAULTWINDOWWIDTH,DEFAULTWINDOWHEIGHT);
};

void close_x() {
    XDestroyImage(screen_img);
    screen_img = NULL;
    XFreeGC(dis, gc);
    XDestroyWindow(dis,win);
    XCloseDisplay(dis);	
    exit(1);				
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




void draw_triangle(struct Vec4* points,int index1,int index2,int index3)
{
    unsigned int width=0,height=0;
    
    struct Vec4 p1 = points[index1];
    struct Vec4 p2 = points[index2];
    struct Vec4 p3 = points[index3];
    
    float factorx = screen_img->width/scale_x;
    float factory = screen_img->height/scale_y;
    
    XPoint v1 = (XPoint){.x=factorx*p1.x,.y=factory*(scale_y-p1.y)};
    XPoint v2 = (XPoint){.x=factorx*p2.x,.y=factory*(scale_y-p2.y)};
    XPoint v3 = (XPoint){.x=factorx*p3.x,.y=factory*(scale_y-p3.y)};
    XPoint v4 = v1;
    
    XPoint v[4] = {v1,v2,v3,v4};
    
    XFillPolygon(dis,win,gc,v,4,Convex,CoordModeOrigin);
}




XPoint to_screen_coords(float x,float y)
{
    float newx = 0.5f*x*scale_z/scale_x + 0.5f; //(x*scale_z+scale_x)/(2*scale_x)
    float newy = 0.5f - 0.5f*y*scale_z/scale_y; //1.0 minus (y*scale_z+scale_y)/(2*scale_y)..
    return (XPoint){.x=screen_img->width*newx,.y=screen_img->height*newy};
}

void pipeline(struct Vec4* points,int index0,int index1,int index2,
              void* attributes,int attributes_size,int aindex0,int aindex1,int aindex2)
{
    struct Vec4 triangle[3]={points[index0],points[index1],points[index2]};
    
    
    void* attribute0 = attributes+attributes_size*aindex0;
    void* attribute1 = attributes+attributes_size*aindex1;
    void* attribute2 = attributes+attributes_size*aindex2;
    
    //use globals as uniforms?
    void* vertexOut0 = vertexShader(&triangle[0],attribute0);//callback
    void* vertexOut1 = vertexShader(&triangle[1],attribute1);//callback
    void* vertexOut2 = vertexShader(&triangle[2],attribute2);//callback
    
    void* vertexOut[3] = {vertexOut0,vertexOut1,vertexOut2};
    
    
    
    //Rasterize
    //Make a fragment, sequencially for now
    //http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
    //https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage
    
    //If its too close, dont draw...
    if(triangle[0].z <= 0.05f) return;
    if(triangle[1].z <= 0.05f) return;
    if(triangle[2].z <= 0.05f) return;
    if(triangle[0].z <= 0.05f) return;
    //Should we do the same for too far?
    
    triangle[0].x/=triangle[0].z;
    triangle[0].y/=triangle[0].z;
    triangle[0].z=1;
    triangle[1].x/=triangle[1].z;
    triangle[1].y/=triangle[1].z;
    triangle[1].z=1;
    triangle[2].x/=triangle[2].z;
    triangle[2].y/=triangle[2].z;
    triangle[2].z=1;
    
    float t0x = triangle[0].x;
    float t0y = triangle[0].y;
    float t1x = triangle[1].x;
    float t1y = triangle[1].y;
    float t2x = triangle[2].x;
    float t2y = triangle[2].y;
    float maxX = maxf(t0x, maxf(t1x, t2x));
    float minX = minf(t0x, minf(t1x, t2x));
    float maxY = maxf(t0y, maxf(t1y, t2y));
    float minY = minf(t0y, minf(t1y, t2y));
    
    //https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
    //https://en.wikipedia.org/wiki/Barycentric_coordinate_system
    
    float d1y2y = t1y-t2y;
    float d2x1x = t2x-t1x;
    float d2y0y = t2y-t0y;
    float d0x2x = t0x-t2x;
    float d0y2y = -d2y0y;
    
    float detT = d1y2y * d0x2x + d2x1x * d0y2y;
    
    float lambda0_prime =  -(d1y2y*t2x  + d2x1x*t2y);
    float lambda1_prime =  -(d2y0y*t2x  + d0x2x*t2y);
    
    //clipping
    if(minX < -scale_x) minX = -scale_x;
    if(minY < -scale_y) minY = -scale_y;
    if(maxX >  scale_x) maxX = scale_y;
    if(maxY >  scale_y) maxY = scale_y;
    
    int width = screen_img->width;
    int height = screen_img->height;
    
    //"how much" you need to move, to move to another pixel
    float deltax = 1.0f/screen_img->width;
    float deltay = 1.0f/screen_img->height;
    
    for(float x = minX;x<=maxX;x+=deltax)
    {
        for(float y = minY;y<=maxY;y+=deltay)
        {
            float lambda0 = d1y2y * x  + d2x1x * y + lambda0_prime;
            if(lambda0 < 0) continue;
            
            float lambda1 = d2y0y * x  + d0x2x * y + lambda1_prime;
            if(lambda1 < 0) continue;
            
            lambda0/=detT;
            lambda1/=detT;
            
            float lambda2 = 1 - lambda0 - lambda1;
            
            if(lambda2 >= 0)
            {
                //callback
                //use globals as uniforms?
                struct Vec4 pixel_color = fragmentShader(x,y,triangle,lambda0,lambda1,lambda2,vertexOut);//callback
                
                Color c;
                c.rgb.r = clamp(pixel_color.x*255,0,255);
                c.rgb.g = clamp(pixel_color.y*255,0,255);
                c.rgb.b = clamp(pixel_color.z*255,0,255);
                c.rgb.a = clamp(pixel_color.w*255,0,255);
                
                
                XPoint pixel_coords = to_screen_coords(x,y);
                
                //Is there a way to do this without branching?
                if(pixel_coords.x<0 || pixel_coords.x>=width) continue;
                if(pixel_coords.y<0 || pixel_coords.y>=height) continue;
                
                int32_t * pixels = (int32_t*)(screen_img->data);
                pixels[pixel_coords.x+width*pixel_coords.y]=c.integer;
            }
        }
    }
}
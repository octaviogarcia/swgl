
#include "draw.h"


Display *dis;
int screen;
Window win;
GC gc;

float scale_x = 1.0;
float scale_y = 1.0;
float scale_z = 1.0;

int window_width_px=300;//update below pls if you change this
int window_height_px=300;//update below pls if you change this
float deltax = 1.0f/300;//divided by width
float deltay = 1.0f/300;//divided by height

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

int32_t interpolate(float i,int32_t floor,int32_t roof)
{
    if(i>=1.0) return roof;
    if(i<=0.0) return floor;
    
    return roof*i+floor*(1.0-i);
}
int32_t colorf(float red,float green,float blue)
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
                            window_width_px, window_height_px, 0, colori(255,255,255),colori(0,0,0));
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
    
    float x1=v1.x;
    float y1=v1.y;
    float z1=v1.z;
    float x2=v2.x;
    float y2=v2.y;
    float z2=v2.z;
    
    return VEC4(y1*z2-y2*z1,x2*z1-x1*z2,x1*y2-x2*y1,1.0);
}
float dotProduct(struct Vec4 v1,struct Vec4 v2)
{
    return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z+v1.w*v2.w;
}

void draw_triangle(struct Vec4* points,int index1,int index2,int index3)
{
    unsigned int width=0,height=0;
    
    get_window_size(&width,&height);
    
    struct Vec4 p1 = points[index1];
    struct Vec4 p2 = points[index2];
    struct Vec4 p3 = points[index3];
    
    float factorx = width/scale_x;
    float factory = height/scale_y;
    
    XPoint v1 = (XPoint){.x=factorx*p1.x,.y=factory*(scale_y-p1.y)};
    XPoint v2 = (XPoint){.x=factorx*p2.x,.y=factory*(scale_y-p2.y)};
    XPoint v3 = (XPoint){.x=factorx*p3.x,.y=factory*(scale_y-p3.y)};
    XPoint v4 = v1;
    
    XPoint v[4] = {v1,v2,v3,v4};
    
    XFillPolygon(dis,win,gc,v,4,Convex,CoordModeOrigin);
}



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


XPoint to_screen_coords(float x,float y)
{
    float newx = 0.5f*x*scale_z/scale_x + 0.5f; //(x*scale_z+scale_x)/(2*scale_x)
    float newy = 0.5f - 0.5f*y*scale_z/scale_y; //same but 1.0 minus (y*scale_z+scale_y)/(2*scale_y)..
    return (XPoint){.x=window_width_px*newx,.y=window_height_px*newy};
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
#if 0
    printf("Vertex1 %f %f %f %f\n",t0x,t0y,triangle[0].z,triangle[0].w);
    printf("Vertex2 %f %f %f %f\n",t1x,t1y,triangle[1].z,triangle[1].w);
    printf("Vertex3 %f %f %f %f\n",t2x,t2y,triangle[2].z,triangle[2].w);
    printf("X %f %f Y %f %f\n",minX,maxX,minY,maxY);
    printf("width %d height %d\n",window_width_px,window_height_px);
    printf("deltax %f deltay %f\n",deltax,deltay);
#endif
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
                struct Vec2 FragmentPos = VEC2(x,y);
                //callback
                //use globals as uniforms?
                int32_t pixel_color = fragmentShader(x,y,triangle,lambda0,lambda1,lambda2,vertexOut);//callback
                
                XPoint pixel_coords = to_screen_coords(x,y);
                XSetForeground(dis,gc,pixel_color);
                XDrawPoint(dis,win,gc,pixel_coords.x,pixel_coords.y);
            }
        }
    }
}
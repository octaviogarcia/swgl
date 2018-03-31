
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
float * depth_buffer = NULL;

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

void clear_depth_buffer()
{
    int buffer_size = screen_img->width*screen_img->height;
    
    for(int i = 0;i<buffer_size;++i)
    {
        depth_buffer[i]=1.0f/0.0f;
    }
    
}
void clear_image()
{
    memset((screen_img->data),0,
           screen_img->width*
           screen_img->height*
           screen_img->bits_per_pixel/8);
}


void clear_all_buffers()
{
    int buffer_size = screen_img->width*screen_img->height;
    int32_t* data = (int32_t*)screen_img->data;
    
    for(int i = 0;i<buffer_size;++i)
    {
        depth_buffer[i]=1.0f/0.0f;
        data[i]=0;
    }
}

void UpdateScreenData(int width,int height)
{
    XImage* old_screen_img = screen_img;
    char* mem = malloc(width*height*4);//4B per pixel
    
    screen_img = XCreateImage(dis, visinfo.visual, visinfo.depth,
                              ZPixmap, 0, mem, width, height,
                              32, 0);
    
    if(old_screen_img) XDestroyImage(old_screen_img);
    if(depth_buffer) free(depth_buffer);
    depth_buffer = malloc(width*height*sizeof(float));
    //printf("isinf(depth_buffer[0])=%d\n",isinf(depth_buffer[0]));
    clear_all_buffers();
}


void init_x() {
    dis=XOpenDisplay((char *)0);
    screen=DefaultScreen(dis);
    
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
    XFreeGC(dis, gc);
    XDestroyWindow(dis,win);
    XCloseDisplay(dis);	
    exit(1);				
};

void get_window_size(unsigned int* width,unsigned int* height)
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




void draw_triangle( Vec4* points,int index1,int index2,int index3)
{
    unsigned int width=0,height=0;
    
    Vec4 p1 = points[index1];
    Vec4 p2 = points[index2];
    Vec4 p3 = points[index3];
    
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


static void sort_counter_clockwise(Vec4 triangle[3])
{
    float centerx = triangle[0].x+triangle[1].x+triangle[2].x;
    float centery = triangle[0].y+triangle[1].y+triangle[2].y;
    centerx/=3.0f;
    centery/=3.0f;
    
    float center_to_a_x = triangle[0].x - centerx;
    float center_to_a_y = triangle[0].y - centery;
    float center_to_b_x = triangle[1].x - centerx;
    float center_to_b_y = triangle[1].y - centery;
    
    //by sine law, if its negative, the angle goes "downwards" i.e. clockwise
    float cross_product = 
        center_to_a_x*center_to_b_y -
        center_to_a_y*center_to_b_x;
    
    //swap em
    if(cross_product<0) 
    {
        Vec4 tmp = triangle[0];
        triangle[0]=triangle[1];
        triangle[1]=tmp;
    }
    return;
}

void pipeline( Vec4* points,int index0,int index1,int index2,
              void* attributes,int attributes_size,int aindex0,int aindex1,int aindex2)
{
    Vec4 triangle[3]={points[index0],points[index1],points[index2]};
    
    
    void* attribute0 = attributes+attributes_size*aindex0;
    void* attribute1 = attributes+attributes_size*aindex1;
    void* attribute2 = attributes+attributes_size*aindex2;
    
    //use globals as uniforms?
    void* vertexOut0 = vertexShader(&triangle[0],attribute0);//callback
    void* vertexOut1 = vertexShader(&triangle[1],attribute1);//callback
    void* vertexOut2 = vertexShader(&triangle[2],attribute2);//callback
    
    void* vertexOut[3] = {vertexOut0,vertexOut1,vertexOut2};
    
    
    //@HACK: is there a better way?
    //sort counter clockwise... when we are 
    //actually drawing with bayesian coordinates
    //so lambdas have proper positive values
    //https://stackoverflow.com/questions/6989100/sort-points-in-clockwise-order
    sort_counter_clockwise(triangle);
    
    
    //Rasterize
    //Make a fragment, sequencially for now
    //http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
    //https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage
    
    //If its too close, dont draw...
    
    /*if(triangle[0].z <= 0.05f) return;
    if(triangle[1].z <= 0.05f) return;
    if(triangle[2].z <= 0.05f) return;
    if(triangle[0].z <= 0.05f) return;*/
    //Should we do the same for too far?
    
    //needed for the z buffer, if we divide it by w you would get 1...
    float t0z = triangle[0].z;
    float t1z = triangle[1].z;
    float t2z = triangle[2].z;
    
    float t0w = triangle[0].w;
    float t1w = triangle[1].w;
    float t2w = triangle[2].w;
    
    triangle[0].x/=t0w;
    triangle[0].y/=t0w;
    triangle[0].z/=t0w;
    triangle[0].w=1;
    
    
    triangle[1].x/=t1w;
    triangle[1].y/=t1w;
    triangle[1].z/=t1w;
    triangle[1].w=1;
    
    
    triangle[2].x/=t2w;
    triangle[2].y/=t2w;
    triangle[2].z/=t2w;
    triangle[2].w=1;
    
    
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
    
    //used to avoid redrawing samepixel
    //fixes artifacs and may get some perf
    int drawn_pos = -1;
    
    for(float y = maxY;y>=minY;y-=deltay)
    {
        for(float x = minX;x<=maxX;x+=deltax)
        {
            
            XPoint pixel_coords = to_screen_coords(x,y);
            int buffer_pos = pixel_coords.x+width*pixel_coords.y;
            if(pixel_coords.x<0 || pixel_coords.x>=width) continue;
            if(pixel_coords.y<0 || pixel_coords.y>=height) continue;
            if(buffer_pos<=drawn_pos) continue;
            drawn_pos=buffer_pos;
            
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
                float z = lambda0*t0z+lambda1*t1z+lambda2*t2z;
                if(z>=depth_buffer[buffer_pos]) continue;
                depth_buffer[buffer_pos]=z;
                
                if(z<=0.02f) continue;
                
                Vec4 pixel_color = fragmentShader(x,y,triangle,lambda0,lambda1,lambda2,vertexOut);//callback
                
                int32_t * pixels = (int32_t*)(screen_img->data);
                {//alpha blend
                    Color behind_pixeli = {.integer=pixels[buffer_pos]};
                    
                    float behinda = behind_pixeli.rgb.a/255.0f;
                    float behindr = behinda*behind_pixeli.rgb.r/255.0f;
                    float behindg = behinda*behind_pixeli.rgb.g/255.0f;
                    float behindb = behinda*behind_pixeli.rgb.b/255.0f;
                    
                    float a = pixel_color.w;
                    float r = a*pixel_color.x;
                    float g = a*pixel_color.y;
                    float b = a*pixel_color.z;
                    float _a = 1.0f-a;
                    
                    pixel_color.x = r + behindr*_a;
                    pixel_color.y = g + behindg*_a;
                    pixel_color.z = b + behindb*_a;
                    pixel_color.w = a + behinda*_a;
                }
                
                Color c;
                c.rgb.r = clamp(pixel_color.x*255,0,255);
                c.rgb.g = clamp(pixel_color.y*255,0,255);
                c.rgb.b = clamp(pixel_color.z*255,0,255);
                c.rgb.a = clamp(pixel_color.w*255,0,255);
                
                pixels[buffer_pos]=c.integer;
                
            }
        }
    }
}
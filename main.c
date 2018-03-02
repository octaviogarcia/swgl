#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "draw.h"


void* vertexShader(struct Vec4* vertex,void* attribute)
{
    return NULL;
}

//triangle is array of vec4, actually just vec2, z=1
//vertexOut, array of 3 void*
void* interpolateVertexOut(struct Vec2 FragmentPos,struct Vec4* triangle,void ** vertexOut)
{
    return NULL;
}

int32_t fragmentShader(struct Vec2 FragmentPos,void* ivertexOut)
{
    return colori(255,255,255);
}

XPoint to_screen_coords(struct Vec2 FragmentPos)
{
    double newx = FragmentPos.x+0.5;
    double newy = FragmentPos.y+0.5;
    newy = 1.0 - newy;
    
    
    unsigned int width=0,height=0;
    get_window_size(&width,&height);
    
    
    double factorx = width/scale_x;
    double factory = height/scale_y;
    
    return (XPoint){.x=factorx*newx,.y=factory*newy};
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
    
    triangle[0].x/=triangle[0].z;
    triangle[0].y/=triangle[0].z;
    triangle[0].z=1;
    triangle[1].x/=triangle[1].z;
    triangle[1].y/=triangle[1].z;
    triangle[1].z=1;
    triangle[2].x/=triangle[2].z;
    triangle[2].y/=triangle[2].z;
    triangle[2].z=1;
    
    int maxX = max(triangle[0].x, max(triangle[1].x, triangle[2].x));
    int minX = min(triangle[0].x, min(triangle[1].x, triangle[2].x));
    int maxY = max(triangle[0].y, max(triangle[1].y, triangle[2].y));
    int minY = min(triangle[0].y, min(triangle[1].y, triangle[2].y));
    
    struct Vec2 vs1 = VEC2(triangle[1].x - triangle[0].x, triangle[1].y - triangle[0].y);
    struct Vec2 vs2 = VEC2(triangle[2].x - triangle[0].x, triangle[2].y - triangle[0].y);
    
    for(double x = minX;x<=maxX;x+=0.001)
    {
        for(double y = minY;y<=maxY;y+=0.001)
        {
            struct Vec4 q = VEC4(x - triangle[0].x, y - triangle[0].y,1,1);
            
            //this is wrong...
            //check
            //https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage
            double s = division( crossProduct(q, triangle[1]) ,crossProduct(triangle[0], triangle[1]) );
            double t = division( crossProduct(triangle[0], q) , crossProduct(triangle[0], triangle[1]) );
            
            if ( (s >= 0) && (t >= 0) && (s + t <= 1))
            { /* inside triangle */
                struct Vec2 FragmentPos = VEC2(x,y);
                
                //callback
                void* iVertexOut=interpolateVertexOut(FragmentPos,triangle,vertexOut);
                
                //use globals as uniforms?
                int32_t pixel_color = fragmentShader(FragmentPos,iVertexOut);//callback
                
                XPoint pixel_coords = to_screen_coords(FragmentPos);
                XSetForeground(dis,gc,pixel_color);
                XDrawPoint(dis,win,gc,pixel_coords.x,pixel_coords.y);
            }
        }
    }
}
int main (int argc,char ** argv) 
{
    struct Vec4 points[] = { VEC4(0,0,0,1), VEC4(1,0,0,1), VEC4(0,1,0,1), VEC4(1,1,0,1), VEC4(0.5,1,0,1) };
    int num_points = 5;
    
    XEvent event;		/* the XEvent declaration !!! */
    KeySym key;		/* a dealie-bob to handle KeyPress Events */	
    char text[255];		/* a char buffer for KeyPress Events */
    
    init_x();
    
    bool draw_mouse = false;
    
    int oldx = -1;
    int oldy = -1;
    
    int32_t pointer_color = colori(0,0,0);
    
    /* look for events forever... */
    while(1) 
    {		
        /* get the next event and stuff it into our event variable.
        Note:  only events we set the mask for are detected!
        */
        XNextEvent(dis, &event);
        
        if (event.type==Expose && event.xexpose.count==0) 
        {
            /* the window was exposed redraw it! */
            redraw();
        }
        if (event.type==KeyPress&&
            XLookupString(&event.xkey,text,255,&key,0)==1) 
        {
            /* use the XLookupString routine to convert the invent
            KeyPress data into regular text.  Weird but necessary...
            */
            if (text[0]=='q') 
            {
                close_x();
            }
            else if(text[0]=='r')
            {
                pointer_color = colori_delta_red(pointer_color,1);
            }
            else if(text[0]=='R')
            {
                pointer_color = colori_delta_red(pointer_color,-1);
            }
            else if(text[0]=='g')
            {
                pointer_color = colori_delta_green(pointer_color,1);
            }
            else if(text[0]=='G')
            {
                pointer_color = colori_delta_green(pointer_color,-1);
            }
            else if(text[0]=='b')
            {
                pointer_color = colori_delta_blue(pointer_color,1);
            }
            else if(text[0]=='B')
            {
                pointer_color = colori_delta_blue(pointer_color,-1);
            }
            
            {
                int32_t red = colori_get_red(pointer_color);
                int32_t green = colori_get_green(pointer_color);
                int32_t blue = colori_get_blue(pointer_color);
                printf("R = %d G = %d B = %d\n",red,green,blue);
                
                /*XSetForeground(dis,gc,colori(255,255,255));
                snprintf(text,254,"R = %d G = %d B = %d",red,green,blue);
                XClearArea(dis,win,250,250,10,10,true);
                XDrawString(dis,win,gc,250,250, text, strlen(text));
                XSetForeground(dis,gc,pointer_color);*/
                
            }
        }
        if (event.type==ButtonPress && !draw_mouse) 
        {
            draw_mouse = true;
            oldx = event.xmotion.x;
            oldy = event.xmotion.y;
        }
        
        
        if(draw_mouse && event.type == MotionNotify)
        {
            int newx=event.xmotion.x;
            int newy=event.xmotion.y;
            
            
            if(oldx == -1) oldx = newx;
            if(oldy == -1) oldy = newy;
            
            XSetForeground(dis,gc,pointer_color);
            
            XDrawLine(dis,win,gc,oldx,oldy,newx,newy);
            
            oldx = newx;
            oldy = newy;
        }
        
        if (event.type==ButtonRelease && draw_mouse)
        {
            draw_mouse = false;
            //printf("set false\n");
        }
        
        pipeline(points,0,1,4,NULL,0,0,0,0);
        
        
        //XSetForeground(dis,gc,colori(255,255,255));
        //draw_triangle(points,0,1,4);
    }
}






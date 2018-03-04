#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>

#include "draw.h"
#include <math.h>
#include "math.h"



float mytransform[4][4] = {
    {1.0f,0.0f,0.0f,0.0f},
    {0.0f,1.0f,0.0f,0.0f},
    {0.0f,0.0f,1.0f,0.0f},
    {0.0f,0.0f,0.0f,1.0f}};

float identity[4][4] = {
    {1.0f,0.0f,0.0f,0.0f},
    {0.0f,1.0f,0.0f,0.0f},
    {0.0f,0.0f,1.0f,0.0f},
    {0.0f,0.0f,0.0f,1.0f}};

float (*transform)[4][4];

void* vertexShader(struct Vec4* vertex,void* attribute)
{
    *vertex = apply_matrix4x4(*transform,*vertex);
    return attribute;
}

struct Vec4 fragmentShader(float fragx,float fragy,struct Vec4 triangle[3],float lambda0,float lambda1, float lambda2,void * vertexOut[3])
{
    struct Vec4 color0 = *(struct Vec4*)vertexOut[0];
    struct Vec4 color1 = *(struct Vec4*)vertexOut[1];
    struct Vec4 color2 = *(struct Vec4*)vertexOut[2];
    
    struct Vec4 result = scale(lambda0,color0);
    result = add(result,scale(lambda1,color1));
    result = add(result,scale(lambda2,color2));
    
    return result;
}

int main (int argc,char ** argv) 
{
    struct Vec4 points[] = { 
        VEC4(0.5,0.5,1,1), //0
        VEC4(-0.5,0.5,1,1), //1
        VEC4(-0.5,-0.5,1,1), //2
        VEC4(0.5,-0.5,1,1), //3
        VEC4(0,0,1,1), //4
        VEC4(2,2,2,1), //5
        VEC4(-2,2,2,1), //6
        VEC4(-2,-2,2,1), //7
        VEC4(2,-2,2,1) }; //8
    int num_points = sizeof points / sizeof points[0];
    
    
    XEvent event;		/* the XEvent declaration !!! */
    KeySym key;		/* a dealie-bob to handle KeyPress Events */	
    char text[255];		/* a char buffer for KeyPress Events */
    
    init_x();
    
    bool mouse_clicked = false;
    
    int oldx = -1;
    int oldy = -1;
    
    int degrees = 0;
    
    
    double dt = 1/30.0;
    struct timespec tstart={0,0}, tend={0,0};
    
    /* look for events forever... */
    while(1) 
    {		
        /* get the next event and stuff it into our event variable.
        Note:  only events we set the mask for are detected!
        */
        
        
        clock_gettime(CLOCK_MONOTONIC, &tstart);
        
        if(dt < (1/30.0))
        {
            double diff = (1/30.0) - dt;
            tstart.tv_nsec+=(diff*1000000000);
            if(tstart.tv_nsec >= 1000000000) {
                tstart.tv_nsec -= 1000000000;
                tstart.tv_sec++;
            }
            clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&tstart,NULL);
        }
        
        XNextEvent(dis, &event);
        
        
        if (event.type==Expose && event.xexpose.count==0) 
        {
            /* the window was exposed redraw it! */
            
            
            get_window_size(&window_width_px,&window_height_px);
            deltax = 1.0f/window_width_px;
            deltay = 1.0f/window_height_px;
            
            screen_img = XGetImage(dis, win, 0,0, 
                                   window_width_px, window_height_px, AllPlanes,
                                   ZPixmap);
        }
        
        
        if (event.type==KeyPress&&
            XLookupString(&event.xkey,text,255,&key,0)==1) 
        {
            /* use the XLookupString routine to convert the invent
            KeyPress data into regular text.  Weird but necessary...
            */
            
            double deltamov = 0.1f;
            
            if (text[0]=='q') 
            {
                close_x();
            }
            else if(text[0]=='c')
            {
                degrees+=1;
                float angle = degrees*PI / 180.0f;
                float cos_angle = cos(angle);
                float sin_angle = sin(angle);
                
                mytransform[0][0]=cos_angle;
                mytransform[0][1]=-sin_angle;
                mytransform[1][0]=sin_angle;
                mytransform[1][1]=cos_angle;
            }
            else if(text[0]=='v')
            {
                degrees-=1;
                float angle = degrees*PI / 180.0f;
                float cos_angle = cos(angle);
                float sin_angle = sin(angle);
                
                mytransform[0][0]=cos_angle;
                mytransform[0][1]=-sin_angle;
                mytransform[1][0]=sin_angle;
                mytransform[1][1]=cos_angle;
            }
            else if(text[0]=='w')
            {
                mytransform[1][3]+=deltamov;
            }
            else if(text[0]=='s')
            {
                mytransform[1][3]-=deltamov;
            }
            else if(text[0]=='a')
            {
                mytransform[0][3]-=deltamov;
            }
            else if(text[0]=='d')
            {
                mytransform[0][3]+=deltamov;
            }
            else if(text[0]=='r')
            {
                mytransform[2][3]+=deltamov;
            }
            else if(text[0]=='f')
            {
                mytransform[2][3]-=deltamov;
            }
        }
        if (event.type==ButtonPress && !mouse_clicked) 
        {
            mouse_clicked = true;
            oldx = event.xmotion.x;
            oldy = event.xmotion.y;
        }
        
        
        if(event.type == MotionNotify)
        {
            int newx=event.xmotion.x;
            int newy=event.xmotion.y;
            
            newx = clamp(newx,0,window_width_px);
            newy = clamp(newy,0,window_height_px);
            
            oldx = newx;
            oldy = newy;
        }
        
        if (event.type==ButtonRelease && mouse_clicked)
        {
            mouse_clicked = false;
        }
        
        struct Vec4 colors[] = {VEC4(1,0,0,1),VEC4(0,1,0,1),VEC4(0,0,1,1),VEC4(1,1,1,1)};
        //white background
        transform=&identity;
        pipeline(points,5,6,7,colors,sizeof(typeof(colors[0])),3,3,3);
        pipeline(points,5,7,8,colors,sizeof(typeof(colors[0])),3,3,3);
        
        //colorful triangle
        transform=&mytransform;
        pipeline(points,0,1,2,colors,sizeof(typeof(colors[0])),0,1,2);
        
        XPutImage(dis, win, gc, screen_img, 
                  0, 0, 0, 0, 
                  window_width_px, window_height_px);
        
        clock_gettime(CLOCK_MONOTONIC, &tend);
        
        dt =((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
            ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
    }
}






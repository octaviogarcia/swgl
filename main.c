#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "draw.h"
#include <math.h>
#include "math.h"




float transform[4][4] = {
    {1.0f,0.0f,0.0f,0.0f},
    {0.0f,1.0f,0.0f,0.0f},
    {0.0f,0.0f,1.0f,0.0f},
    {0.0f,0.0f,0.0f,1.0f}};


void* vertexShader(struct Vec4* vertex,void* attribute)
{
    *vertex = apply_matrix(transform,*vertex);
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
    struct Vec4 points[] = { VEC4(0.5,0.5,1,1), VEC4(-0.5,0.5,1,1), VEC4(-0.5,-0.5,1,1), VEC4(0.5,-0.5,1,1), VEC4(0,0,1,1) };
    int num_points = 5;
    
    
    XEvent event;		/* the XEvent declaration !!! */
    KeySym key;		/* a dealie-bob to handle KeyPress Events */	
    char text[255];		/* a char buffer for KeyPress Events */
    
    init_x();
    
    bool draw_mouse = false;
    
    int oldx = -1;
    int oldy = -1;
    
    Color pointer_color = colori(0,0,0);
    
    int degrees = 0;
    
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
            
            get_window_size(&window_width_px,&window_height_px);
            deltax = 1.0f/window_width_px;
            deltay = 1.0f/window_height_px;
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
            else if(text[0]=='d')
            {
                struct Vec4 colors[] = {VEC4(1,0,0,1),VEC4(0,1,0,1),VEC4(0,0,1,1)};
                pipeline(points,0,1,2,colors,sizeof(typeof(colors[0])),0,1,2);
            }
            else if(text[0]=='c')
            {
                degrees+=1;
                float angle = degrees*PI / 180.0f;
                float cos_angle = cos(angle);
                float sin_angle = sin(angle);
                
                transform[0][0]=cos_angle;
                transform[0][1]=-sin_angle;
                transform[1][0]=sin_angle;
                transform[1][1]=cos_angle;
            }
            else if(text[0]=='C')
            {
                redraw();
            }
            
            {
                int32_t red = pointer_color.rgb.r;
                int32_t green = pointer_color.rgb.g;
                int32_t blue = pointer_color.rgb.b;
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
            
            XSetForeground(dis,gc,pointer_color.integer);
            
            XDrawLine(dis,win,gc,oldx,oldy,newx,newy);
            
            oldx = newx;
            oldy = newy;
        }
        
        if (event.type==ButtonRelease && draw_mouse)
        {
            draw_mouse = false;
        }
    }
}






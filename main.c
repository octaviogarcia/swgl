#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "draw.h"


//TODO: fix weird color interpolation issue

void* vertexShader(struct Vec4* vertex,void* attribute)
{
    return attribute;
}

int32_t fragmentShader(float fragx,float fragy,struct Vec4 triangle[3],float lambda0,float lambda1, float lambda2,void * vertexOut[3])
{
    int32_t * p_color0 = vertexOut[0];
    int32_t * p_color1 = vertexOut[1];
    int32_t * p_color2 = vertexOut[2];
    
    int32_t color0 = *p_color0;
    int32_t color1 = *p_color1;
    int32_t color2 = *p_color2;
    
    return lambda0*color0+lambda1*color1+lambda2*color2;;
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
                int32_t colors[] = {colori(255,0,0),colori(0,255,0),colori(0,0,255)};
                
                pipeline(points,0,1,2,colors,sizeof(int32_t),0,1,2);
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
        }
    }
}






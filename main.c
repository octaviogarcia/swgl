#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "draw.h"


int main (int argc,char ** argv) 
{
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
    }
}






#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>

#include "draw.h"
#include "math.h"


_Atomic bool close_program = false;
_Atomic double dt = 1/30.0;
pthread_mutex_t img_data_lock = PTHREAD_MUTEX_INITIALIZER;

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
    
    struct Vec4 result;
    
    {
        struct Vec4 scaled0 = scale(lambda0,color0);
        struct Vec4 scaled1 = scale(lambda1,color1);
        struct Vec4 scaled2 = scale(lambda2,color2);
        result = add(scaled0,add(scaled1,scaled2));
    }
    
    return result;
}
void* draw_thread(void* usr_info)
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
    int oldx = -1;
    int oldy = -1;
    int degrees = 0;
    
    struct timespec tstart={0,0}, tend={0,0};
    while(!close_program) 
    {		
        clock_gettime(CLOCK_MONOTONIC, &tstart);
        
        struct Vec4 colors[] = {VEC4(1,0,0,0.5f),VEC4(0,1,0,0.5f),VEC4(0,0,1,0.5f),VEC4(0.5f,0.5f,1,1)};
        
        
        pthread_mutex_lock(&img_data_lock);
        
        clear_depth_buffer();
        clear_image();
        
        
        //background
        transform=&identity;
        pipeline(points,5,6,7,colors,sizeof(typeof(colors[0])),3,3,3);
        pipeline(points,5,7,8,colors,sizeof(typeof(colors[0])),3,3,3);
        
        //colorful triangle
        transform=&mytransform;
        pipeline(points,0,1,2,colors,sizeof(typeof(colors[0])),0,1,2);
        
        pthread_mutex_unlock(&img_data_lock);
        
        XLockDisplay(dis);
        
        XPutImage(dis, win, gc, screen_img, 
                  0, 0, 0, 0, 
                  screen_img->width, screen_img->height);
        
        XUnlockDisplay(dis);
        
        clock_gettime(CLOCK_MONOTONIC, &tend);
        
        dt =((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
            ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
    }
    return NULL;
}

void* logger_thread(void* usr_info)
{
    while(!close_program)
    {
        usleep(1000);
        printf("FPS %f\r",1.0/dt);
    }
    return NULL;
}

int keys[1024] = {0};

int main (int argc,char ** argv) 
{
    XEvent event;		/* the XEvent declaration !!! */	
    char text[255];		/* a char buffer for KeyPress Events */
    
    if (pthread_mutex_init(&img_data_lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
    
    
    XInitThreads();
    init_x();
    
    bool mouse_clicked = false;
    
    
    pthread_t draw_thread_descriptor;
    bool draw_inited = false;
    
    pthread_t logger_thread_descriptor;
    bool logger_inited = false;
    
    
    while(1) 
    {		
        /* get the next event and stuff it into our event variable.
        Note:  only events we set the mask for are detected!
        */
        
        XNextEvent(dis, &event);
        
        
        if (event.type==Expose && event.xexpose.count==0) 
        {
            /*window exposed, get new parameters */
            
            
            pthread_mutex_lock(&img_data_lock);
            
            int width=0,height=0;
            get_window_size(&width,&height);
            UpdateScreenData(width,height);
            
            pthread_mutex_unlock(&img_data_lock);
        }
        
        
        if(!draw_inited)
        {
            draw_inited = true;
            int err = pthread_create(&draw_thread_descriptor, NULL,draw_thread, NULL);
            
            if(err)
            {
                printf("Could initialize draw thread, err %d",err);
                exit(1);
            }
        }
        
        if(!logger_inited)
        {
            logger_inited = true;
            int err = pthread_create(&logger_thread_descriptor, NULL,logger_thread, NULL);
            if(err)
            {
                printf("Could initialize draw thread, err %d",err);
                exit(1);
            }
        }
        
        
        
        if (event.type==KeyPress)  
        {
            KeySym keysym = XLookupKeysym(&event.xkey, 0);
            
            double deltamov = 0.1f;
            
            if (keysym == XK_F1) 
            {
                close_program = true;
                pthread_join(draw_thread_descriptor,NULL);
                pthread_join(logger_thread_descriptor,NULL);
                pthread_mutex_destroy(&img_data_lock);
                close_x();
            }
            else if(keysym == XK_c)
            {
                /*
                degrees+=1;
                float angle = degrees*PI / 180.0f;
                float cos_angle = cos(angle);
                float sin_angle = sin(angle);
                
                mytransform[0][0]=cos_angle;
                mytransform[0][1]=-sin_angle;
                mytransform[1][0]=sin_angle;
                mytransform[1][1]=cos_angle;
                */
            }
            else if(keysym == XK_v)
            {
                /*
                degrees-=1;
                float angle = degrees*PI / 180.0f;
                float cos_angle = cos(angle);
                float sin_angle = sin(angle);
                
                mytransform[0][0]=cos_angle;
                mytransform[0][1]=-sin_angle;
                mytransform[1][0]=sin_angle;
                mytransform[1][1]=cos_angle;
                */
            }
            else if(text[0]==XK_w)
            {
                //mytransform[1][3]+=deltamov;
            }
            else if(text[0]==XK_s)
            {
                //mytransform[1][3]-=deltamov;
            }
            else if(text[0]==XK_a)
            {
                //mytransform[0][3]-=deltamov;
            }
            else if(text[0]==XK_d)
            {
                //mytransform[0][3]+=deltamov;
            }
            else if(text[0]==XK_r)
            {
                //mytransform[2][3]+=deltamov;
            }
            else if(text[0]==XK_f)
            {
                //mytransform[2][3]-=deltamov;
            }
        }
        
        if (event.type==ButtonPress && !mouse_clicked) 
        {
            /*
            mouse_clicked = true;
            oldx = event.xmotion.x;
            oldy = event.xmotion.y;*/
        }
        
        
        if(event.type == MotionNotify)
        {
            /*
            int newx=event.xmotion.x;
            int newy=event.xmotion.y;
            
            newx = clamp(newx,0,window_width_px);
            newy = clamp(newy,0,window_height_px);
            
            oldx = newx;
            oldy = newy;*/
        }
        
        if (event.type==ButtonRelease && mouse_clicked)
        {
            //mouse_clicked = false;
        }
        
    }
}






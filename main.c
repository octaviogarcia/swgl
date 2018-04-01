#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <tgmath.h>
#include <pthread.h>
#include <stdatomic.h>

#include "draw.h"
#include "math.h"
//TODO: proper projection, with FOV and everythig
//normal transformation
//https://learnopengl.com/Lighting/Basic-Lighting
//doesnt work -_-
//probably inverse is wrong as af

_Atomic bool close_program = false;
_Atomic double dt = 1/30.0;
pthread_mutex_t img_data_lock = PTHREAD_MUTEX_INITIALIZER;



float perspective[4][4] = {
    {1.0f,0.0f,0.0f,0.0f},
    {0.0f,1.0f,0.0f,0.0f},
    {0.0f,0.0f,1.0f,0.0f},
    {0.0f,0.0f,1.0f,0.0f}};

void setPerspective(float fov,float aspect,float near,float far)
{
    
    float f =  1.0f/tan((PI / 180.0f)*fov/2.0f);
    perspective[0][0]=f/aspect;
    perspective[1][1]=f;
    //perspective[2][2]=far/(far-near);
    //perspective[2][3]=1.0f;
    //perspective[3][2]=-near*far/(far-near);//??
    
    return;
}


float mytransform[4][4] = {
    {1.0f,0.0f,0.0f,0.0f},
    {0.0f,1.0f,0.0f,0.0f},
    {0.0f,0.0f,1.0f,0.0f},
    {0.0f,0.0f,0.0f,0.0f}};

float normal_mytransform[4][4] = {
    {1.0f,0.0f,0.0f,0.0f},
    {0.0f,1.0f,0.0f,0.0f},
    {0.0f,0.0f,1.0f,0.0f},
    {0.0f,0.0f,0.0f,0.0f}};

//set w to z so it gets converted to screenspace when dividing by it
float identity[4][4] = {
    {1.0f,0.0f,0.0f,0.0f},
    {0.0f,1.0f,0.0f,0.0f},
    {0.0f,0.0f,1.0f,0.0f},
    {0.0f,0.0f,1.0f,0.0f}};

float (*transform)[4][4];



void* vertexShader(Vec4 triangle[3],void* attributes[3],int index)
{
    
    triangle[index] = apply_matrix4x4(perspective,apply_matrix4x4(*transform,triangle[index]));
    Vec4 normal = ((Vec4*)attributes[index])[1];
    if(index==0)//might not be paralelizable lol
    {
        float inverted[4][4];
        inverse(mytransform,inverted);
        transpose(inverted,normal_mytransform);
        
        normal_mytransform[3][0]=0;
        normal_mytransform[3][1]=0;
        normal_mytransform[3][2]=0;
        
        normal_mytransform[0][3]=0;
        normal_mytransform[1][3]=0;
        normal_mytransform[2][3]=0;
        
        normal_mytransform[3][3]=0;
    }
    //((Vec4*)attributes[index])[1] =apply_matrix4x4(normal_mytransform,normal);
    return attributes[index];
}

Vec4 LightPos = VEC4(0.75f,0.75f,0.15f,1);
Vec4 LightColor = VEC4(1,1,1,1);
float LightStrength = 3.0f;
float ambient = 0.05f;

Vec4 fragmentShader(float fragx,float fragy,float fragz, Vec4 triangle[3],float lambda0,float lambda1, float lambda2,void * vertexOut[3])
{
    Vec4 normal = *((Vec4*)vertexOut[0]+1);
    
    /*
    {
    Vec4 to1 = VEC4(triangle[1].x-triangle[0].x,
    triangle[1].y-triangle[0].y,
    triangle[1].z-triangle[0].z,
    1);
    
    Vec4 to2 = VEC4(triangle[2].x-triangle[0].x,
    triangle[2].y-triangle[0].y,
    triangle[2].z-triangle[0].z,
    1);
    
    normal = crossProduct(to2,to1);
    }*/
    
    normal =  normalizeV3(normal);
    
    Vec4 lightDir;
    lightDir.x = LightPos.x - fragx;
    lightDir.y = LightPos.y - fragy;
    lightDir.z = LightPos.z - fragz;
    lightDir.w = 1;
    
    float diff = maxf(dotProductV3(normal,lightDir),0.0f);
    Vec4 diffused = scale(diff*LightStrength,LightColor);
    
    Vec4 result;
    
#if 0
    Vec4 color0 = *( Vec4*)vertexOut[0];
    Vec4 color1 = *( Vec4*)vertexOut[1];
    Vec4 color2 = *( Vec4*)vertexOut[2];
    
    {
        Vec4 scaled0 = scale(lambda0,color0);
        Vec4 scaled1 = scale(lambda1,color1);
        Vec4 scaled2 = scale(lambda2,color2);
        result = add(scaled0,add(scaled1,scaled2));
    }
#else
    result = VEC4(0.5f,0.5f,0.5f,1);
#endif
    result = scale(ambient,result);
    result.w/=ambient;
    
    
    
    return VEC4(
        result.x*diffused.x,
        result.y*diffused.y,
        result.z*diffused.z,
        result.w);
}


void* draw_thread(void* usr_info)
{
    
    Vec4 points[] = { 
        VEC4(2,2,2,1), //0
        VEC4(-2,2,2,1), //1
        VEC4(-2,-2,2,1), //2
        VEC4(2,-2,2,1), //3
        VEC4(0.25f,0.25f,0.5f,1),//4
        VEC4(-0.25f,0.25f,0.5f,1),//5
        VEC4(-0.25f,-0.25f,0.5f,1),//6
        VEC4(0.25f,-0.25f,0.5f,1),//7
        VEC4(0.25f,0.25f,0.75f,1),//8
        VEC4(-0.25f,0.25f,0.75f,1),//9
        VEC4(-0.25f,-0.25f,0.75f,1),//10
        VEC4(0.25f,-0.25f,0.75f,1)//11
    }; 
    
    Vec4 attr[] = {
        //color, normal
        VEC4(1,0,0,1),VEC4(0,0,-1,1),
        VEC4(0,1,0,1),VEC4(0,0,-1,1),
        VEC4(0,0,1,1),VEC4(0,0,-1,1)};
    
    int num_points = sizeof points / sizeof points[0];
    int oldx = -1;
    int oldy = -1;
    int degrees = 0;
    
    struct timespec tstart={0,0}, tend={0,0};
    while(!close_program) 
    {		
        clock_gettime(CLOCK_MONOTONIC, &tstart);
        
        
        pthread_mutex_lock(&img_data_lock);
        
        clear_all_buffers();
        
        float aspect = screen_img->height;
        aspect/=screen_img->width;
        
        setPerspective(90,aspect,0.1f,10.0f);
        
        
        
        //colorful cube
        transform=&mytransform;
        
        //Front
        pipeline(points,4,5,6,attr,sizeof(typeof(attr[0]))*2,0,1,2);
        pipeline(points,4,7,6,attr,sizeof(typeof(attr[0]))*2,0,1,2);
        
        /*
        //Back
        pipeline(points,8,9,10,colors,sizeof(typeof(colors[0])),0,1,2);
        pipeline(points,8,10,11,colors,sizeof(typeof(colors[0])),0,1,2);
        
        //Top
        pipeline(points,4,8,5,colors,sizeof(typeof(colors[0])),0,1,2);
        pipeline(points,9,8,5,colors,sizeof(typeof(colors[0])),0,1,2);
        
        //Bottom
        pipeline(points,7,11,6,colors,sizeof(typeof(colors[0])),0,1,2);
        pipeline(points,10,11,6,colors,sizeof(typeof(colors[0])),0,1,2);
        
        //Left
        pipeline(points,5,9,10,colors,sizeof(typeof(colors[0])),0,1,2);
        pipeline(points,6,5,10,colors,sizeof(typeof(colors[0])),0,1,2);
        
        //Right
        pipeline(points,4,8,11,colors,sizeof(typeof(colors[0])),0,1,2);
        pipeline(points,7,4,11,colors,sizeof(typeof(colors[0])),0,1,2);
        */
        
        
        /*
        //background
        transform=&identity;
        pipeline(points,0,1,2,colors,sizeof(typeof(colors[0])),3,3,3);
        pipeline(points,0,2,3,colors,sizeof(typeof(colors[0])),3,3,3);
        */
        
        
        
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
    
    int degrees1=0,degrees2=0;
    
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
            
            unsigned int width=0,height=0;
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
            KeyCode keycode = event.xkey.keycode;
            
            static double deltamov = 0.1f;
            static float deltaangle = PI / 180.0f;
            
            float cos_angle = cos(deltaangle);
            float sin_angle = sin(deltaangle);
            
            float anticlockwise[4][4] = 
            {
                {cos_angle,-sin_angle,0.0f,0.0f},
                {sin_angle, cos_angle,0.0f,0.0f},
                {0.0f,0.0f,1.0f,0.0f},
                {0.0f,0.0f,0.0f,1.0f}
            };
            
            float clockwise[4][4] = 
            {
                {cos_angle,sin_angle,0.0f,0.0f},
                {-sin_angle, cos_angle,0.0f,0.0f},
                {0.0f,0.0f,1.0f,0.0f},
                {0.0f,0.0f,0.0f,1.0f}
            };
            
            float zanticlockwise[4][4] = 
            {
                {cos_angle,0.0f,-sin_angle,0.0f},
                {0.0f     ,1.0f,      0.0f,0.0f},
                {sin_angle,0.0f,cos_angle ,0.0f},
                {0.0f     ,0.0f,      0.0f,1.0f}
            };
            
            
            float zclockwise[4][4] = 
            {
                {cos_angle,0.0f,sin_angle,0.0f},
                {0.0f     ,1.0f,      0.0f,0.0f},
                {-sin_angle,0.0f,cos_angle ,0.0f},
                {0.0f     ,0.0f,      0.0f,1.0f}
            };
            
            
            pthread_mutex_lock(&img_data_lock);
            if (keycode == XKeysymToKeycode(dis, XK_F1)) 
            {
                
                close_program = true;
                pthread_mutex_unlock(&img_data_lock);
                pthread_join(draw_thread_descriptor,NULL);
                pthread_join(logger_thread_descriptor,NULL);
                pthread_mutex_destroy(&img_data_lock);
                
                if(screen_img)
                    XDestroyImage(screen_img);
                screen_img = NULL;
                if(depth_buffer)
                    free(depth_buffer);
                depth_buffer=NULL;
                close_x();
            }
            else if(keycode ==  XKeysymToKeycode(dis,XK_c))
            {
                matrixProduct(anticlockwise,mytransform);
            }
            else if(keycode ==  XKeysymToKeycode(dis,XK_v))
            {
                matrixProduct(clockwise,mytransform);
            }
            else if(keycode ==  XKeysymToKeycode(dis,XK_b))
            {
                matrixProduct(zanticlockwise,mytransform);
            }
            else if(keycode ==  XKeysymToKeycode(dis,XK_n))
            {
                matrixProduct(zclockwise,mytransform);
            }
            else if(keycode== XKeysymToKeycode(dis,XK_w))
            {
                mytransform[1][3]+=deltamov;
            }
            else if(keycode== XKeysymToKeycode(dis,XK_s))
            {
                mytransform[1][3]-=deltamov;
            }
            else if(keycode== XKeysymToKeycode(dis,XK_a))
            {
                mytransform[0][3]-=deltamov;
            }
            else if(keycode== XKeysymToKeycode(dis,XK_d))
            {
                mytransform[0][3]+=deltamov;
            }
            else if(keycode== XKeysymToKeycode(dis,XK_r))
            {
                mytransform[2][3]+=deltamov;
            }
            else if(keycode== XKeysymToKeycode(dis,XK_f))
            {
                mytransform[2][3]-=deltamov;
            }
            pthread_mutex_unlock(&img_data_lock);
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






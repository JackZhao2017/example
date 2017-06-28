#include "devices/device_camera.h"
#include "mxc_v4l2.h"
#include <string.h>
#include <linux/videodev2.h>
#define TRUE  1
#define FALSE 0

unsigned char        *g_framebuf=NULL;

int g_cap_width      =640;
int g_cap_height     =480;
int g_cap_fmt        =V4L2_PIX_FMT_UYVY;


int main()
{
	int len=0,i=0;
	FILE *fp=NULL;
	if(capture_init(g_cap_width,g_cap_height,g_cap_fmt)<0)
    {
         printf("<%s>:camera capture initialize failed \n",__func__);
    } 
    g_framebuf=(unsigned char *)calloc(1,g_cap_width*g_cap_height*2);
    capture_getframedata(g_framebuf,&len);
    memset(g_framebuf,0,g_cap_width*g_cap_height*2);
    for(i=0;i<5;i++){
    	char buf[128];
    	memset(buf,0,sizeof(buf));
    	capture_getframedata(g_framebuf,&len);
    	sprintf(buf,"test%d.uyvy",i);
    	fp=fopen(buf,"wb");
    	fwrite(g_framebuf,1,g_cap_width*g_cap_height*2,fp);
    	fclose(fp);
    }
	capture_release();
    free(g_framebuf);
}
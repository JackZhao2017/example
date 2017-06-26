#include "device_imagout.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include <sys/time.h>
#include "mxcfb.h"
#include "mxc_v4l2.h"
#include "ipu.h"


#define TFAIL               -1
#define TPASS               0
#define TRUE                1
#define FALSE               0
#define TEST_BUFFER_NUM     3

char g_v4l_output[100]="/dev/video17";
int  g_fd_v4louput=-1;
char *g_fb_dev="/dev/fb0";
uint32_t g_out_fmt = V4L2_PIX_FMT_UYVY;

static int g_frame_size;
static int g_num_buffers;
static int g_overlay = 0;
static int g_mem_type = V4L2_MEMORY_MMAP;

static int g_get_width=640;
static int g_get_height=480;

static int g_display_width = 640;
static int g_display_height = 480;
static int g_display_top = 0;
static int g_display_left = 0;

static int g_vflip = 0;
static int g_hflip = 0;
static int g_vdi_enable = 0;
static int g_vdi_motion = 0;
static int g_icrop_w = 0;
static int g_icrop_h = 0;
static int g_icrop_top = 0;
static int g_icrop_left = 0;
static int g_out_rotate = 0;
static int g_frame_period = 33333;
static int g_loop_count = 1;

struct testbuffer
{
        unsigned char *start;
        size_t offset;
        unsigned int length;
};
struct testbuffer out_buffers[TEST_BUFFER_NUM];


static void print_pixelformat(char *prefix, int val)
{
	printf("%s: %c%c%c%c\n", prefix ? prefix : "pixelformat",
					val & 0xff,
					(val >> 8) & 0xff,
					(val >> 16) & 0xff,
					(val >> 24) & 0xff);
}



static void fb_setup(void)
{
    struct mxcfb_gbl_alpha alpha;
	int fd;
    struct fb_fix_screeninfo fb_fg_fix;

	if((fd = open(g_fb_dev,O_RDWR,0))<0)
	{
		printf("<%s>:unable open %s \n",__func__,g_fb_dev);
	}

    alpha.alpha  = 0;
    alpha.enable = 1;

    if (ioctl(fd, MXCFB_SET_GBL_ALPHA, &alpha) < 0) {
		      printf("<%s>:set alpha %d failed for fb0\n",__func__,alpha.alpha);
    }
    if (ioctl(fd, FBIOGET_FSCREENINFO, &fb_fg_fix) < 0) {
            printf("<%s>:Get fix of fb2 failed\n",__func__);
            close(fd);
    }
    printf("<%s>:fb_fg_fix.id :%s \n",__func__,fb_fg_fix.id);
	close(fd);
}

static int  v4l_ouput_setup(struct v4l2_format *fmt)
{
        struct v4l2_requestbuffers buf_req;
        int i=0;
        struct v4l2_buffer buf;
        if (ioctl(g_fd_v4louput, VIDIOC_S_FMT, fmt) < 0)
        {
                printf("<%s>:set format failed\n",__func__);
                return TFAIL;
        }

        if (ioctl(g_fd_v4louput, VIDIOC_G_FMT, fmt) < 0)
        {
                printf("<%s>:get format failed\n",__func__);
                return TFAIL;
        }
        char fourcc[5]={0};
        strncpy(fourcc, (char *)&fmt->fmt.pix.pixelformat, 4);
        printf("<%s>:fmt:\n fmt.fmt.pix.width=%d"
                       "fmt.fmt.pix.height=%d "
                       "fmt.fmt.pix.pixelformat =%s\n",__func__,fmt->fmt.pix.width,fmt->fmt.pix.height,fourcc);
        memset(&buf_req, 0, sizeof(buf_req));
        buf_req.count =TEST_BUFFER_NUM;
        buf_req.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        buf_req.memory = g_mem_type;
        if (ioctl(g_fd_v4louput, VIDIOC_REQBUFS, &buf_req) < 0)
        {
                printf("<%s>:request buffers failed\n",__func__);
                return TFAIL;
        }
        g_num_buffers = buf_req.count;
        printf("<%s>:v4l2_output test: Allocated %d buffers\n",__func__, buf_req.count);

        for (i = 0; i < g_num_buffers; i++) {

			memset(&buf, 0, sizeof (buf));
			buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
			buf.memory = g_mem_type;
			buf.index = i;
			if (ioctl(g_fd_v4louput, VIDIOC_QUERYBUF, &buf) < 0)
			{
				printf("<%s>:VIDIOC_QUERYBUF error\n",__func__);
			}

			out_buffers[i].length = buf.length;
			out_buffers[i].offset = (size_t) buf.m.offset;
			out_buffers[i].start = mmap (NULL, out_buffers[i].length,
					PROT_READ | PROT_WRITE, MAP_SHARED,
					g_fd_v4louput, out_buffers[i].offset);
			printf("<%s>:VIDIOC_QUERYBUF: addr: %p length = %d, offset = %d\n",__func__,out_buffers[i].start,
				out_buffers[i].length, out_buffers[i].offset);
			if (out_buffers[i].start == NULL) {
				printf("<%s>:v4l2_out test: mmap failed\n",__func__);
			}
		}

        return TPASS;
}

int  v4l_output_init(CAPTURE_IMAGOUT_INFO *info)
{
	struct v4l2_capability cap;
	struct v4l2_fmtdesc fmtdesc;
	struct v4l2_cropcap cropcap;
	struct v4l2_framebuffer fb;
	struct v4l2_crop crop;
	struct v4l2_rect icrop;
	struct v4l2_control ctrl;
	struct v4l2_format fmt;
	int    retval;
    if(info!=NULL){
        g_out_fmt      =info->fmt;
        g_get_width    =info->in_width;
        g_get_height   =info->in_height;
        g_display_width=info->display_width;
        g_display_height=info->display_height;
    }
	if((g_fd_v4louput=open(g_v4l_output,O_RDWR,0))<0)
	{
		printf("<%s>:unable open %s\n",__func__,g_v4l_output);
		return -1;
	}
	if (!ioctl(g_fd_v4louput, VIDIOC_QUERYCAP, &cap)) {
		printf("<%s>: %s :driver=%s, card=%s, bus=%s, "
				"version=0x%08x, "
				"capabilities=0x%08x\n",__func__,g_v4l_output,
				cap.driver, cap.card, cap.bus_info,
				cap.version,
				cap.capabilities);
	}

	fmtdesc.index = 0;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	while (!ioctl(g_fd_v4louput, VIDIOC_ENUM_FMT, &fmtdesc)) {
        char fourcc[5]={0};
        strncpy(fourcc, (char *)&fmtdesc.pixelformat, 4);
		printf("<%s>:fmt %s: fourcc = %s\n",__func__,
				fmtdesc.description,
				fourcc);
		fmtdesc.index++;
	}

	memset(&cropcap, 0, sizeof(cropcap));
    cropcap.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    if (ioctl(g_fd_v4louput, VIDIOC_CROPCAP, &cropcap) < 0)
    {
            printf("<%s>:get crop capability failed\n",__func__);
            retval = TFAIL;
            goto err;
    }
    printf("<%s>:cropcap.bounds.width = %d  cropcap.bound.height = %d\n" \
           "<%s>:cropcap.defrect.width = %d cropcap.defrect.height = %d\n",__func__,
           cropcap.bounds.width, cropcap.bounds.height,__func__,
           cropcap.defrect.width, cropcap.defrect.height);

        crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        crop.c.top = g_display_top;
        crop.c.left = g_display_left;
        crop.c.width = g_display_width;
        crop.c.height = g_display_height;

        if (ioctl(g_fd_v4louput, VIDIOC_S_CROP, &crop) < 0)
        {
                printf("<%s>:set crop failed\n",__func__);
                retval = TFAIL;
                goto err;
        }

        // Set rotation
        ctrl.id = V4L2_CID_ROTATE;
        ctrl.value = g_out_rotate;
        if (ioctl(g_fd_v4louput, VIDIOC_S_CTRL, &ctrl) < 0)
        {
                printf("<%s>:set ctrl rotate failed\n",__func__);
                retval = TFAIL;
                goto err;
        }
        ctrl.id = V4L2_CID_VFLIP;
        ctrl.value = g_vflip;
        if (ioctl(g_fd_v4louput, VIDIOC_S_CTRL, &ctrl) < 0)
        {
                printf("<%s>:set ctrl vflip failed\n",__func__);
                retval = TFAIL;
                goto err;
        }
        ctrl.id = V4L2_CID_HFLIP;
        ctrl.value = g_hflip;
        if (ioctl(g_fd_v4louput, VIDIOC_S_CTRL, &ctrl) < 0)
        {
                printf("<%s>:set ctrl hflip failed\n",__func__);
                retval = TFAIL;
                goto err;
        }
		if (g_vdi_enable) {
			ctrl.id = V4L2_CID_MXC_MOTION;
			ctrl.value = g_vdi_motion;
			if (ioctl(g_fd_v4louput, VIDIOC_S_CTRL, &ctrl) < 0)
			{
			printf("<%s>:set ctrl motion failed\n",__func__);
			retval = TFAIL;
			goto err;
			}
		}



		fb.capability = V4L2_FBUF_CAP_EXTERNOVERLAY;
        if (g_overlay)
                fb.flags = V4L2_FBUF_FLAG_OVERLAY;
        else
                fb.flags = V4L2_FBUF_FLAG_PRIMARY;

        ioctl(g_fd_v4louput, VIDIOC_S_FBUF, &fb);

        memset(&fmt, 0, sizeof(fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        fmt.fmt.pix.width=g_get_width;
        fmt.fmt.pix.height=g_get_height;
        fmt.fmt.pix.pixelformat = g_out_fmt;
	if (g_vdi_enable)
		fmt.fmt.pix.field = V4L2_FIELD_INTERLACED_TB;
	if (g_icrop_w && g_icrop_h) {
		icrop.left = g_icrop_left;
		icrop.top = g_icrop_top;
		icrop.width = g_icrop_w;
		icrop.height = g_icrop_h;
		fmt.fmt.pix.priv = (unsigned int)&icrop;
        } else
		fmt.fmt.pix.priv = 0;

	retval = v4l_ouput_setup(&fmt);
    printf("<%s>:init success \n",__func__);
    return retval;     
err:
    return -1;
}

int v4l_output_process(unsigned char * img,int len)
{
	struct v4l2_buffer buf;
	static int i=0; //dengerous!!!
	int count = 100;
	int streamon_cnt = 0;
	enum v4l2_buf_type type;
	int retval=0;
	if (g_vdi_enable)
		streamon_cnt = 1;
	memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.memory = g_mem_type;
   		if (i < g_num_buffers) {
       		 buf.index = i;
        	if (ioctl(g_fd_v4louput, VIDIOC_QUERYBUF, &buf) < 0)
        	{
                printf("<%s>: VIDIOC_QUERYBUF failed\n",__func__);
				retval = -1;
        	}

    	}
    	else {
            buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
            buf.memory = g_mem_type;
            if (ioctl(g_fd_v4louput, VIDIOC_DQBUF, &buf) < 0)
            {
                printf("<%s>:   %xVIDIOC_DQBUF failed\n",__func__,img);
				retval = -1;
            }
    	}
    	memcpy(out_buffers[buf.index].start,img,len);
        if (g_vdi_enable)
			 buf.field = V4L2_FIELD_INTERLACED_TB;
     	if ((retval = ioctl(g_fd_v4louput, VIDIOC_QBUF, &buf)) < 0)
     	{
            printf("<%s>:VIDIOC_QBUF failed %d\n",__func__, retval);
			retval = -1;
     	}

    	if ( i == streamon_cnt ) { // Start playback after buffers queued
            type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
            if (ioctl (g_fd_v4louput, VIDIOC_STREAMON, &type) < 0) {
                   printf("<%s>:Could not start stream\n",__func__);
				   retval = -1;
            }
            fb_setup();
    	}
    	i++;
    return retval;
}


void v4l_out_release(void )
{
	int i=0,ret;
	enum v4l2_buf_type type;   
    type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ioctl (g_fd_v4louput, VIDIOC_STREAMOFF, &type);

	for (i = 0; i < g_num_buffers; i++) {
		munmap (out_buffers[i].start, out_buffers[i].length);
	}
	close(g_fd_v4louput);
	printf("<%s>:v4l_out_release \n",__func__);
}
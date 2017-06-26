#include "device_camera.h"

#include <errno.h>

/* Verification Test Environment Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include "mxc_v4l2.h"
#define TEST_BUFFER_NUM 3


struct testbuffer buffers[TEST_BUFFER_NUM];


int g_top = 0;
int g_left = 0;
int g_input = 0;
int g_rotate = 0;

int g_camera_framerate = 30;
int g_extra_pixel = 0;
int g_capture_mode = 0;



int fd_v4l=-1;
char g_v4l_device[100] = "/dev/video0";

static long long currenttime(){
    struct timeval now;
    gettimeofday(&now, NULL);
    long long when = now.tv_sec * 1000LL + now.tv_usec / 1000;
    return when;
}
static int start_capture();

int capture_getframedata(unsigned char *framebuf,int *len)
{
        struct v4l2_buffer buf;
        long long startTime =0 ,endTime;
        memset(&buf, 0, sizeof (buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        startTime=currenttime();
        if (ioctl (fd_v4l, VIDIOC_DQBUF, &buf) < 0) {
                 printf("<%s>: VIDIOC_DQBUF failed.\n",__func__);
                 printf("<%s>:buf.index:%d buffers.offset :%d buf.m.offset:%d \n",__func__,
                            buf.index,buffers[buf.index].offset,buf.m.offset);
                 return -1;
        }
        endTime=currenttime();      
        memcpy(framebuf,buffers[buf.index].start,buffers[buf.index].length);
        *len=buffers[buf.index].length; 
                
        if (ioctl (fd_v4l, VIDIOC_QBUF, &buf) < 0) {
                 printf("<%s>:VIDIOC_QBUF failed\n",__func__);
                 return -1;
        }
        if((endTime-startTime))
        printf("<%s>:  DQbuf time  %d  \n",__func__, endTime-startTime);
        return 0;
}
static void print_pixelformat(char *prefix, int val)
{
	printf("%s: %c%c%c%c\n", prefix ? prefix : "pixelformat",
					val & 0xff,
					(val >> 8) & 0xff,
					(val >> 16) & 0xff,
					(val >> 24) & 0xff);
}
static int start_capture()
{
		struct v4l2_buffer buf;
		struct v4l2_format fmt;

		enum v4l2_buf_type type;
		unsigned int i;
        int stcnt=0;
		struct v4l2_requestbuffers req;
    	memset(&req, 0, sizeof (req));
    	req.count = TEST_BUFFER_NUM;
    	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	req.memory = V4L2_MEMORY_MMAP;

    	if (ioctl(fd_v4l, VIDIOC_REQBUFS, &req) < 0)
    	{
            printf("<%s>:VIDIOC_REQBUFS failed  fd =%d \n",__func__,fd_v4l);
            return -1;
    	}


	 	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(fd_v4l, VIDIOC_G_FMT, &fmt) < 0)
        {
                printf("<%s>:get format failed\n",__func__);
        }
        else
        {
                char fourcc[5] = {0};
                strncpy(fourcc, (char *)&fmt.fmt.pix.pixelformat, 4);
                printf("<%s>: Width = %d\n",__func__, fmt.fmt.pix.width);
                printf("<%s>: Height = %d \n",__func__, fmt.fmt.pix.height);
                printf("<%s>: Image size = %d\n",__func__, fmt.fmt.pix.sizeimage);
                printf("<%s>:pixelformat = %s\n",__func__,fourcc);
        }
start:
        for (i = 0; i < TEST_BUFFER_NUM; i++)
        {

                memset(&buf, 0, sizeof (buf));
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
				buf.index = i;

                if (ioctl(fd_v4l, VIDIOC_QUERYBUF, &buf) < 0)
                {
                        printf("<%s>:VIDIOC_QUERYBUF error\n",__func__);
                        return -1;
                }

                buffers[i].length = buf.length;
                buffers[i].offset = (size_t) buf.m.offset;
                buffers[i].start = mmap (NULL, buffers[i].length,PROT_READ | PROT_WRITE, 
                				MAP_SHARED,fd_v4l, buffers[i].offset);
                printf("<%s>:Length: %d Address: %p \n",__func__, 
                			buf.length, buffers[i].start);
				memset(buffers[i].start, 0xFF, buffers[i].length);
        }

        for (i = 0; i < TEST_BUFFER_NUM; i++)
        {
                memset(&buf, 0, sizeof (buf));
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;
				buf.m.offset = buffers[i].offset;
				// if (g_extra_pixel){
	   //              buf.m.offset += g_extra_pixel *
	   //              	(out_width + 2 * g_extra_pixel) + g_extra_pixel;
				// }
                if (ioctl (fd_v4l, VIDIOC_QBUF, &buf) < 0) {
                        printf("<%s>:VIDIOC_QBUF error\n",__func__);
                        return -1;
                }
                printf("<%s>:buf.index:%d buffers.offset :%d buf.m.offset:%d \n",__func__,
                            buf.index,buffers[i].offset,buf.m.offset);
        }

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl (fd_v4l, VIDIOC_STREAMON, &type) < 0) {
                printf("<%s>:VIDIOC_STREAMON error\n",__func__);
                return -1;
        }
        if(stcnt==2)
                return -1;
        if(stcnt++<2)
        {
            memset(&buf, 0, sizeof (buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            if (ioctl (fd_v4l, VIDIOC_DQBUF, &buf) < 0) {
                 printf("<%s>: VIDIOC_DQBUF failed.\n",__func__);
                 printf("<%s>:buf.index:%d  buf.m.offset:%d \n",__func__,
                            buf.index,buf.m.offset);
                goto cleanup;
            }
            if (ioctl (fd_v4l, VIDIOC_QBUF, &buf) < 0) {
                 printf("<%s>:VIDIOC_QBUF failed\n",__func__);
            }
        }

        return 0;
cleanup:
        for (i = 0; i < TEST_BUFFER_NUM; i++)
        {
            if (-1 == munmap (buffers[i].start, buffers[i].length))      
            printf("<%s>:munmap error",__func__); 
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl (fd_v4l, VIDIOC_STREAMOFF, &type) < 0) {
                printf("<%s>:VIDIOC_STREAMOFF error\n",__func__);
                return -1;
        }
        goto start;
}

int capture_init(int out_width,int out_height,int cap_fmt)
{
	struct v4l2_format fmt;
    struct v4l2_control ctrl;
    struct v4l2_streamparm parm;
	struct v4l2_crop crop;
	struct v4l2_mxc_offset off;
	struct v4l2_dbg_chip_ident chip;
	struct v4l2_frmsizeenum fsize;
	struct v4l2_fmtdesc ffmt;
    if ((fd_v4l = open(g_v4l_device, O_RDWR, 0)) < 0)
   	{
                printf("<%s>:Unable to open %s\n",__func__, g_v4l_device);
                return -1;
    }
    printf("<%s>:open %s fd_v4l =%d \n",__func__, g_v4l_device,fd_v4l);


	printf("<%s>:sensor supported frame size:\n",__func__);
	fsize.index = 0;
	while (ioctl(fd_v4l, VIDIOC_ENUM_FRAMESIZES, &fsize) >= 0) {
		printf("<%s>:%dx%d\n",__func__, fsize.discrete.width,
					       fsize.discrete.height);
		fsize.index++;
	}
    
	ffmt.index = 0;
	while (ioctl(fd_v4l, VIDIOC_ENUM_FMT, &ffmt) >= 0) {
		print_pixelformat("<capture_init>:sensor frame format",ffmt.pixelformat);
		ffmt.index++;
	}
	
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = g_camera_framerate;
	parm.parm.capture.capturemode = g_capture_mode;

	if (ioctl(fd_v4l, VIDIOC_S_PARM, &parm) < 0)
	{
	        printf("<%s>:VIDIOC_S_PARM failed\n",__func__);
	        return -1;
	}

	// if (ioctl(fd_v4l, VIDIOC_S_INPUT, &g_input) < 0)
	// {
	// 	printf("<%s>:VIDIOC_S_INPUT failed\n",__func__);
	// 	return -1;
	// }

	// crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	// if (ioctl(fd_v4l, VIDIOC_G_CROP, &crop) < 0)
	// {
	// 	printf("<%s>:VIDIOC_G_CROP failed\n",__func__);
	// 	return -1;
	// }
 //    printf("<%s>:VIDIOC_G_CROP width  %d\n",__func__,crop.c.width);
 //    printf("<%s>:VIDIOC_G_CROP height %d\n",__func__,crop.c.height);
 //    printf("<%s>:VIDIOC_G_CROP top    %d\n",__func__,crop.c.top);
 //    printf("<%s>:VIDIOC_G_CROP left   %d\n",__func__,crop.c.left);

	// crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	// crop.c.width = in_width;
	// crop.c.height = in_height;
	// crop.c.top = g_top;
	// crop.c.left = g_left;
    
 //    printf("<%s>:VIDIOC_S_CROP width  %d\n",__func__,crop.c.width);
 //    printf("<%s>:VIDIOC_S_CROP height %d\n",__func__,crop.c.height);
 //    printf("<%s>:VIDIOC_S_CROP top    %d\n",__func__,crop.c.top);
 //    printf("<%s>:VIDIOC_S_CROP left   %d\n",__func__,crop.c.left);
	// if (ioctl(fd_v4l, VIDIOC_S_CROP, &crop) < 0)
	// {
	// 	printf("<%s>:VIDIOC_S_CROP failed\n",__func__);
	// 	return -1;
	// }
    memset(&fmt,0,sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = cap_fmt;
    fmt.fmt.pix.width = out_width;
    fmt.fmt.pix.height = out_height;
 //    fmt.fmt.pix.field = V4L2_FIELD_NONE;
 //    if (g_extra_pixel){
	// 		off.u_offset = (2 * g_extra_pixel + out_width) * (out_height + g_extra_pixel)
	// 			 - g_extra_pixel + (g_extra_pixel / 2) * ((out_width / 2)
	// 			 + g_extra_pixel) + g_extra_pixel / 2;
	// 		off.v_offset = off.u_offset + (g_extra_pixel + out_width / 2) *
	// 			((out_height / 2) + g_extra_pixel);
 //        	fmt.fmt.pix.bytesperline = out_width + g_extra_pixel * 2;
	// 		fmt.fmt.pix.priv = (uint32_t) &off;
 //        	fmt.fmt.pix.sizeimage = (out_width + g_extra_pixel * 2 )
 //        		* (out_height + g_extra_pixel * 2) * 3 / 2;
	// } else {
	//         fmt.fmt.pix.bytesperline = out_width;
	// 		fmt.fmt.pix.priv = 0;
 //        	fmt.fmt.pix.sizeimage = 0;
	// }

	if (ioctl(fd_v4l, VIDIOC_S_FMT, &fmt) < 0)
    {
                printf("<%s>:set format failed\n",__func__);
                return -1;
    }

    ctrl.id = V4L2_CID_PRIVATE_BASE + 0;
    ctrl.value = g_rotate;
    if (ioctl(fd_v4l, VIDIOC_S_CTRL, &ctrl) < 0)
    {
                printf("<%s>:set ctrl failed\n",__func__);
                return -1;
    }

    if(start_capture()<0)
    {
        return -1;
    }


    return 0;
}


static int stop_capturing()
{
        enum v4l2_buf_type type;
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        return ioctl (fd_v4l, VIDIOC_STREAMOFF, &type);	
}
void capture_release()
{
		int i=0;
		struct v4l2_requestbuffers req;
	   	if (stop_capturing(fd_v4l) < 0)
        {
            printf("<%s>:stop_capturing failed\n",__func__);
        }
        for(i=0;i<TEST_BUFFER_NUM;i++){
			if (-1 == munmap (buffers[i].start, buffers[i].length))      
			printf("<%s>:munmap error",__func__); 
		}
        if(fd_v4l>0)
		    close(fd_v4l);
        printf("<%s>:capture_release success \n",__func__);
}
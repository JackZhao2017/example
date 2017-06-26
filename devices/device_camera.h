
#ifndef _DEVICE_CAMERA_H_
#define _DEVICE_CAMERA_H_



struct testbuffer
{
        unsigned char *start;
        size_t offset;
        unsigned int length;
};

int  capture_getframedata(unsigned char *buf,int *len);
int  capture_init(int cap_width,int cap_height,int cap_fmt);
void capture_release();

#endif
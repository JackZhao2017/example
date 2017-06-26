#ifndef _DEVICE_IMAGOUT_H_
#define _DEVICE_IMAGOUT_H_


typedef struct {
	int in_width;
	int in_height;
	int display_width;
	int display_height;
	int fmt;
}CAPTURE_IMAGOUT_INFO;

int  v4l_output_init(CAPTURE_IMAGOUT_INFO *info);
int  v4l_output_process(unsigned char * img,int len);
void v4l_out_release(void);

#endif


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/select.h>

#include "message/message_core.h"
#include "message/message_ringbuffer.h"
#include "message/message_cmdqueue.h"
#include "devices/device_uart.h"



#define TRUE  1
#define FALSE 0
#define TXBUFSIZE 256

static pthread_t p_meassgeSend,p_messgeRead;
static sem_t     g_sem_read,g_sem_send;
static RINGBUFFER g_ringbufInfo;
static int g_uart=-1;
static unsigned char g_writebuf=NULL;
static int g_iswritefinished=0,g_writelen=0;

static void *messageRead(void *arg)
{
	unsigned char  data[RINGBUFSIZE];
	int ,len=0,isSync=0,retval=0;
	int i =0;
	struct timeval tv;
	fd_set r_fds;
	while(g_rrun) { 
		FD_ZERO(&r_fds);
		FD_SET(g_uart,&r_fds);
		tv.tv_sec = 1;
		tv.tv_usec = 0;	
		iores = select(g_uart + 1,&r_fds,NULL,NULL,&tv);
		switch(iores){
			case -1:
				printf("-----------------------------select\n");
 				break;
 			case 0:
  				printf("-----------------------------time out\n");
				break;
			default:
				device_uartRead(data ,&len ,g_uart);
				putdatatoBuffer(&g_ringbufInfo,data,len);
				putdatato
				break;
		}

		if(!isSync)
        		isSync=detectSync(&g_ringbufInfo,SYN_SIGN);
        if(isSync){
        	if(detectMsginfo(&g_ringbufInfo,&len)){
        		if(len){
        			getdatafromBuffer(&g_ringbufInfo,data,len);
        			printf("\n--------------------len  %d\n",len);
        			for(i=0;i<len;i++)
        		   		printf("0x%x ",data[i]);
        		   	printf("\n---------------------------\n");
        			retval=message_resolver(data);
        			if(retval<0)
        				printf("%s resolver faild \n",__func__);
        			isSync=0;
        		}
        	}else{
        			isSync=0;
        	}
        }	 	
	}
	pthread_exit(NULL);

}
static void *messageWrite(void *arg)
{
	while(g_trun)
	{
		sem_wait(&g_sem_send);
		g_isTxfinished=TRUE;
		write(g_uart,g_writebuf,g_writelen);
		g_isTxfinished=FALSE;
	}
	pthread_exit(NULL);
}

int app_messageRead(void *buf,int len, int cmd)
{
	return 0;
}
int app_messageWrite(char *buf ,int len)
{
	int i=0;
	memset(g_writebuf,0,TXBUFSIZE);
	memcpy(g_writebuf,buf,len);
	g_writelen=len;
	sem_post(&g_sem_send);
	return 0;
}
int app_messageInit()
{
	if((g_uart=device_uartInit())<0){
		return -1;
	}
	ret = pthread_create(&p_messageRead, NULL, messageRead, NULL);
	if(ret<0)
	{
		printf("failed to create uart read thread \n");
		return -1;
	}
	ret = pthread_create(&p_meassgeSend, NULL, messageRead, NULL);
	if(ret<0)
	{
		printf("failed to create uart send thread \n");
		return -1;
	}
	sem_init(&g_sem_read,0,0);
	sem_init(&g_sem_send,0,0);
	g_writebuf=malloc(TXBUFSIZE);
	memset(&g_ringbufInfo,0,sizeof(g_ringbufInfo));
	ringbufferInit(&g_ringbufInfo,RINGBUFSIZE);
	return 0;
}
void app_messageRelease()
{
	int ret;
	g_rrun = 0;
	sem_post(&g_sem_read);
	ret = pthread_join(p_messageRead, NULL);
	if(ret < 0)
		printf("fail to stop read thread\n");
	g_trun = 0;
	sem_post(&g_sem_send);
	ret = pthread_join(p_meassgeSend, NULL);
	if(ret < 0)
		printf("fail to stop send thread\n");

}
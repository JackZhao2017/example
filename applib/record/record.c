#include "record.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "Encoder.h"
//#define printf  
char g_timer[128];

//recorder variable
char g_record_path[128];
EncoderHandle Encoder_handler=NULL;
FILE *g_frecord=NULL;
FILE *g_fmessage=NULL;
int g_encoderTime=3*60*1000;
int g_Encoder_count=0;

int g_Encoder_width=640;
int g_Encoder_height=480;
int g_Encoder_fmt=V4L2_PIX_FMT_UYVY;
//end

static long long currenttime(){
    struct timeval now;
    gettimeofday(&now, NULL);
    long long when = now.tv_sec * 1000LL + now.tv_usec / 1000;
    return when;
}
void gettimer(char *t)
{
	long long time = currenttime();
	int sec=time/1000%60;
	int minu=time/1000/60%60;
	int hour=time/1000/60/60%24;
	sprintf(t,"%02d%02d%02d",hour,minu,sec);
	memset(g_timer,0,sizeof(g_timer));
	memcpy(g_timer,t,strlen(t));
}



void getEncoderName(char *fn)
{
  char timer[56];
  memset(timer,0,sizeof(timer));
  gettimer(timer);
  sprintf(fn,"%s/Encoder%s.h264",g_record_path,timer);
  printf("%s\n",fn);
}

void recorderParam(const char *path ,int time,int w ,int h, int fmt){
	memset(g_record_path,0,sizeof(g_record_path));
	strcpy(g_record_path,path);
	if(time>0)
	   g_encoderTime=time;

	g_Encoder_width=w;
	g_Encoder_height=h;
	g_Encoder_fmt=fmt;
}


void recordmsgInit()
{
    char name[128];
    memset(name,0,sizeof(name));
    sprintf(name,"%s/message%s.txt",g_record_path,g_timer);
    g_fmessage=fopen(name,"w");
}
int recordmsgWrite(BUFINFO info)
{
    int i=0;
    fprintf(g_fmessage, "len :%d\n",info.len);
    for(i=0;i<info.len;i++)
    fprintf(g_fmessage, "0x%02x  ",info.addr[i]);
    fprintf(g_fmessage, "\n");
    return 0;
}

FILE * recorderInit(const char *path)
{
	FILE *fd=NULL;
	char name_path[128]={0};			
	memset(name_path,0,sizeof(name_path));
	sprintf(name_path,"%s/Encoder%s.bin",path,g_timer);
	if((fd=fopen(name_path,"wb"))==NULL){
		printf("%s open %s\n",__func__,name_path);
	}
	
	printf("<%s> config size %d ldw_fcw_info size %d\n",__func__,sizeof(ADAS_CONFIG),sizeof(LDW_FCW_INFO));
	return fd;
}
int recorderWriteConfig(ADAS_CONFIG adasconfig,FILE *fd){
	int ret=0;
	ret=fwrite(&adasconfig,1,sizeof(ADAS_CONFIG),fd);
	if(ret<sizeof(ADAS_CONFIG))
		return -1;
	return 0;
}
int recorderWriteInfo(LDW_FCW_INFO ldwfcwinfo,FILE *fd)
{
	int ret=0;
	ret=fwrite(&ldwfcwinfo,1,sizeof(LDW_FCW_INFO),fd);
	if(ret<sizeof(LDW_FCW_INFO))
		return -1;
	return 0;
}

void recorderRelease(FILE *fd)
{
	fclose(fd);
}





FILE * openRecordbin(const char *path)
{
	
	FILE * fd=NULL;
	if((fd=fopen(path,"rb"))==NULL){
		printf("%s open %s\n",__func__,path);
	}
	int size=sizeof(LDW_FCW_INFO);
	printf("%d\n",size);
	return fd;
}

int readConfig(FILE *fd ,ADAS_CONFIG *config)
{
	fread(config,1,sizeof(ADAS_CONFIG),fd);
}

int readrecordInfo(FILE *fd ,LDW_FCW_INFO *info)
{
	int retval=0;
	retval=fread(info,1,sizeof(LDW_FCW_INFO),fd);
	if(retval<sizeof(LDW_FCW_INFO)){
		return -1;
	}
	return 0;
}

void releaseRecord(FILE *fd){
	fclose(fd);	
}

int isRestartEncode=0;



int  process_record(unsigned char *buf,unsigned int len,RECORD_INFO recorder,BUFINFO messageInfo)
{
   
    int isRecord=0,ret=0,retval=0;
    static  long long old=0;
    long long now=0;
    FILE *fp=NULL;
    if(!isRestartEncode){
          char fn[128];           
          memset(fn,0,sizeof(fn));
          getEncoderName(fn);  
          Encoder_handler=Encoder_open("hardh264v2",fn,g_Encoder_width,g_Encoder_height,10,g_Encoder_fmt);
          if(Encoder_handler==NULL){
             printf("%s Encoder_handler creat failed\n",__func__);
             return -1;
          }
          g_frecord=recorderInit(g_record_path);
          if(g_frecord==NULL){
             printf("%s recorderInit faild\n",__func__);
             return -1;
          }
          recorderWriteConfig(recorder.config,g_frecord);
          recordmsgInit();
          old=currenttime();
          isRestartEncode=1;
    }
    if(isRestartEncode){

          ret=Encoder_write(Encoder_handler, buf, len);
          if(ret<0){
             printf("%s encoder h264 write  faild\n",__func__);
             isRecord=1;
             retval=-1;
          }

          ret =recorderWriteInfo(recorder.info,g_frecord);
          if(ret<0){
             isRecord=1;
             retval=-1;
             printf("%s encoder  bin write  faild\n",__func__);
          }
          recordmsgWrite(messageInfo);
          now=currenttime();
          ++g_Encoder_count;
          if(now-old>g_encoderTime){
             isRecord=1;
          }
    }
    if(isRecord){
          printf("%s every %d minute write %d frame\n",__func__,g_encoderTime/60/1000, g_Encoder_count);
          g_Encoder_count=0;
          recorderRelease(g_frecord);
          Encoder_close(Encoder_handler);
          close(g_fmessage);
          isRestartEncode=0;
    }
    return retval;
}
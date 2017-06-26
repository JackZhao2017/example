#ifndef _CAMERA_RECORD_H_
#define _CAMERA_RECORD_H_

#include "adaslib/ADAS_HEAD.h"
#include <stdio.h>
#include <stdlib.h>


typedef struct {
	LDW_INFO    ldwInfo;
	LDW_RESULT	ldwResult;
	FCW_INFO    fcwInfo;
	FCW_RESULT  fcwResult;
	float 		speed;
}LDW_FCW_INFO;

typedef struct {
	LDW_FCW_INFO info;
	ADAS_CONFIG  config;
}RECORD_INFO;

#define RECORD_NUM  600 
#define RECORD_TIME  3*60*1000

void recorderParam(const char *path ,int time,int w ,int h, int fmt);
void gettimer(char *t);
void getrecordmsgpath(char *t,char *n);

FILE * recorderInit(const char *path);
void recorderRelease(FILE *fd);
int recorderWriteConfig(ADAS_CONFIG adasconfig,FILE *fd);
int recorderWriteInfo(LDW_FCW_INFO ldwfcwinfo ,FILE *fd);


FILE * openRecordbin(const char *path);
int readConfig(FILE *fd ,ADAS_CONFIG *config);
int readrecordInfo(FILE *fd ,LDW_FCW_INFO *info);
void releaseRecord(FILE *fd);


int  process_record(unsigned char *buf,unsigned int len,RECORD_INFO recorder);
#endif
#include "device_timer.h"
#include <semaphore.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUM_MEMBER  4
#define FALSE 		 	0
#define TRUE 		 	1


typedef struct {
	char name[128];
	int  id;
	char finished;
	char freq;
	char freqcnt;
	char count;
	void *func;
	sem_t sem;
}TIMER_MEMBER;

typedef struct 
{	
	TIMER_MEMBER *member[MAX_NUM_MEMBER];
	float basefreq;
	int num;
	int isinit;
}TIMER_INFO;

static TIMER_MEMBER memberpool[MAX_NUM_MEMBER];
static TIMER_INFO g_timerinfo;
static struct itimerval g_timevalue;  


int unregister_timer(int id){
	if(g_timerinfo.member[id-1]==NULL||!g_timerinfo.isinit){
		printf("%d unable unregister \n",id);
		return -1;
	}
	--g_timerinfo.num;
	sem_destroy(&g_timerinfo.member[id-1]->sem);
	memset(g_timerinfo.member[id-1],0,sizeof(TIMER_MEMBER));
	g_timerinfo.member[id-1]=NULL;
	if(g_timerinfo.member[id-1]==NULL)
		printf("%d unregister success\n",id);
}
int register_timer(char *name,int  hz)
{
	int i=0;
	if(!g_timerinfo.isinit){
		capture_timeInit(TIMER_BASE_HZ);
	}
	for(i=0;i<MAX_NUM_MEMBER;i++){
		if(g_timerinfo.member[i])
			continue;
		g_timerinfo.member[i]=&memberpool[i];
		g_timerinfo.member[i]->id=i+1;
		g_timerinfo.member[i]->freq=hz;
		g_timerinfo.member[i]->freqcnt=g_timerinfo.basefreq/hz;
		sem_init(&g_timerinfo.member[i]->sem,0,0);
		memcpy(g_timerinfo.member[i]->name,name,strlen(name));
		++g_timerinfo.num;
		return g_timerinfo.member[i]->id;
	}
	return -1; 
}
void wait_timersignal(int id)
{
	g_timerinfo.member[id-1]->finished=TRUE;
	sem_wait(&g_timerinfo.member[id-1]->sem);
	g_timerinfo.member[id-1]->finished=FALSE;
}

static void capture_signal(int signo)
{	
	int i=0;
	if(signo==SIGALRM){
		for(i=0;i<MAX_NUM_MEMBER;i++){
			if(g_timerinfo.member[i]==NULL)
				continue;
			++g_timerinfo.member[i]->count;
			if(g_timerinfo.member[i]->count==g_timerinfo.member[i]->freqcnt){
				g_timerinfo.member[i]->count=0;
				if(g_timerinfo.member[i]->finished){
				    sem_post(&g_timerinfo.member[i]->sem);				   
				}else{
					printf("<%s> timer member %s lose a time count frequency %d\n",__func__,g_timerinfo.member[i]->name,g_timerinfo.member[i]->freq);
				}
			}
		}
	}		
}

int capture_timeInit(int mfps)
{
    if(mfps<1)
		return -1;
	memset(&g_timerinfo,0,sizeof(g_timerinfo));
	memset(&g_timevalue,0,sizeof(g_timevalue));
    printf("<%s>:process id is %d\n",__func__, getpid());		
    signal(SIGALRM, capture_signal);
    g_timevalue.it_value.tv_sec = 0;
    g_timevalue.it_value.tv_usec = 1000000/mfps;
    g_timevalue.it_interval.tv_sec = 0;
    g_timevalue.it_interval.tv_usec = 1000000/mfps;
    setitimer(ITIMER_REAL, &g_timevalue, NULL);     //(2)
    printf("<%s>:time interrupt frame :%d \n",__func__,g_timevalue.it_value.tv_usec);
    g_timerinfo.num=0;
	g_timerinfo.basefreq=mfps;
	g_timerinfo.isinit=TRUE;
	return 0;
}

int capture_timeRelease()
{
	int ret=0;
	memset(&g_timevalue,0,sizeof(g_timevalue));
	setitimer(ITIMER_REAL, &g_timevalue, NULL);
	signal(SIGALRM, NULL);
	printf("<%s>:capture_timerelease success \n",__func__);	
	return 0;
}

	











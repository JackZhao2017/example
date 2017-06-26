#ifndef _DEVIDE_TIMER_H_
#define _DEVICE_TIMER_H_




#define TIMER_1_HZ 	 1
#define TIMER_5_HZ   5
#define TIMER_10_HZ  10
#define TIMER_15_HZ	 15
#define TIMER_30_HZ  30
#define TIMER_BASE_HZ TIMER_30_HZ

int capture_timeInit(int mfps);
int register_timer(char *name ,int hz);
void wait_timersignal(int id);
int capture_timeRelease();

#endif





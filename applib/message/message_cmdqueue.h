#ifndef _MESSAGE_CMDQUEUE_H_
#define _MESSAGE_CMDQUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "message_core.h"

void putcmdintoQueue(SYS_CTRLINFO ctrlinfo);
void getcmdfromQueue(SYS_CTRLINFO *ctrlinfo);	
int  iscmdneedProcess(void);

#ifdef __cplusplus
};
#endif

#endif
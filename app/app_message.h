#ifndef _APP_MESSAGE_H_
#define _APP_MESSAGE_H_

int  app_messageWrite(char *buf ,int len);
int  app_messageRead(void *buf,int len,int cmd);
int  app_messageInit(void);
void app_messageRelease(void);

#endif
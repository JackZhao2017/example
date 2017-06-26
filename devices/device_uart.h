#ifndef _MESSAGE_UART_H_
#define _MESSAGE_UART_H_

int  device_uartInit(void);
int  device_uartWrite(char *buf, int len ,int fd);
int  device_uartRead(char *buf,int *len ,int fd);
void device_uartRelease(void);

#endif
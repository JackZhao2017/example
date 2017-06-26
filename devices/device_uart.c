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

#define TRUE  1
#define FALSE 0
#define DEFAULT_RATE 115200
char 		*g_uartDev="/dev/ttymxc1";

static speed_t baudrate_map(unsigned long b)
{
    speed_t retval;

    switch(b)
    {

        case 9600:
            retval = B9600;
            break;

        case 19200:
            retval = B19200;
            break;

        case 38400:
            retval = B38400;
            break;

        case 57600:
            retval = B57600;
            break;

        case 115200:
            retval = B115200;
            break;
    }
    return(retval);
}

int device_uartRead(char *buf ,int *len ,int fd)
{
	iores = ioctl(fd, FIONREAD, &iocount);
	if(iocount){
        iores = read(fd, buf, iocount);
	}
	*len =iores;
	return 0;
}

int device_uartWrite(char *buf ,int len ,int fd)
{
	write(fd,buf,len);
	return 0;
}

int device_uartInit()
{
	struct termios options;
	unsigned long baudrate = DEFAULT_RATE;
	int fd = open(g_uartDev, O_RDWR | O_NOCTTY);
	if (g_fd_uart == -1) {
		printf("open_port: Unable to open serial port - %s", g_uartDev);
		return -1;
	}
	fcntl(g_fd_uart, F_SETFL, 0);
	tcgetattr(g_fd_uart, &options);

	options.c_cflag &= ~CSTOPB;

	options.c_cflag &= ~CSIZE;
	
	options.c_cflag &= ~PARODD;
	options.c_cflag |= CS8;
	options.c_cflag &= ~CRTSCTS;

	options.c_lflag &= ~(ICANON | IEXTEN | ISIG | ECHO| ECHONL);
	options.c_oflag &= ~OPOST;
	options.c_iflag &= ~(ICRNL | INPCK | ISTRIP | IXON | BRKINT );

	options.c_cc[VMIN] = 1;
	options.c_cc[VTIME] = 0;
	options.c_cflag |= (CLOCAL | CREAD);



	if(baudrate) {
		cfsetispeed(&options, baudrate_map(baudrate));
		cfsetospeed(&options, baudrate_map(baudrate));
	}

	tcsetattr(g_fd_uart, TCSANOW, &options);
	printf("UART %lu, %dbit, %dstop, %s, HW flow %s\n", baudrate, 8,
	       (options.c_cflag & CSTOPB) ? 2 : 1,
	       (options.c_cflag & PARODD) ? "PARODD" : "PARENB",
	       (options.c_cflag & CRTSCTS) ? "enabled" : "disabled");
	printf("uart initilizate successful \n");
	return fd;
}

void device_uartRelease(int fd)
{	
	close(fd);
	printf("%s successful\n",__func__ );
}


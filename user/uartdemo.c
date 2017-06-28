/*
	create by jack
*/

#include "app/app_message.h"


int main(int argc ,char **argv)
{
	app_messageInit();
	while(1){
		sleep(1);
	}
	app_messageRelease();
}



#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <unistd.h> 
#include <string.h>
#include <functional>
#include "zenarp.h"
#include <thread>
int g_flag=0;

unsigned char sourcemac[6];
unsigned char sourceip[4];

char local_mac[20];
char local_ip[20] = {'\0'};
char local_netmask[20] = {'\0'};
char local_gateway[20] = {'\0'};


int main()
{
	int err;
	pthread_t CatcharpID;
	pthread_t SendarpID;

	setLog_init();   
    setLog_display((LogLev)LevDispTRACE);
    setLog_local("log/ip_log",1000, LevDispTRACE);
    auto func1=std::bind(Sendarp);
    auto func2=std::bind(Catcharp);
    GetNetInfo();
//	while(1)
    {
    //    err=pthread_create(&SendarpID,NULL, func1,NULL);
        std::thread(func1).detach();
//        if(err != 0){
//			Log0("sendarp fail");
//			return 0;
//		}
           std::thread(func2).detach();
//		err= pthread_create(&CatcharpID,NULL, (void *)Catcharp,NULL);
//		if(err != 0){
//			Log0("catcharp fail");
//			return 0;
//		}
		//printf("LINE: %d FUN: %s\n", __LINE__, __FUNCTION__);
    //	pthread_join(SendarpID,NULL);
//		pthread_join(CatcharpID,NULL);
	}

    while(1)
    {
        MSLEEP(1);
    }
	return 1;
}

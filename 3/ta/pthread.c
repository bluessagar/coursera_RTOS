#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>

#include <syslog.h> // to print the sys logs

#define TH_COUNT  128      // we need to create 128 threads

// pThread structure
typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[TH_COUNT];        // 1 each thread
threadParams_t threadParams[TH_COUNT];  // 1 each thread



void *eachThread(void *threadp)
{
    int i=0;
    int gSum = 0;

    threadParams_t* threadParams = (threadParams_t *)threadp;

    for(i=0; i<threadParams->threadIdx; i++)
    {
        gsum=gsum-i;
    }

    // syslog message 
    syslog(LOG_INFO, "[COURSE:1][ASSIGNMENT:3]: Thread idx=%d, sum[1...%d] =%d\n", threadParams->threadIdx, threadParams->threadIdx, gsum);

}


int main (int argc, char *argv[])
{
   int i=0;

   for(i=0;i<TH_COUNT;i++)
   {
        threadParams[i].threadIdx=i;

        pthread_create(&threads[i],   // pointer to thread descriptor
                (void *)0,     // use default attributes
                eachThread, // thread function entry point
                (void *)&(threadParams[i]) // parameters to pass in
                );

   }
   
    // Join all the threds 
	for (i = 0; i < TH_COUNT; i++)
	{
		pthread_join(threads[i], NULL);
	}


   printf("Assignment Complete. \n");

   closelog();
}

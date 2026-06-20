#define _GNU_SOURCE     // enable GNU specific extensions
#include <pthread.h>    // POSIX threads
#include <stdio.h>      // for printf
#include <stdlib.h>     // for exit
#include <errno.h>      // 
#include <sys/time.h>   // to get system time 
#include <sys/types.h>  
#include <sys/utsname.h>
#include <syslog.h>
#include <sched.h>      // this is importatnt header file to be able to set the real time scheduling and CPU affinity
#include <unistd.h>


#define NUM_THREADS 64      // total 64 threads

#define COURSE  1
#define ASSIGNMENT  3

// thread parameter structure
typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];
pthread_t mainthread;

pthread_attr_t orig_sched_attr;
struct sched_param fifo_param;

#define SCHED_POLICY SCHED_FIFO
#define MAX_ITERATIONS (1000000)


void print_scheduler(void)
{
    int schedType = sched_getscheduler(getpid());

    switch(schedType)
    {
        case SCHED_FIFO:
            printf("Pthread policy is SCHED_FIFO\n");
            break;
        case SCHED_OTHER:
            printf("Pthread policy is SCHED_OTHER\n");
            break;
        case SCHED_RR:
            printf("Pthread policy is SCHED_RR\n");
            break;
        default:
            printf("Pthread policy is UNKNOWN\n");
    }
}

void *counterThread(void *threadp)
{
    int sum=0, i, iterations;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    double start=0.0, stop=0.0;
    struct timeval startTime, stopTime;

    gettimeofday(&startTime, 0);
    start = ((startTime.tv_sec * 1000000.0) + startTime.tv_usec)/1000000.0;


    for(iterations=0; iterations < MAX_ITERATIONS; iterations++)
    {
        sum=0;
        for(i=1; i < (threadParams->threadIdx)+1; i++)
            sum=sum+i;
    }


    gettimeofday(&stopTime, 0);
    stop = ((stopTime.tv_sec * 1000000.0) + stopTime.tv_usec)/1000000.0;

    printf("\nThread idx=%d, sum[0...%d]=%d, running on CPU=%d, start=%lf, stop=%lf", 
           threadParams->threadIdx,
           threadParams->threadIdx, sum, sched_getcpu(),
           start, stop);
}

int main (int argc, char *argv[])
{
    struct utsname uname_str;
    uname(&uname_str);

    // print the header in the syslog as per course requirement
    syslog(LOG_INFO, "[COURSE:%d][ASSIGNMENT:%d] %s %s %s %s %s GNU/Linux", 
    COURSE, ASSIGNMENT, 
    uname_str.sysname, 
    uname_str.nodename,
    uname_str.release,
    uname_str.version,
    uname_str.machine
    );

    // print current scheduler priority
    printf("INITIAL ");
    print_scheduler();

    // set the schedular policy to FIFO
    pthread_attr_t fifo_sched_attr;
    pthread_attr_init(&fifo_sched_attr);
    pthread_attr_setinheritsched(&fifo_sched_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&fifo_sched_attr, SCHED_POLICY);

    // set the cpu core to be used to 4
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    int cpuidx= 0 ;
    CPU_SET(cpuidx,&cpuset);

    int max_prio=sched_get_priority_max(SCHED_POLICY);
    fifo_param.sched_priority=max_prio;    

    if(sched_setscheduler(getpid(), SCHED_POLICY, &fifo_param) < 0)
    {
        perror("sched_setscheduler");
        return 1;
    }

    // store the desired priority in attribute object, so that threads created with this attribute gets that priority
    pthread_attr_setschedparam(&fifo_sched_attr, &fifo_param);

    // print adjusted priority
    printf("ADJUSTED "); 
    print_scheduler();

 

   // get affinity set for main thread
   mainthread = pthread_self();

   // Check the affinity mask assigned to the thread 
   int rc = pthread_getaffinity_np(mainthread, sizeof(cpu_set_t), &cpuset);
   if (rc != 0)
       perror("pthread_getaffinity_np");
   else
   {
       printf("main thread running on CPU=%d, CPUs =", sched_getcpu());

       for (int j = 0; j < CPU_SETSIZE; j++)
           if (CPU_ISSET(j, &cpuset))
               printf(" %d", j);

       printf("\n");
   }

    // create all the threads here
    for(int i=0; i < NUM_THREADS; i++)
   {
       threadParams[i].threadIdx=i;

       pthread_create(&threads[i],   // pointer to thread descriptor
                      &fifo_sched_attr,     // use FIFO RT max priority attributes
                      counterThread, // thread function entry point
                      (void *)&(threadParams[i]) // parameters to pass in
                     );

   }

   for(int i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);


   printf("\nTEST COMPLETE\n");
}

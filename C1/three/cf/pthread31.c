#define _GNU_SOURCE
#include <pthread.h>        // For thread functions
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sched.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <syslog.h>         // To add the system logs

#define NUM_THREADS 64      // Total number of threads

// Thread structure
typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];     // array to store the thread handles
pthread_t mainthread;               // handle to main thread
pthread_t startthread;              // handle to starter thread
threadParams_t threadParams[NUM_THREADS];   // array of thread paramters

pthread_attr_t fifo_sched_attr;     // attribute structure for change the scheduler
pthread_attr_t orig_sched_attr;     // attribute structure to read the current scheduler method
struct sched_param fifo_param;  

#define SCHED_POLICY SCHED_FIFO     // expected shceduler policy
#define MAX_ITERATIONS (1000000)

// function to print the scheduler policy
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

// Function to set FIFO scheduler policy
void set_scheduler(void)
{
    int max_prio, scope, rc, cpuidx;
    cpu_set_t cpuset;

    printf("INITIAL "); print_scheduler();

    // set the scheduler attribute structure 
    pthread_attr_init(&fifo_sched_attr);
    pthread_attr_setinheritsched(&fifo_sched_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&fifo_sched_attr, SCHED_POLICY);

    // set the cpu affiniety
    CPU_ZERO(&cpuset);
    cpuidx=(3);
    CPU_SET(cpuidx, &cpuset);
    pthread_attr_setaffinity_np(&fifo_sched_attr, sizeof(cpu_set_t), &cpuset);

    // get the max priority and store in paramers
    max_prio=sched_get_priority_max(SCHED_POLICY);
    fifo_param.sched_priority=max_prio;    

    // chnage the schedular policy to curren thread
    if((rc=sched_setscheduler(getpid(), SCHED_POLICY, &fifo_param)) < 0)
        perror("sched_setscheduler");

    pthread_attr_setschedparam(&fifo_sched_attr, &fifo_param);

    printf("ADJUSTED "); print_scheduler();
}

// Entry Function for each thread.
void *counterThread(void *threadp)
{
    int sum=0, i, rc, iterations;
    threadParams_t *threadParams = (threadParams_t *)threadp;
    pthread_t mythread;
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

    // print the syslog to indicate that we ran through each of the thread.
    syslog(LOG_INFO, "[COURSE:%d][ASSIGNMENT:%d]: Thread idx=%d, sum[1...%d]=%d Running on core : %d",
        1, 3, threadParams->threadIdx,
           threadParams->threadIdx, sum, sched_getcpu());

}

// entry function for the starter thread, this is where we create all the threads. 
void *starterThread(void *threadp)
{
   int i, rc;

   printf("starter thread running on CPU=%d\n", sched_getcpu());

   for(i=0; i < NUM_THREADS; i++)
   {
       threadParams[i].threadIdx=i;
       pthread_create(&threads[i],   // pointer to thread descriptor
                      &fifo_sched_attr,     // use FIFO RT max priority attributes
                      counterThread, // thread function entry point
                      (void *)&(threadParams[i]) // parameters to pass in
                     );
   }

    // join all the threads. 
   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);

}

// our main fuction which creates the starter thread. It also prints the assignment header in the syslog file. 
int main (int argc, char *argv[])
{
   int rc;
   int i, j;
   cpu_set_t cpuset;

   struct utsname buf;
   uname(&buf);
   syslog(LOG_INFO,"[COURSE:1][ASSIGNMENT:3] %s %s %s %s %s GNU/Linux",buf.sysname,buf.nodename,buf.release,buf.version,buf.machine);

   // change the scheduler policy to FIFO
   set_scheduler();

   CPU_ZERO(&cpuset);

   // get affinity set for main thread
   mainthread = pthread_self();

   // Check the affinity mask assigned to the thread 
   rc = pthread_getaffinity_np(mainthread, sizeof(cpu_set_t), &cpuset);
   if (rc != 0)
       perror("pthread_getaffinity_np");
   else
   {
       printf("main thread running on CPU=%d, CPUs =", sched_getcpu());
       for (j = 0; j < CPU_SETSIZE; j++)
           if (CPU_ISSET(j, &cpuset))
               printf(" %d", j);
       printf("\n");
   }

    // now create the starter thread
   pthread_create(&startthread,   // pointer to thread descriptor
                  &fifo_sched_attr,     // use FIFO RT max priority attributes
                  starterThread, // thread function entry point
                  (void *)0 // parameters to pass in
                 );

   pthread_join(startthread, NULL);

   printf("\nTEST COMPLETE\n");
}

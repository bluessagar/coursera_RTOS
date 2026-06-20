#define _GNU_SOURCE
#include <pthread.h>        
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>       // To read the system time 
#include <sys/types.h>
#include <sched.h>          

#include <syslog.h> // to print the sys logs

#define NUM_THREADS 64      // Maximum Buber of threads 
#define NUM_CPUS 8          // Number of cores

// thread strucure required by pthread_create
typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];         // Threads struct array
pthread_t mainthread;                   // thread structure for main thread
pthread_t startthread;                      // thread structure for starter thread
threadParams_t threadParams[NUM_THREADS];   // Thread Parameters array

pthread_attr_t fifo_sched_attr;
pthread_attr_t orig_sched_attr;
struct sched_param fifo_param;

#define SCHED_POLICY SCHED_FIFO         // FIFO shecduling policy
#define MAX_ITERATIONS (1000000)    

// function to print the name of the scheduler policy
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

// function to setup the scbeduler 
void set_scheduler(void)
{
    int max_prio, scope, rc, cpuidx;
    cpu_set_t cpuset;

    // first print the current scheduler type
    printf("INITIAL "); print_scheduler();

    // setup the desired scheduler policy to SCHED_FIFO 
    pthread_attr_init(&fifo_sched_attr);                // initialize with default values
    pthread_attr_setinheritsched(&fifo_sched_attr, PTHREAD_EXPLICIT_SCHED); // set the scheduling policy - independent of main thread
    pthread_attr_setschedpolicy(&fifo_sched_attr, SCHED_POLICY);    // set the scheduling policy to FIFO 

    CPU_ZERO(&cpuset);      // clear the CPU selection
    cpuidx=(3);             // set the cpu core to 4
    CPU_SET(cpuidx, &cpuset);   // set the CPU core in the cpu selection

    // set the CPU core affinity for the thread to be created with updated attribute structure
    pthread_attr_setaffinity_np(&fifo_sched_attr, sizeof(cpu_set_t), &cpuset);  

    max_prio=sched_get_priority_max(SCHED_POLICY);  // get the max priority for the scheduling policy
    fifo_param.sched_priority=max_prio;    // set the priority to the max for the scheduling policy

    // set the scheduler policy and priority for the main thread
    if((rc=sched_setscheduler(getpid(), SCHED_POLICY, &fifo_param)) < 0)
        perror("sched_setscheduler");

    // set the scheduler policy and priority for the thread to be created with updated attribute structure
    pthread_attr_setschedparam(&fifo_sched_attr, &fifo_param);

    // first print the current scheduler type after adjustment
    printf("ADJUSTED "); print_scheduler();

    // print the syslog message for tracing. 
    syslog(LOG_INFO, "[COURSE:1][ASSIGNMENT:3]: Main thread running on CPU=%d\n", sched_getcpu());
}

// Entry point function to all the tasks, calculate and print time of the day. 
void *counterThread(void *threadp)
{
    int sum=0, i, rc, iterations;
    threadParams_t *threadParams = (threadParams_t *)threadp;
    pthread_t mythread;
    double start=0.0, stop=0.0;
    struct timeval startTime, stopTime;

    gettimeofday(&startTime, 0);
    // calculate and record start time
    start = ((startTime.tv_sec * 1000000.0) + startTime.tv_usec)/1000000.0;

    // somthing to process in each thread. 
    for(iterations=0; iterations < MAX_ITERATIONS; iterations++)
    {
        sum=0;
        for(i=1; i < (threadParams->threadIdx)+1; i++)
            sum=sum+i;
    }

    // calculate and record stop time
    gettimeofday(&stopTime, 0);
    stop = ((stopTime.tv_sec * 1000000.0) + stopTime.tv_usec)/1000000.0;

    // print the syslog message for tracing.
    syslog(LOG_INFO, "[COURSE:1][ASSIGNMENT:3]: Thread idx=%d, sum[1...%d]=%d, running on CPU=%d, start=%lf, stop=%lf\n", 
           threadParams->threadIdx,
           threadParams->threadIdx, sum, sched_getcpu(),
           start, stop);

    // print the message to the console for tracing.
    printf("\nThread idx=%d, sum[0...%d]=%d, running on CPU=%d, start=%lf, stop=%lf", 
           threadParams->threadIdx,
           threadParams->threadIdx, sum, sched_getcpu(),
           start, stop);
}

// Entry function for starter thread to create the counter threads and wait for their completion
void *starterThread(void *threadp)
{
   int i, rc;

   printf("starter thread running on CPU=%d\n", sched_getcpu());
   syslog(LOG_INFO, "[COURSE:1][ASSIGNMENT:3]: Starter thread running on CPU=%d\n", sched_getcpu());

   for(i=0; i < NUM_THREADS; i++)
   {
       threadParams[i].threadIdx=i;

       // create the counter thread with FIFO scheduling policy
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

// main function to setup the scheduler, create the starter thread 
int main (int argc, char *argv[])
{
   int rc;
   int i, j;            // loop variables
   cpu_set_t cpuset;

   // set the scheduler with FIFO policy as well as set the CPU core to 4 
   set_scheduler();

   CPU_ZERO(&cpuset);   // clear the CPU selection

   // get affinity set for main thread
   mainthread = pthread_self();

   // Check the affinity mask assigned to the thread 
   rc = pthread_getaffinity_np(mainthread, sizeof(cpu_set_t), &cpuset);
   if (rc != 0)
       perror("pthread_getaffinity_np");
   else
   {
        // print the CPU core on which the main thread is running
       printf("main thread running on CPU=%d, CPUs =", sched_getcpu());

       for (j = 0; j < CPU_SETSIZE; j++)
           if (CPU_ISSET(j, &cpuset))
               printf(" %d", j);

       printf("\n");
   }

   // create the starter thread with FIFO scheduling policy and max priority
   pthread_create(&startthread,   // pointer to thread descriptor
                  &fifo_sched_attr,     // use FIFO RT max priority attributes
                  starterThread, // thread function entry point
                  (void *)0 // parameters to pass in
                 );

    // wait for the starter thread to complete
   pthread_join(startthread, NULL);

   printf("\nTEST COMPLETE\n");
   syslog(LOG_INFO, "[COURSE:1][ASSIGNMENT:3]: Test complete\n");

}

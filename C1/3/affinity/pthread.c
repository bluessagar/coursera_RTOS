#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/utsname.h>
#include <syslog.h>
#include <sched.h>

#define COUNT 64
#define COURSE 1
#define ASSIGNMENT 3

typedef struct
{
    int threadIdx;
} threadParams_t;

// POSIX thread declarations and scheduling attributes
//
pthread_t threads[COUNT];
threadParams_t threadParams[COUNT];

void *incThread(void *threadp)
{
    int sum = 0;
    int idx = ((threadParams_t *)threadp)->threadIdx;
    for (int i = 1; i <= idx; i++)
    {
        sum = sum + i;
    }
    syslog(LOG_INFO, "[COURSE:%d][ASSIGNMENT:%d]: Thread idx=%d, sum[1...%d]=%d Running on core : %d",
        COURSE, ASSIGNMENT, idx, idx, sum, sched_getcpu());
    return NULL;
}

int main(int argc, char *argv[])
{
    struct utsname uname_str;
    uname(&uname_str);

    syslog(LOG_INFO, "[COURSE:%d][ASSIGNMENT:%d] %s %s %s %s %s GNU/Linux",
            COURSE, ASSIGNMENT,
            uname_str.sysname, uname_str.nodename, uname_str.release,
            uname_str.version, uname_str.machine);
    
    // Set scheduler policy
    pthread_attr_t fifo_sched_attr;
    struct sched_param fifo_param;
    pthread_attr_init(&fifo_sched_attr);
    pthread_attr_setinheritsched(&fifo_sched_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&fifo_sched_attr, SCHED_FIFO);

    int max_prio = sched_get_priority_max(SCHED_FIFO);
    fifo_param.sched_priority = max_prio;

    if (sched_setscheduler(0, SCHED_FIFO, &fifo_param) < 0) {
        perror("sched_setscheduler");
        return 1;
    }

    pthread_attr_setschedparam(&fifo_sched_attr, &fifo_param);
            
    for (int i = 0; i < COUNT; i++) {
        threadParams[i].threadIdx = i + 1;
        if (pthread_create(&threads[i], &fifo_sched_attr, incThread, (void *)&(threadParams[i])) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("TEST COMPLETE\n");
}

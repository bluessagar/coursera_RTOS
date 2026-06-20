#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <syslog.h>

#define NUM_THREADS 128 
#define SCHED_POLICY SCHED_FIFO //Scheduling policy assigned to threads

//threadParams_t structure with data member of ThreadId...
typedef struct{
	int ThreadId;
} threadParams_t;

//Thread Attributes
pthread_attr_t thread_attr;
//Thread Priority
struct sched_param thread_prio;

void *threadfunc(void* arg)
{
	int sum = 0;
	threadParams_t* threadnum = (threadParams_t*) arg;
	
	for(int i = 1;i < threadnum->ThreadId;i++)
	{
		sum = sum + i;
	}
	syslog(LOG_INFO,"[COURSE:1][ASSIGNMENT:3] Thread idx=%d, sum[1...%d]=%d Running on core: %d",threadnum->ThreadId,threadnum->ThreadId,sum,sched_getcpu());
	return NULL;
}

void conf(void)
{
	int cpuid,max_prio,rc;
	//int tot;
	cpu_set_t cpuset;

	//Thread Attribute Configuration
	pthread_attr_init(&thread_attr);
	pthread_attr_setinheritsched(&thread_attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&thread_attr,SCHED_POLICY);

	//Adding CPU Core to CPU Set for Affinity 
	CPU_ZERO(&cpuset);
	cpuid = 0;
	CPU_SET(cpuid,&cpuset);

	pthread_attr_setaffinity_np(&thread_attr,sizeof(cpu_set_t),&cpuset);
	
	//Parent Thread set to highest priority of the select Scheduling Policy
	max_prio=sched_get_priority_max(SCHED_POLICY);
	thread_prio.sched_priority = max_prio;

	if((rc = sched_setscheduler(getpid(),SCHED_POLICY,&thread_prio)) < 0)
	{
		perror("Setting Scheduler for Parent Thread Failed");
	}

	//Setting the Thread Attribute with the Scheduling Parameter
	pthread_attr_setschedparam(&thread_attr,&thread_prio);
}

pthread_t thread[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];

int main(int argc,char *argv[])
{
	struct utsname buf;
	uname(&buf);
	openlog("",0,LOG_USER);
	syslog(LOG_INFO,"[COURSE:1][ASSIGNMENT:3] %s %s %s %s %s GNU/Linux",buf.sysname,buf.nodename,buf.release,buf.version,buf.machine);
	conf();

	//128 threads created
	for(int i=1;i<=NUM_THREADS;i++)
	{
		threadParams[i-1].ThreadId = i;
		pthread_create(&thread[i-1],&thread_attr,threadfunc,(void*)&(threadParams[i-1]));
	}

	//Thread Joining before Program Ends
	for(int i=1;i<=NUM_THREADS;i++)
	{
		threadParams[i-1].ThreadId = i;
		pthread_join(thread[i-1],NULL);
	}

	return 0;

}
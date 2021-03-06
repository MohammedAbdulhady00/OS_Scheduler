#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <signal.h>
#include <sys/shm.h>
#include <unistd.h>
#include "SRTN.h"
#include "../DataStructures/Priority_Queue.h"
#include "../IO.h"

#ifdef scheduler_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN process* CurrentProcess;
EXTERN FILE *logFile;
EXTERN int time;

bool SRTNCmp(process *p1, process *p2)
{
    if (p1->remainingTime < p2->remainingTime)
        return true;

    if (p1->remainingTime == p2->remainingTime)
        return p1->arrivalId < p2->arrivalId;

    return false;
}

void *SRTNInit(void *args)
{
    return CreatePriorirtyQueue(SRTNCmp);
}

// void SRTNNewProcessHandler(void* ReadyQueue, process* p) {
//     //create_process(p);
//     //kill(p->pWaitId,SIGSTOP);
//     if(CurrentProcess != NULL && p->runningTime < *(CurrentProcess->remainingTime)) {
//         CurrentProcess->state = STOPPED;
//         //TODO: LOG PROCESS STOPPED
//         kill((pid_t)CurrentProcess->pWaitId, SIGSTOP);
//         //CurrentProcess->state = STOPPED;
//         printf("Process %d Stopped!remain time %d\n", CurrentProcess->pid,*CurrentProcess->remainingTime);
//         logProcess(logFile, CurrentProcess, time_after);
//         create_process(p);
//         //kill(p->pWaitId,SIGCONT);
//         PriorityQueuePush((PriorityQueue *)ReadyQueue, CurrentProcess);
//         CurrentProcess = p;
//         printf("Process %d Started! remain time %d\n", CurrentProcess->pid,*CurrentProcess->remainingTime);
//         p->waitTime = time_after - p->arrivalTime;
//         logProcess(logFile, p, time_after);
//     }
//     else if(CurrentProcess != NULL) {
//         //TODO: LOG ENQUEUE
//         printf("No preemption\n");
//         PriorityQueuePush((PriorityQueue *)ReadyQueue, p);
//         printf("No Errors\n");
//         return;
//     }
//     else {
//         //TODO LOG STARTED
//         create_process(p);
//         //kill(p->pWaitId,SIGCONT);
//         p->state = STARTED;
//         CurrentProcess = p;
//         p->waitTime = time_after - p->arrivalTime;
//         printf("Process %d Started!\n", CurrentProcess->pid);
//         logProcess(logFile, p, time_after);
//     }
// }

void SRTNNewProcessHandler(void* ReadyQueue, process* p) {
    PriorityQueuePush((PriorityQueue*)ReadyQueue, p);
}
void SRTNEnqueue(void *ReadyQueue, process *p)
{
    PriorityQueuePush((PriorityQueue *)ReadyQueue, p);
}

process *SRTNRunNext(void *ReadyQueue)
{
    if (PriorityQueueEmpty((PriorityQueue *)ReadyQueue))
        return NULL;
    process *p = PriorityQueueGetMin((PriorityQueue *)ReadyQueue);
    if (p->state == STOPPED)
    {
        kill(p->pWaitId,SIGCONT);
        p->state = RESUMED;
        printf("Process %d Resumed!\n", p ->pid);
        //p->waitTime = time_after - p->arrivalTime;
        logProcess(logFile, p, time);
    }
    else
    {
        //create_process(p);
        kill(p->pWaitId,SIGCONT);
        p->state = STARTED;
        printf("Process %d Started!\n", p ->pid);
        p->waitTime = time - p->arrivalTime;
        logProcess(logFile, p, time);
    }
    CurrentProcess = p;
    PriorityQueuePop((PriorityQueue *)ReadyQueue);
    // TODO: FORK next process possibly need to implement a new function as this will be shared among all algorithms

    return NULL;
}

void SRTNNewProcessFinalizationHandler(void *ReadyQueue)
{
    if(PriorityQueueEmpty((PriorityQueue*)ReadyQueue))
        return;
      
    process *p = PriorityQueueGetMin((PriorityQueue *)ReadyQueue);
    if(CurrentProcess == NULL) {
        PriorityQueuePop((PriorityQueue*)ReadyQueue);
        if (p->state == STOPPED)
            {
                kill(p->pWaitId,SIGCONT);
                p->state = RESUMED;
                printf("Process %d Resumed! remain time %d\n", p ->pid, p->remainingTime);
                p->waitTime = time - p->arrivalTime - p->runningTime + p->remainingTime;
                logProcess(logFile, p, time);
            }
            else
            {
                create_process(p);
                //kill(p->pWaitId,SIGCONT);
                p->state = STARTED;
                p->waitTime = time - p->arrivalTime - p->runningTime + p->remainingTime;
                printf("Process %d Started! remain time %d \n", p ->pid, p->remainingTime);
                logProcess(logFile, p, time);
            }
            CurrentProcess = p;
    }
    else if (CurrentProcess->remainingTime > p->remainingTime) { //Check preemption 
        PriorityQueuePop((PriorityQueue*)ReadyQueue);
        PriorityQueuePush((PriorityQueue*)ReadyQueue, CurrentProcess);
        if (p->state == STOPPED)
            {
                kill(p->pWaitId,SIGCONT);
                p->state = RESUMED;
                //p->waitTime = time_after - p->arrivalTime;
                printf("Process %d Resumed! remain time %d\n", p ->pid, p->remainingTime);
                p->waitTime = time - p->arrivalTime - p->runningTime + p->remainingTime;
            }
            else
            {
                create_process(p);
                //kill(p->pWaitId,SIGCONT);
                p->state = STARTED;
                p->waitTime = time - p->arrivalTime - p->runningTime + p->remainingTime;
                printf("Process %d Started! remain time %d \n", p ->pid, p->remainingTime);
            }
            
        CurrentProcess->state = STOPPED;
        //TODO: LOG PROCESS STOPPED
        kill((pid_t)CurrentProcess->pWaitId, SIGSTOP);
        //CurrentProcess->state = STOPPED;
        printf("Process %d Stopped! remain time %d\n", CurrentProcess->pid, CurrentProcess->remainingTime);
        logProcess(logFile, CurrentProcess, time);
        logProcess(logFile, p, time);
        CurrentProcess = p;
    } 
    

}
void SRTNTerminationHandler(void *ReadyQueue)
{
    int statloc =0;
    printf("Process %d terminated!\n", CurrentProcess->pid);
    CurrentProcess->state = FINISHED;
    CurrentProcess->finishTime = time;
    logProcess(logFile, CurrentProcess, time);
    //waitpid(CurrentProcess->pWaitId,&statloc);
    CurrentProcess = NULL;
    // if (PriorityQueueEmpty((PriorityQueue *)ReadyQueue)) {
    //     CurrentProcess = NULL;
    // }
    // else {

    //     process * p = PriorityQueueGetMin((PriorityQueue *)ReadyQueue);
    //     PriorityQueuePop((PriorityQueue *)ReadyQueue);
    //     if (p->state == STOPPED)
    //     {
    //         kill(p->pWaitId,SIGCONT);
    //         p->state = RESUMED;
    //         printf("Process %d Resumed! remain time %d\n", p ->pid, *p->remainingTime);
    //         //p->waitTime = time_after - p->arrivalTime;
    //         logProcess(logFile, p, time_after);
    //     }
    //     else
    //     {
    //         //kill(p->pWaitId,SIGCONT);
    //         create_process(p);
    //         p->state = STARTED;
    //         printf("Process %d Started! remain time %d\n", p ->pid, *p->remainingTime);
    //         p->waitTime = time_after - p->arrivalTime;
    //         logProcess(logFile, p, time_after);
    //     }
    //     CurrentProcess = p;
    // }
}

void SRTNDestroy(void *ReadyQueue)
{
    DestroyPriorityQueue((PriorityQueue *)ReadyQueue);
}
bool SRTNEmpty(void *ReadyQueue)
{
    return PriorityQueueEmpty((PriorityQueue *) ReadyQueue) ;
}
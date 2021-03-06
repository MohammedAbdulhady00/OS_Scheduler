#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include "../DataStructures/LinkedQueue.h"
FILE *logFile, *perfFile;
char getch(void)
{
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    printf("%c\n", buf);
    return buf;
}
void readInput(LinkedQueue *Q)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    process *tmp;
    FILE *file = fopen("processes.txt", "r");
    if (NULL == file)
    {
        perror("file can't be opened \n");
    }

    while ((read = getline(&line, &len, file)) != -1)
    {
        if (line[0] == '#')
            continue;
        tmp = malloc(sizeof(process));
        tmp->pid = atoi(strtok(line, "\t"));
        tmp->arrivalTime = atoi(strtok(NULL, "\t"));
        tmp->runningTime = atoi(strtok(NULL, "\t"));
        tmp->priority = atoi(strtok(NULL, "\t"));
        enqueueLinkedQueue(Q, tmp);
    }
    if (line)
        free(line);
}
void arrow(int rp, int p)
{
    if (rp == p)
    {
        printf("--> ");
    }
    else
    {
        printf("    ");
    }
}
int menu(int *position)
{
#define MAX 3
#define MIN 1
    (*position) = 1;
    int keypressed = 0, Quantum = 0;
    while (keypressed != 10)
    {

        system("clear");
        arrow(1, (*position));
        printf("1.HPF\n");
        arrow(2, (*position));
        printf("2.SRTN\n");
        arrow(3, (*position));
        printf("3.RR\n");
        keypressed = getch();
        if (keypressed == 66 && (*position) != MAX)
            (*position)++;
        else if (keypressed == 65 && (*position) != MIN)
            (*position)--;
    }
    if ((*position) == 3)
    {
        system("clear");
        printf("Please Enter the Quantum: ");
        scanf("%d", &Quantum);
    }
    return Quantum;
}
void logProcess(FILE *logFile, process *p, int clk)
{
    fprintf(logFile, "At time %d process %d ", clk, p->pid);
    switch (p->state)
    {
    case STARTED:
        fprintf(logFile, "started arr %d total %d remain %d wait %d\n", p->arrivalTime, p->runningTime, remaintime, p->waitTime);
        break;
    case RESUMED:
        fprintf(logFile, "resumed arr %d total %d remain %d wait %d\n", p->arrivalTime, p->runningTime, remaintime, p->waitTime);
        break;
    case STOPPED:
        fprintf(logFile, "stopped arr %d total %d remain %d wait %d\n", p->arrivalTime, p->runningTime, remaintime, p->waitTime);
        break;
    case FINISHED:
        fprintf(logFile, "finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", p->arrivalTime, p->runningTime, remaintime, p->waitTime, p->finishTime - p->arrivalTime, ((float)p->finishTime - p->arrivalTime) / p->runningTime);
        break;
    default:
        break;
    }
}
void test(LinkedQueue *Q)
{
    // printf("#id arrival runtime priority\n");
    process *p;
    while (Q->front)
    {
        p = Q->front->val;
        for (int i = 0; i < 4; i++)
        {
            p->state = (enum pState)i;
            p->arrivalTime = 0;
            p->waitTime = 0;
            p->finishTime = i;
            logProcess(logFile, p, i);
        }
        // printf("%d\t%d\t%d\t%d\n", p->pid, p->arrivalTime, p->runningTime, p->priority);
        dequeueLinkedQueue(Q);
        free(p);
    }
}
void initializeOut(/*FILE **logFile, FILE **perfFile*/)
{
    // Opening file in reading mode
    logFile = fopen("scheduler.log", "w+");
    if (logFile == NULL)
    {
        printf("Error!");
        exit(1);
    }
    fprintf(logFile, "#At time x process y state arr w total z remain y wait k\n");
    perfFile = fopen("scheduler.perf", "w+");
    if (perfFile == NULL)
    {
        printf("Error!");
        exit(1);
    }
}
void freeOut(/*FILE **log, FILE **perf*/)
{
    fclose(perfFile);
    fclose(logFile);
}
int main()
{
    LinkedQueue *Q = CreateLinkedQueue();
    int position, Quantum;
    initializeOut(/*&log, &perf*/);
    readInput(Q);
    Quantum = menu(&position);
    test(Q);
    freeOut(/*&log, &perf*/);
    DestroyLinkedQueue(Q);
    printf("Read Input test completed successfully!\n");
    printf("You choose Option = %d Quantum = %d!\n", position, Quantum);
    return 0;
}

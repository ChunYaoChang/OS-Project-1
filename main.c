#define _GNU_SOURCE
#include <sched.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#define FIFO 1
#define RR 2
#define SJF 3
#define PSJF 4
#define PARENT_CPU 0
#define CHILD_CPU 1
#define MY_TIME 334
#define MY_PRINTK 335

int running_procs;
int now_time;
int finish_procs;
int context_switch_time;
static int lock = 0;

typedef struct process
{
    char name[32];
    int ready_time;
    int exec_time;
    pid_t pid;
} Process;

static void unlock()
{
    // printf("fuck\n");
    lock = 1;
}

void assign_cpu(int pid, int mode)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(mode, &mask);
    sched_setaffinity(pid, sizeof(mask), &mask);
}

void invoke_process(int pid)
{
    struct sched_param param;
    param.sched_priority = 0;
    sched_setscheduler(pid, SCHED_OTHER, &param);
}

void sleep_process(int pid)
{
    struct sched_param param;
    param.sched_priority = 0;
    sched_setscheduler(pid, SCHED_IDLE, &param);
}

void unit_time()
{
    volatile unsigned long i;
	for (i = 0; i < 1000000UL; i++);	
}

int create_process(Process pr)
{
    int pid = fork();
    /* child process */
    if (pid == 0)
    {
        while (lock == 0);
        unsigned long int start_time, start_ntime;
        unsigned long int end_time, end_ntime;
        char dmesg_infor[128];
        int tmp = syscall(MY_TIME, &start_time, &start_ntime);
        // printf("%d\n", tmp);
        for (int i = 0; i < pr.exec_time; i++)
        {
            unit_time();
            // if (i % 100 == 0) printf("child process\n");
        }
        // printf("%d %d\n", start_time, start_ntime);
        tmp = syscall(MY_TIME, &end_time, &end_ntime);
        sprintf(dmesg_infor, "[Project1] %d %lu.%09lu %lu.%09lu\n", getpid(), start_time, start_ntime, end_time, end_ntime);
        syscall(MY_PRINTK, dmesg_infor);
        // printf("%s\n", dmesg_infor);
        exit(0);
    }
    /* parent process */
    else
    {
        assign_cpu(pid, CHILD_CPU);
        return pid;
    }
}

int cmp(const void *a, const void *b)
{
    return (((Process *)a)->ready_time - ((Process *)b)->ready_time);
}

int select_next_process(Process *proc, int num_procs, int policy)
{
    /* non-preemptive */
    if (running_procs != -1 && (policy == SJF || policy == FIFO))
        return running_procs;

    int selected_proc = -1;
    /* PSJF SJF */
    if (policy == PSJF || policy == SJF)
    {
        for (int i = 0; i < num_procs; i++)
        {
            if (proc[i].pid == -1 || proc[i].exec_time == 0) continue;
            if (selected_proc == -1 || proc[i].exec_time < proc[selected_proc].exec_time)
                selected_proc = i;
        }
    }
    /* FIFO */
    else if (policy == FIFO)
    {
        for (int i = 0; i < num_procs; i++)
        {
            if (proc[i].pid == -1 || proc[i].exec_time == 0) continue;
            if (selected_proc == -1 || proc[i].ready_time < proc[selected_proc].ready_time)
                selected_proc = i;
        }
    }
    /* RR */
    else if (policy == RR)
    {
        /* if there's no process running, choose first ready and unfinish process */
        if (running_procs == -1)
        {
            for (int i = 0; i < num_procs; i++)
                if (proc[i].pid != -1 && proc[i].exec_time > 0)
                {
                    selected_proc = i;
                    break;
                }
        }
        /* else if time_quantum expired, choose next ready and unfinish process */
        else if ((now_time - context_switch_time) % 500 == 0)
        {
            selected_proc = (running_procs + 1) % num_procs;
            while (proc[selected_proc].pid == -1 || proc[selected_proc].exec_time == 0)
            {
                if (selected_proc == running_procs) break;
                selected_proc = (selected_proc + 1) % num_procs;
            }
        }
        else selected_proc = running_procs;
    }
    return selected_proc;
}

int schedule(Process *proc, int num_procs, int policy)
{
    qsort(proc, num_procs, sizeof(Process), cmp);
    for (int i = 0; i < num_procs; i++)
        proc[i].pid = -1;
    
    assign_cpu(getpid(), PARENT_CPU);
    invoke_process(getpid());

    running_procs = -1;
    now_time = 0;
    finish_procs = 0;

    while(1)
    {
        /* if there's process finish, wait it */
        if (running_procs != -1 && proc[running_procs].exec_time == 0)
        {
            waitpid(proc[running_procs].pid, NULL, 0);
            printf("%s %d\n", proc[running_procs].name, proc[running_procs].pid);
            fflush(stdout);
            running_procs = -1;
            finish_procs++;
            if (finish_procs == num_procs) break;
        }
        /* if there's process ready, execute it */
        for (int i = 0; i < num_procs; i++)
        {
            if (proc[i].ready_time == now_time)
            {
                proc[i].pid = create_process(proc[i]);
                sleep_process(proc[i].pid);
                // printf("%s, pid = %d\n", proc[i].name, proc[i].pid);
            }
        }
        /* select next process to run */
        int next_procs = select_next_process(proc, num_procs, policy);
        // if (now_time % 100 == 0) printf("now_time = %d, next_procs = %d\n", now_time, next_procs);
        if (next_procs != -1)
        {
            if (next_procs != running_procs)
            {
                sleep_process(proc[running_procs].pid);
                kill(proc[next_procs].pid, SIGUSR1);
                invoke_process(proc[next_procs].pid);
                running_procs = next_procs;
                context_switch_time = now_time;
            }
        }
        unit_time();
        if (running_procs != -1) proc[running_procs].exec_time--;
        now_time++;
        // if (now_time % 100 == 0) printf("scheduler\n");
    }
}

int to_int(char* sched_policy)
{
    if (strcmp(sched_policy, "FIFO") == 0) return 1;
    if (strcmp(sched_policy, "RR") == 0) return 2;
    if (strcmp(sched_policy, "SJF") == 0) return 3;
    if (strcmp(sched_policy, "PSJF") == 0) return 4;
}

int main(int argc, char *argv[])
{
    char sched_policy[8];
    scanf("%s", sched_policy);
    int policy = to_int(sched_policy);
    int num_procs;
    scanf("%d", &num_procs);
    Process *proc = (Process*)malloc(num_procs * sizeof(Process));
    for (int i = 0; i < num_procs; i++)
        scanf("%s%d%d", proc[i].name, &proc[i].ready_time, &proc[i].exec_time);

    signal(SIGUSR1, unlock);
    schedule(proc, num_procs, policy);
    exit(0);
}
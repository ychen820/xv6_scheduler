#include "types.h"
#include "user.h"

/* CS550 ATTENTION: to ensure correct compilation of the base code, 
   stub functions for the system call user space wrapper functions are provided. 
   REMEMBER to disable the stub functions (by commenting the following macro) to 
   allow your implementation to work properly. */
#define STUB_FUNCS
#ifdef STUB_FUNCS
//void set_sched(int s) {}
//void set_priority(int pid, int priority) {}
#endif

#define CHILD_COUNT  6
#define SLEEP_TICKS  30

#define LOOP_CNT 0x01000000

void usage(void)
{
    printf(1, "Usage: proj1_schdtest scheduler_type [with_rg_proc] \n"
              "\tscheduler_type:\n"
              "\t\t0: Use the default xv6 scheduler\n"
              "\t\t1: Use the priority-based scheduler\n"
              "\twith_rg_proc (used with priority-based scheduler):\n"
              "\t\t0: Without runtime-generated process\n"
              "\t\t1: With runtime-generated process\n");
}

int
main(int argc, char *argv[])
{
    int scheduler = 0, with_rg_proc = 0;
    int i = 0, j = 0;
    int tmp = 0;
    struct {
        int pid;
        int priority;
    } pa[CHILD_COUNT];
    int rg_pid = 0;
    int pipe_fd[CHILD_COUNT][2];
    int c = 0;

    if (argc < 2)
    {
        usage();
        exit();
    }

    printf(1, "==============\n");

    if (argv[1][0] == '0')
    {
        scheduler = 0;
        printf(1, "Using the default xv6 scheduler \n");
    }
    else
    {
        scheduler = 1;
        printf(1, "Using the priority based scheduler, ");

        if (argc < 3)
        {
            usage();
            exit();
        }

        if (argv[2][0] == '0')
        {
            with_rg_proc = 0;
            printf(1, "without runtime-generated process\n");
        }
        else
        {
            with_rg_proc = 1;
            printf(1, "with runtime-generated process\n");
        }
    }

    printf(1, "--------------\n");

    set_sched(scheduler);
    
    for (i = 0; i < CHILD_COUNT; i++)
    {
        if (pipe(pipe_fd[i]) < 0)
        {
            printf(1, "pipe() failed!\n");
            exit();
        }

        pa[i].pid = fork();
        if (pa[i].pid < 0)
        {
            printf(1, "fork() failed!\n");
            exit();
        }
        else if (pa[i].pid == 0) // child
        {  
            #if 1
            close(pipe_fd[i][1]); // close the write end
            read(pipe_fd[i][0], &c, 1);
            #endif

            while(j < LOOP_CNT)
            {
                tmp += j;
                j++;
            }
            
            exit();
        }
        else // parent
        {
            close(pipe_fd[i][0]); // close the read end

            if (scheduler == 0) // RR
            {
                printf(1, "Parent: child (pid=%d) created!\n", 
                       pa[i].pid, pa[i].priority);
            }
            else // priority-based
            {
                if (i == 0 || i == 1)
                {
                    pa[i].priority = 1;
                    set_priority(pa[i].pid, pa[i].priority);
                }
                else if (i == 4 || i == 5)
                {
                    pa[i].priority = 3;
                    set_priority(pa[i].pid, pa[i].priority);
                }
                else 
                {
                    pa[i].priority = 2;
                    //Note: no need to set priority, because it should be set to 2 by the kernel by default
                }

                printf(1, "Parent: child (pid=%d priority=%d) created!\n", 
                       pa[i].pid, pa[i].priority);
            }
        }
    }

    printf(1, "==============\n");

    enable_sched_trace(1);

    #if 1
    for (i = 0; i < CHILD_COUNT; i++)
    {   
       write(pipe_fd[i][1], "A", 1); // start child-i
    }
    #endif

    for (i = 0; i < CHILD_COUNT; i++)
    {   
        if (with_rg_proc)
        {
            if (i == 4)
            {
                rg_pid = fork();
                if (pa[i].pid < 0)
                {
                    printf(1, "fork() failed!\n");
                    exit();
                }
                else if (rg_pid == 0) // child
                {
                    while(j < LOOP_CNT)
                    {
                        tmp += j;
                        j++;
                    }
                    
                    exit();
                }
                else // parent
                {
                    if (wait() < 0)
                    {
                        printf(1, "wait() on child-%d failed!\n", i);
                    }
                }
            }
        }

        if (wait() < 0)
        {
            printf(1, "wait() on child-%d failed!\n", i);
        }
    }

    enable_sched_trace(0);

    printf(1, "\n");
    
    exit();
}

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#define CHILDNUM 10

int count = 0;
int i = 0;
int total_count = 0;
pid_t pid[CHILDNUM];
int child_execution_time[CHILDNUM] ={6,10,6,5,4,3,2,1,6,5}; 
int front, rear = 0;
int run_queue[20];

void signal_user_handler(int signum)  // sig child handler 
{
	
        printf("pid: %d remaining cpu-burst%d\n",getpid(),child_execution_time[i]);
	child_execution_time[i] -- ; 
	if(child_execution_time[i] <= 0)
	{
		//printf("child end\n");
		exit(0);
	}
}

void signal_callback_handler(int signum)  // sig parent handler
{
	total_count ++;
	count ++;
        if(total_count >= 49 )
                exit(0);
	
	printf("time %d:\n",total_count);
	kill(pid[run_queue[front% 20]],SIGINT);
	child_execution_time[run_queue[front%20]] --;
	if((count == 3)|(child_execution_time[run_queue[front%20]]==0)){
		//printf("front : %d , rear %d\n",front,rear);
		//printf("child_time : %d ",child_time[run_queue[front&10]]);
		count  = 0;
	        if(child_execution_time[run_queue[front%20]] != 0)
        	        run_queue[(rear++)%20] = run_queue[front%20];
		front ++; 
	}
}


int main(int argc, char *argv[])
{
	//pid_t pid;
	
        while(i< CHILDNUM) {
        pid[i] = fork();
        run_queue[(rear++)%20] = i ;
        if (pid[i]== -1) {
                perror("fork error");
                return 0;
        }
        else if (pid[i]== 0) {
                //child
                struct sigaction old_sa;
                struct sigaction new_sa;
		memset(&new_sa, 0, sizeof(new_sa));
		new_sa.sa_handler = &signal_user_handler;
                sigaction(SIGINT, &new_sa, &old_sa);
		while(1);
		return 0;
        }
        else {
                //parent
                //printf("my pid is %d\n", getpid());
		// iterative signal , timer --> alarm
		struct sigaction old_sa;
		struct sigaction new_sa;
		memset(&new_sa, 0, sizeof(new_sa));	

		new_sa.sa_handler = &signal_callback_handler;
		sigaction(SIGALRM, &new_sa, &old_sa);

		struct itimerval new_itimer, old_itimer;
		new_itimer.it_interval.tv_sec = 1;
		new_itimer.it_interval.tv_usec = 0;
		new_itimer.it_value.tv_sec = 1;
		new_itimer.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL, &new_itimer, &old_itimer);
 	}
	i++;
        }
	while(1);
        return 0;

}

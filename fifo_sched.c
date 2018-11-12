#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>


int count = 0;
int i = 0;
int total_count = 0;
pid_t pid[3];
int child_execution_time[3] ={9,5,3}; 
int front, rear = 0;
int run_queue[10];


void signal_child_handler(int signum)  // sig child handler 
{
	
	printf("child remaining execution time = %d\n", child_execution_time[i]);
	child_execution_time[i]-- ;
	if(child_execution_time[i]<=0){	
	printf("Process end");
	exit(1);
	}
}


void signal_parent_handler(int signum)  // sig parent handler
{
	if(front == rear)
		exit(1);
	
//	printf("Got parent signal:%d\n",signum);
	
	kill(pid[run_queue[front%10]],SIGINT);
	child_execution_time[run_queue[front%10]] --;
	if(child_execution_time[run_queue[front%10]] == 0){
		front ++;
	}
	total_count ++;
	printf("time: %d ==================\n", total_count);
	//printf("Total execution time = %d \n", total_count);
	
}


int main(int argc, char *argv[])
{
	//pid_t pid;
	while(i< 3) 
	{
		pid[i] = fork();
        run_queue[(rear++)%10] = i ;
        if (pid[i]== -1) 
		{
        	perror("fork error");
            return 0;
        }
        else if (pid[i]== 0) 
		{//child
			struct sigaction old_sa;
            struct sigaction new_sa;
			memset(&new_sa, 0, sizeof(new_sa));
			new_sa.sa_handler = &signal_child_handler;
			sigaction(SIGINT, &new_sa, &old_sa);
		

			while(1);
			return 0;
        }
        else 
		{//parent
			struct sigaction old_sa;
			struct sigaction new_sa;
			memset(&new_sa, 0, sizeof(new_sa));	

			new_sa.sa_handler = &signal_parent_handler;
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


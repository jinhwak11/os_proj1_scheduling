#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


int count = 0;
int i = 0;
int total_count = 0;
pid_t pid[3];
int child_execution_time[3] ={6,6,6}; 
int child_io_time[3]={7,3,3};
int child_io_ctime[3]={7,3,3};
int front, rear = 0;
int w_front, w_rear = 0;
int run_queue[10];
int wait_queue[10];

void wait_queue_check()
{
	int walk = w_front;
	if( (w_front%10) != (w_rear%10)){ //when the wait queue is not empty
		while((walk%10)!=(w_rear)%10){
			printf("process %d in I/O\n ",wait_queue[walk%10]);

			child_io_time[wait_queue[walk%10]] -- ;
			if(child_io_time[wait_queue[walk%10]] == 0){
				w_front ++;
				run_queue[(rear++)%10] = wait_queue[walk%10];
				child_io_time[wait_queue[walk%10]] = child_io_ctime[wait_queue[walk%10]];
			}
			walk++;

		}
	}
			
}

void signal_user_handler(int signum)  // sig child handler 
{
	int curr_execution_time ;
	curr_execution_time = child_execution_time[i];
        printf("caught signal %d %d\n",signum,getpid());
	curr_execution_time-- ; 
	if(curr_execution_time <= 0)
	{
		printf("child process end and will go to io\n");
		curr_execution_time = child_execution_time[i];
//		exit(0);
	}
}

void signal_callback_handler(int signum)  // sig parent handler
{
        //printf("Caught signal_parent %d\n",signum);
	total_count ++;
	count ++;
        if(total_count >= 30 ){
		for(int k = 0; k < 3 ; k ++)
		{
			kill(pid[k],SIGKILL);
		}
                exit(0);
	}
	if((front%10) != (rear%10)){
	kill(pid[run_queue[front% 10]],SIGINT);
	child_execution_time[run_queue[front%10]] --;
	}
	wait_queue_check();
	if((count == 3)|(child_execution_time[run_queue[front%10]]==0)){
		//printf("front : %d , rear %d\n",front,rear);
		//printf("child_time : %d ",child_time[run_queue[front&10]]);
		count  = 0;
	        if(child_execution_time[run_queue[front%10]] != 0)
        	        run_queue[(rear++)%10] = run_queue[front%10];
		if(child_execution_time[run_queue[front%10]] == 0 )
			wait_queue[(w_rear++)%10] = run_queue[front%10];
		front ++; 
	}
}


int main(int argc, char *argv[])
{
	//pid_t pid;
	
        while(i< 3) {
        pid[i] = fork();
        run_queue[(rear++)%10] = i ;
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

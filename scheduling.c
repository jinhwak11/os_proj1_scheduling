nclude <stdio.h>
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
int seq = 0;

void signal_user_handler(int signum)
{
	
        printf("caught signal %d %d\n",signum,getpid());
	count ++;
	if(count == 3)
		exit(0);
}

void signal_callback_handler(int signum)
{
        printf("Caught signal_parent %d\n",signum);
	total_count ++;
	kill(pid[seq],SIGINT);
	if((total_count %3)== 0){
		seq++;
	}
	if(total_count ==11) 
		exit(0);
}


int main(int argc, char *argv[])
{
	//pid_t pid;
	
        while(i< 3) {
        pid[i] = fork();
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

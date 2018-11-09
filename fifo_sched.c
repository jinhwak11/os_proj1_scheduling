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

struct msgbuf
{
    int mtype;
    int pid;
    int exec_time[3];
};

int count = 0;
int i = 0;
int total_count = 0;
pid_t pid[3];
int child_execution_time[3] ={9,5,3}; 
int front, rear = 0;
int run_queue[10];


void signal_child_handler(int signum)  // sig child handler 
{
	
	printf("Got child signal:%d id:%d\n",signum,getpid());
	printf("child execution time = %d\n", child_execution_time[i]);
	while(child_execution_time[i]>0)
	{
		child_execution_time[i]-- ;
		printf("child execution time = %d\n", child_execution_time[i]);
	}
	
	printf("child end\n");
	exit(1);
}


void signal_parent_handler(int signum)  // sig parent handler
{
	if(front == rear)
		exit(1);
    printf("Got parent signal:%d\n",signum);
	
	kill(pid[run_queue[front%10]],SIGINT);
	//front++;

	while(child_execution_time[run_queue[front%10]]>0)
	{
		count = child_execution_time[run_queue[front%10]];
		for(int j=0; j<count; j++)
		{
			//printf("child execution time = %d\n", child_execution_time[run_queue[front%10]]);
			child_execution_time[run_queue[front%10]] --;
		} 
	}
	printf("Total execution time = %d \n", count);

	front++;
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

			while(1)    //to send msgq
        	{
            	int key;
            	int msgq;
            	int ret;

	            key = 0x12345;
    	        msgq = msgget(key, 0666| IPC_CREAT); //msgget create a message queue and return the identifier
        	    printf("sender msgq: %d\n", msgq); //print out identifier
            	struct msgbuf msg;
	            memset(&msg, 0, sizeof(msg));
    	        msg.mtype =0;
        	    msg.pid = getpid();
	            msg.exec_time[i] = child_execution_time[i];
    	        printf("Sending child %d execution time: %d\n",msg.pid, msg.exec_time[i]);
        	    ret = msgsnd(msgq, &msg, sizeof(msg), NULL); //send the message
            	if(ret == -1)
                	perror("msgsnd error");
                
				return 0;
        	}
			struct sigaction old_sa;
            struct sigaction new_sa;
			memset(&new_sa, 0, sizeof(new_sa));
			new_sa.sa_handler = &signal_child_handler;
			sigaction(SIGINT, &new_sa, &old_sa);
		
			return 0;
        }
        else 
		{//parent
			while(waitpid(pid[i], NULL,0) != -1)
            {
                int key;
                int msgq;
                int ret;
                key = 0x12345;
                msgq = msgget(key, 0666| IPC_CREAT); //create queue
                printf("receiver msgq: %d\n", msgq); //print out identifier

                struct msgbuf msg;
                memset(&msg, 0, sizeof(msg));
                ret = msgrcv(msgq,&msg,sizeof(msgq),0,NULL); //to receive message
                printf("Received msg!\nmsg.exec_time[i]: %d\n", msg.exec_time[i]);

                msgctl(msgq, IPC_RMID, NULL);
                break;
            }
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

}


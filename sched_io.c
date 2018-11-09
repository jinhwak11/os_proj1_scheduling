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
typedef struct node{
	int data;  // io _time
    	int pid_index;
	struct node *ptr;
} node;

struct msgbuf{
	long int  mtype;
	int pid_index;
	int io_time;
};

int child_io_time[3]={0};
node* insert(node* head, int num, int pid_index) {
    node *temp, *prev, *next;
    temp = (node*)malloc(sizeof(node));
    temp->data = child_io_time[pid_index];
    temp->pid_index = pid_index;
    temp->ptr = NULL;
    if(!head){
        head=temp;
    } else{
        prev = NULL;
        next = head;
        while(next && next->data <= num){
            prev = next;
            next = next->ptr;
        }
        if(!next){
            prev->ptr = temp;
        } else{
            if(prev) {
                temp->ptr = prev->ptr;
                prev-> ptr = temp;
            } else {
                temp->ptr = head;
                head = temp;
            }            
        }   
    }
    return head;
}


node* delete(node* head)
{
        node* temp;
        temp = head;
        head = temp->ptr;
	free(temp);

	return head;
}

void free_list(node *head) {
    node *prev = head;
    node *cur = head;
    while(cur) {
        prev = cur;
        cur = prev->ptr;
        free(prev);
    }
}



int count = 0;
int i = 0;
int total_count = 0;
pid_t pid[3];
int child_execution_time[3] ={6,6,6};
int child_execution_ctime[3];
node* head ;
//int child_io_ctime[3]={1,2,3};
int front, rear = 0;
//int w_front, w_rear = 0;
int run_queue[10];
//int wait_queue[10];
int curr_execution_time ;
int io_time;


int msgq;
int ret;
int key = 0x12345;
int flag=0;
struct msgbuf msg;

node* wait_queue_check(node* head)
{
	node* p;
	p= head;
		
	while(p)
	{
		printf("process %d in IO\n",p->pid_index);
		p->data--;
		if(p->data <= 0){
			//printf("process %d finish in IO\n",p->pid_index);
			run_queue[(rear++)%10] = p->pid_index;
			head = delete(head);
		}
		p = p->ptr;
	}		
	return head;
}

void signal_user_handler(int signum)  // sig child handler 
{
        printf("caught signal %d %d\n",signum,getpid());
	child_execution_time[i]-- ; 
	if(child_execution_time[i] <= 0)
	{
	//	printf("child process end and will go to io\n");
		child_execution_time[i] = curr_execution_time; // recover execution time
		//여기다가 메세지에다가 io time 보내주는 거 넣어야함
		memset(&msg,0,sizeof(msg));
		msg.mtype = IPC_NOWAIT;
		msg.pid_index = i;
		msg.io_time = io_time;
		ret = msgsnd(msgq, &msg, sizeof(msg),IPC_NOWAIT);
		if(ret == -1)
			perror("msgsnd error");
		//else
		//	printf("send message");
	}
}

void signal_callback_handler(int signum)  // sig parent handler
{
        //printf("Caught signal_parent %d\n",signum);
	if(flag == 1){
		head= insert(head ,child_io_time[run_queue[(front-1)%10]], run_queue[(front-1)%10]);
		flag = 0;
	}
	total_count ++;
	count ++;
        if(total_count >= 60 ){
		for(int k = 0; k < 3 ; k ++)
		{
			kill(pid[k],SIGKILL);
		}
                exit(0);
	}
	head=wait_queue_check(head);
	if((front%10) != (rear%10)){
		kill(pid[run_queue[front% 10]],SIGINT);
		child_execution_time[run_queue[front%10]] --;
	}
//	wait_queue_check();
	if((count == 3)|(child_execution_time[run_queue[front%10]]==0)){
		//printf("front : %d , rear %d\n",front,rear);
		//printf("child_time : %d ",child_time[run_queue[front&10]]);
		count  = 0;
	        if(child_execution_time[run_queue[front%10]] != 0)
        	        run_queue[(rear++)%10] = run_queue[front%10];
		if(child_execution_time[run_queue[front%10]] == 0 ){
			child_execution_time[run_queue[front%10]] = child_execution_ctime[run_queue[front%10]]; //child execution time recover	
		/*	while(1){
				if(child_io_time[run_queue[front%10]] !=0)
					break;
		V	}*/
	
		//	printf("child_io_time %d\n",child_io_time[run_queue[front%10]]);
			flag= 1; 
			//head= insert(head ,child_io_time[run_queue[front%10]], run_queue[front%10]); // insert to wait queue
		}
		front ++; 
	}
}


int main(int argc, char *argv[])
{
	//pid_t pid;
	
	msgq = msgget( key, IPC_CREAT | 0666);
	for(int l=0; l<3;l++)
		child_execution_ctime[l]=child_execution_time[l]; //copty the time
//	curr_execution_time = child_execution_time[i];
        while(i< 3) {
        pid[i] = fork();
        run_queue[(rear++)%10] = i ;
        if (pid[i]== -1) {
                perror("fork error");
                return 0;
        }
        else if (pid[i]== 0) {
                //child
		//io time
		io_time = 5;
		//printf("msgq id: %d\n", msgq);
		curr_execution_time = child_execution_time[i];
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
		
		//printf("msgq %d\n ",msgq);
		memset(&msg, 0, sizeof(msg));

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
	while(1){
	//memset(&msg, 0, sizeof(msg));
		ret = msgrcv(msgq,&msg,sizeof(msg),IPC_NOWAIT,IPC_NOWAIT); //to receive message
		if(ret != -1){
			printf("get message\n");
			//printf("insert io _time %d to child_io_time",msg.io_time);
			child_io_time[msg.pid_index]=msg.io_time;
			memset(&msg, 0, sizeof(msg));

		}
		
	}
        return 0;

}

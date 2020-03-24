#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <math.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>

#define SEM_NAME1 "/semaphore_count"
#define SEM_NAME2 "/semaphore_broadcast"
#define SEM_NAME3 "/semaphore_handler"
#define SEM_NAME4 "/semaphore_tree"
//#define SEM_NAME5 "/semaphore_pcsync"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define INITIAL_VALUE 1

//Shared Vairable
int *count;
//common variables
int N,A,S;
pid_t child[2];

sem_t *semaphore_count;
sem_t *semaphore_broadcast;
sem_t *semaphore_handler;
sem_t *semaphore_tree;
//sem_t *semaphore_pcsync;

void handler(int sig, siginfo_t *info, void *ucontext)
{
	if (sem_wait(semaphore_handler) < 0) {
            perror("1. sem_wait(3) failed on child");
    }

	    printf("In handler of process %d \n" , getpid());
		
		if(info->si_pid == getppid()){
			printf("In process %d Message received from parent %d \n" , getpid(), getppid());
			N+=A;
		}
		else if(info->si_pid == child[0]){
			printf("In process %d Message received from child %d\n", getpid(), child[0]);
			N-=S;
		}
		else if(info->si_pid == child[1]){
			printf("In process %d Message received from child %d\n", getpid(), child[1]);
			N-=S;
		}
		else{
			printf("In process %d Message received from sibling %d\n", getpid() , info->si_pid);
			N-=S/2;
		}

		if(N==0)
		{
			printf("Process %d has reached 0 points and is exiting\n", getpid() );
			/*
			if (sem_close(semaphore_count) < 0) {
		    	perror("sem_close(3) failed");
		    	sem_unlink(SEM_NAME1);
		    	exit(EXIT_FAILURE);
		    }
	        if (sem_close(semaphore_broadcast) < 0) {
		    	perror("sem_close(3) failed");
		    	sem_unlink(SEM_NAME2);
		   		exit(EXIT_FAILURE);
		    }
	        if (sem_close(semaphore_handler) < 0) {
	        	perror("sem_close(3) failed"); 
	        	sem_unlink(SEM_NAME3);
	        	exit(EXIT_FAILURE);
			}*/
			/*if (sem_post(semaphore_count) < 0) {
	            perror("sem_post(3) error on child");
	    	}*/
	        munmap(count, sizeof *count);
	        sleep(1);
			exit(0);
		}

	if (sem_post(semaphore_handler) < 0) {
            perror("sem_post(3) error on child");
    }


}

static pid_t fork_well()
{	
	
    if (sem_wait(semaphore_count) < 0) {
            perror("2. sem_wait(3) failed on child");
            return 1;
    }
	//CS
		if(--(*count) <= 0)
		{	
			if (sem_post(semaphore_count) < 0) {
	            perror("sem_post(3) error on child");
	    	}
			return 1;
		}


		printf("%d \n", *count);

		pid_t pid;
		
		if ((pid = fork()) < 0)
		{
			printf("Fork failed in process %d\n", (int)getpid());
			exit(1);
		}	
	//sleep(1);
	if (sem_post(semaphore_count) < 0) {
            perror("sem_post(3) error on child");
    }
	
	return pid;
}

void createBtree(int level)
{
	
	if (sem_wait(semaphore_tree) < 0) {
            perror("4. sem_wait(3) failed on child");
    }
		printf("Start %d\n", (int)getpid());
		
		/*Loop to create Binary tree*/
		int i = 0;
		for (; i < level; i++)
		{
			if ( (child[0] = fork_well()) == 0 || (child[1] = fork_well()) == 0)
				continue;
			
			/*if (sem_post(semaphore_pcsync) < 0) {
            perror("6.sem_post(3) \n");
    		}*/

			break;
		}
		
		printf("i am %d,my level is %d, my parent is %d,  my children are %d , %d\n",(int)getpid(), i, (int)getppid(), child[0], child[1]);	
	
	if (sem_post(semaphore_tree) < 0) {
            perror("sem_post(3) error on child");
    }
}

int main(int argc, char *argv[])
{//main()
    struct sigaction sa;
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;
    
    sigaction(SIGUSR1, &sa, NULL);

    union sigval sig;

    count = mmap(NULL, sizeof *count, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    //initialize semaphores
    sem_unlink(SEM_NAME1);
    semaphore_count = sem_open(SEM_NAME1, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    if (semaphore_count == SEM_FAILED) {
        perror("sem_open(3) error");
        exit(EXIT_FAILURE);
    }
 
    sem_unlink(SEM_NAME2);
    semaphore_broadcast = sem_open(SEM_NAME2, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    if (semaphore_broadcast == SEM_FAILED) {
        perror("sem_open(3) error");
        exit(EXIT_FAILURE);   
    }

    sem_unlink(SEM_NAME3);
    semaphore_handler = sem_open(SEM_NAME3, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    if (semaphore_handler == SEM_FAILED) {
        perror("sem_open(3) error");
        exit(EXIT_FAILURE);   
    }

    sem_unlink(SEM_NAME4);
    semaphore_tree = sem_open(SEM_NAME4, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    if (semaphore_tree == SEM_FAILED) {
        perror("sem_open(3) error");
        exit(EXIT_FAILURE);   
    }
/*
    sem_unlink(SEM_NAME5);
    semaphore_pcsync = sem_open(SEM_NAME5, O_CREAT | O_EXCL, SEM_PERMS, 0);
    if (semaphore_pcsync == SEM_FAILED) {
        perror("sem_open(3) error");
        exit(EXIT_FAILURE);
    }*/

    N = atoi(argv[1]);	
    A = 1 ;
    S = 2 ;
    *count = N;
	
	//Create the process tree
	createBtree( floor( log(N)/log(2) ) );

	//Broadcast the signals
    if (sem_wait(semaphore_broadcast) < 0) {
            printf("3. sem_wait(3) failed in %d\n " , getpid());
    }
		printf("%d is now broadcasting\n", getpid()  );
		for(int i = (int)getppid() - atoi(argv[1]) ; i <= (int)getpid() + atoi(argv[1]) ; i++)
		{
			if(i != getpid()){
				printf("SIGNAL TO PROCESS : %d by %d\n", i, getpid());
				sigqueue(i , SIGUSR1, sig);
			}
		}

	if (sem_post(semaphore_broadcast) < 0) {
            perror("sem_post(3) error");
    }
	
    //wait for signals to arrive and be caught by handler
	while(1){
		pause();
	}
}//main()

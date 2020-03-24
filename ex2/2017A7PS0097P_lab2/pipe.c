#include <math.h>
#include<signal.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ipc.h> 
#include <fcntl.h>
#include <sys/wait.h>

#define SEM_NAME "/semaphore_sync"
#define SEM_NAME2 "/semaphore_handler"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define INITIAL_VALUE 1
sem_t *semaphore_sync  ;
sem_t *semaphore_handler;
int count;
//###########################################################
void handler(int sig, siginfo_t *info, void *ucontext)
{
	if (sem_wait(semaphore_handler) < 0) {
            perror("sem_wait(inchild[0]) failed\n");
    }
	if(sig==SIGUSR1)
	{
		count++;
		printf("In handler for SIGUSR1; count = %d\n", count);
	}
	else if (sig == SIGUSR2)
	{
		printf("Number of messages exchanged between c1 and c2 = %d\n" , count);
		exit(0);
	} 

	if (sem_post(semaphore_sync) < 0) {
            	perror("sem_post(inchild[0]) failed\n");
    }


}





//###########################################################
void main(int argc, char *argv[])
{
	count = 0;
	int m = atoi(argv[1]);
	int n = atoi(argv[2]);

	printf("M = %d N = %d \n", m , n);
	
	//fd for child[0] to write and child[1] to read
	int c0[2];
	//fd for child[1] to write and child[0] to read
	int c1[2];
	//Create the pipes 
	pipe(c0);
	pipe(c1);

	pid_t parent, child[2];
	parent = getpid();

	//semaphore for synchronization
	sem_unlink(SEM_NAME);
    semaphore_sync = sem_open(SEM_NAME, O_CREAT | O_EXCL, SEM_PERMS, 1);
    if (semaphore_sync == SEM_FAILED) {
        perror("sem_open() error");
        exit(EXIT_FAILURE);
    }
    sem_unlink(SEM_NAME2);
    semaphore_handler = sem_open(SEM_NAME2, O_CREAT | O_EXCL, SEM_PERMS, 0);
    if (semaphore_handler == SEM_FAILED) {
        perror("sem_open() error");
        exit(EXIT_FAILURE);
    }

    //Signal handler
    struct sigaction sa;
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;
    
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    union sigval sig;

    char* buffer = argv[1];
	
	//Creating two children
	for(int i = 0 ; i < 2 ; i++)
	{
		if((child[i]=fork())==0)
		{	
			child[i] = getpid();
			break;
		}
	}

	if(getpid()==parent)
	{
		/*if (sem_wait(semaphore_sync) < 0) {
            perror("sem_wait(inparent) failed\n");
    	}*/

	    	printf("In parent = %d\n" , parent);
			printf("My child[0] =  %d\n" , child[0]);
			printf("My child[1] =  %d\n" , child[1]);
			
			//Close all read and write ends from parent process
			if(close(c0[0]) == -1 || close(c0[1]) == -1 || close(c1[0]) == -1 || close(c1[1]) == -1)
				perror("Close\n");

			while(1)
			{
				//sleep(1);
			}

		/*if (sem_post(semaphore_sync) < 0) {
            perror("sem_post(inparent) failed\n");
        }*/
	}//endif
	else if(getpid() == child[0])
	{
		
		sleep(1);
		printf("In child[0] = %d\n", child[0]);
		printf("My parent = %d\n", getppid());

		//Close read end of c0 and write end of c1
		if( close(c0[0]) ==-1 || close(c1[1]) == -1 )
			perror("Close\n");
		//sleep(1);
		while(1)
		{
			if (sem_wait(semaphore_sync) < 0) {
            perror("sem_wait(inchild[0]) failed\n");
    		}

			write(c0[1] , buffer, 4);
			printf("Sent %d to sibling from %d\n" , atoi(buffer) , getpid());
			sigqueue(getppid() , SIGUSR1 , sig);
			
			if (sem_post(semaphore_handler) < 0) {
            	perror("sem_post(inchild[0]) failed\n");
        	}
			
			read(c1[0] , buffer, 4);
			
			if (sem_wait(semaphore_sync) < 0) {
            perror("sem_wait(inchild[0]) failed\n");
    		}
			printf("Received %d in %d \n" , atoi(buffer) , getpid());
			gcvt(atoi(buffer) / n , 15, buffer);
			if(atoi(buffer)==0)
			{
				sigqueue(getppid() , SIGUSR2 , sig);
				break;
			}

			if (sem_post(semaphore_sync) < 0) {
            	perror("sem_post(inchild[0]) failed\n");
        	}
			
		}//endwhile
		
		if (sem_post(semaphore_handler) < 0) {
            	perror("sem_post(inchild[0]) failed\n");
        }
		
	}//endelseif
	else if(getpid() == child[1])
	{
		
		sleep(1);
		printf("In child[1] = %d\n", child[1]);
		printf("My parent = %d\n", getppid());
			
		//Close read end of c1 and write end of c0
		if( close(c1[0]) == -1 || close(c0[1]) == -1 )
			perror("Close\n");
		//sleep(1);
		while(1)
		{
			

			read(c0[0] , buffer, 4);
			if (sem_wait(semaphore_sync) < 0) {
            	perror("sem_wait(inchild[1]) failed\n");
    		}			
				printf("Received %d in %d \n" , atoi(buffer) , getpid());
				gcvt(atoi(buffer) / n , 15 , buffer) ;

				
				if(atoi(buffer)==0)
				{	
					sigqueue(getppid() , SIGUSR2 , sig);
					break;
				}
				
				write(c1[1] , buffer , 4);
				printf("Sent %d to sibling from %d\n" , atoi(buffer) , getpid());
				sigqueue(getppid() , SIGUSR1 , sig);





			if (sem_post(semaphore_handler) < 0) {
	        	perror("sem_post(inchild[1]) failed\n");
	        }


	    }//endwhile
	    
	    if (sem_post(semaphore_handler) < 0) {
	        	perror("sem_post(inchild[1]) failed\n");
	    }	        	
		
	}//endelseif
		
			
	


	return;

}//endmain
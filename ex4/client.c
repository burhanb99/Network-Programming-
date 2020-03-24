#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/shm.h> 
#include <sys/stat.h> 
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <time.h> 
#define SEM_NAME1 "/semaphore_client"
#define SEM_NAME2 "/semaphore_server"

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
sem_t *semaphore_client;
sem_t *semaphore_server;

//structure for the shared memory data
typedef struct data_item{
pid_t pid; //clients pid
int slno; // incremented for very item
int a; //any number
int b; //any number
}data_item;

int main(int argc , char* argv[])
{
	//Initialize the semaphores
    semaphore_client = sem_open(SEM_NAME1, O_EXCL);
    if (semaphore_client == SEM_FAILED) {
        perror("sem_open(client) error");
        exit(EXIT_FAILURE);
    }

    semaphore_server = sem_open(SEM_NAME2, O_EXCL);
    if (semaphore_server == SEM_FAILED) {
        perror("sem_open(server) error");
        exit(EXIT_FAILURE);
    }

    printf("Opened semaphores successfully \n");

    int shm_fd = shm_open("shared_mem", O_EXCL | O_RDWR, 0666); 
	
    if(argc < 2)
    {
    	printf("Too few args\n");
    	exit(0);
    }

	int N = atoi(argv[1]);
	pid_t pid;
	printf("Main Client PID %d\n",getpid());
	for(int i = 1 ; i <= N ; i++)
	{
		pid = fork();
		if(pid == 0)
		{//into child
			if (sem_wait(semaphore_client) < 0) {
			perror("sem_wait(server) failed\n");
	    	}

	    	int a, b;
	    	data_item* ptr = mmap(NULL, sizeof(data_item), PROT_WRITE, MAP_SHARED, shm_fd, 0); 	
	    
	    	printf("Enter a , b : \n");
	    	scanf("%d %d"  , &a, &b);
	    	data_item temp;
	    	temp.pid = getpid();
	    	temp.slno = i;
	    	temp.a = a;
	    	temp.b = b;

	    	printf("PID of Sub-Client %d\n", getpid());
			printf("slno = %d\n" , temp.slno);
			printf("a = %d\n" , temp.a);
			printf("b = %d\n" , temp.b);
			
			//Code to print semaphore value is not functioning for some reason
			/*
			int *s1,*s2;
			sem_getvalue(semaphore_server , s1);
			sem_getvalue(semaphore_client , s2);
			printf("\nsemaphore_server value = %d\n", *s1);
			printf("semaphore_client value = %d\n",*s2 );
			*/
			ptr = memmove(ptr , &temp , sizeof(data_item));
	    	if (sem_post(semaphore_server) < 0) {
            perror("sem_post(client) failed\n");
    		}
    		return 0;
		}//end if

	}//end for 
	
	//wait for all sub-clients to finish
	while(wait(NULL)>0);

	return 0;
}//end main
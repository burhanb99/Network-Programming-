#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <sys/stat.h> 
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <signal.h>

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

int count = 0;
//Signal handler
void handler(int sig)
{
	printf("\nNumber of data items processed = %d\n", count);
	exit(0);
}

int main(int argc , char* argv[])
{
	if(argc < 2)
	{
		printf("Too few arguments\n");
		exit(0);
	}
	if(atoi(argv[1]) < sizeof(data_item))
	{
		printf("Size not suficient for data_item! re-run server\n");
		exit(0);
	}

	signal(SIGINT , handler);
	//Initialize the semaphores
	sem_unlink(SEM_NAME1);
    semaphore_client = sem_open(SEM_NAME1, O_CREAT | O_EXCL, SEM_PERMS, 1);
    if (semaphore_client == SEM_FAILED) {
        perror("sem_open(client) error");
        exit(EXIT_FAILURE);
    }


	sem_unlink(SEM_NAME2);
    semaphore_server = sem_open(SEM_NAME2, O_CREAT | O_EXCL, SEM_PERMS, 0);
    if (semaphore_server == SEM_FAILED) {
        perror("sem_open(server) error");
        exit(EXIT_FAILURE);
    }

	shm_unlink("shared_mem");
	int shm_fd = shm_open("shared_mem", O_CREAT | O_RDWR, 0666); 
	//ftruncate(shm_fd, atoi(argv[1]));
	ftruncate(shm_fd, sizeof(data_item)); 
	data_item* ptr = mmap(NULL, atoi(argv[1]), PROT_WRITE, MAP_SHARED, shm_fd, 0); 

	printf("Server PID : %d\n", getpid());
	printf("Open a new terminal and run a client\n");
	while(1)
	{
		//Wait for client to write data to shared memory
		if (sem_wait(semaphore_server) < 0) {
			perror("sem_wait(server) failed\n");
	    }

		printf("\nPID of Server %d\n", getpid());
		printf("PID of client-process who last modified shared memory %d\n", ptr->pid);
		printf("slno = %d\n" , ptr->slno);
		printf("a = %d\n" , ptr->a);
		printf("b = %d\n" , ptr->b);
		printf("Sum = %d \n" , ptr->a+ptr->b);
		
		//Code to print semaphore value is not functioning for some reason
		/*
		int *s1,*s2;
		sem_getvalue(semaphore_server , s1);
		sem_getvalue(semaphore_client , s2);
		printf("\nsemaphore_server value = %d\n", *s1);
		printf("semaphore_client value = %d\n",*s2 );
		*/
		
		count++;
		//sem_post()
		if (sem_post(semaphore_client) < 0) {
            perror("sem_post(client) failed\n");
    	}

	}//end of infinite while

	

	return 0;
}//end of main
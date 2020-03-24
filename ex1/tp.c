#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <math.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/mman.h>
#include <semaphore.h>
int N,A,S;
int *count;
int count_read()
{
	key_t key = ftok("shmfile",65);  
	int shmid = shmget(key,1024,0666|IPC_CREAT);  
	char *str = (char*) shmat(shmid,(void*)0,0);
	int temp = atoi(str);	
	shmdt(str);	
	return(temp);
}

void count_write(int count)
{
	key_t key = ftok("shmfile",65);  
	int shmid = shmget(key,1024,0666|IPC_CREAT);  
	char *str = (char*) shmat(shmid,(void*)0,0);
	sprintf(str, "%d", count);
	shmdt(str);

}

static pid_t fork_ok()
{	
	/*int count = count_read();
	count--;	
	if(count == 0)
		return 1;
	count_write(count);*/
	if(--(*count) == 0)
		return 1;
	pid_t pid;
	
	if ((pid = fork()) < 0)
	{
		fprintf(stderr, "Fork failure in pid %d\n", (int)getpid());
		exit(1);
	}
	
	return pid;
}
void createBtree(int count, int level)
{
	//count_write(count);

	fprintf(stderr, "Start %d\n", (int)getpid());
	/*Loop to create Binary tree*/
	int i = 0;
	for (; i < level; i++)
	{
		if (fork_ok() == 0 || fork_ok() == 0)
			continue;

		break;
	}
	
	fprintf(stderr, "i am %d,my level is %d, my parent is %d\n",(int)getpid(),i, (int)getppid());	
/*	while(wait(0) > 0)
		continue;
	 
	exit(0);
	fprintf(stderr, "Exit %d\n", (int)getpid());	
*/

}

int main(int argc, char *argv[])
{
    count = mmap(NULL, sizeof *count, PROT_READ | PROT_WRITE, 
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    N = atoi(argv[1]);	
    A = 1 ;
    S = 2 ;
    *count = N;
	createBtree( N, floor( log(N)/log(2) ) );
	while(1);
}

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
void syncc(int signo)
{
	printf("Signal handler child invoked\n");
//	return;
}
void syncp(int signo)
{
	printf("Signal handler parent invoked\n");
//	return;
}
void main ()
{//main
  	int i = 0, j = 0; 
  	pid_t ret;
  	int status;
  	signal(SIGUSR1, syncp);
  	signal(SIGCHLD, SIG_DFL);
	ret = fork ();
  	
  	if (ret == 0)
    {
      	signal(SIGUSR1, syncc);
      	for (i = 0; i < 3; i++)
      	{
      		
      		printf ("Child: %d\n", i);
      		kill(getppid(), SIGUSR1);
      		pause();
      		
      	}
      	printf ("Child ends\n");
      	kill(getppid() , SIGUSR1);
      	exit(0);
    }
 	else
    {
      	
      	
		sleep(2);      
      
      	for (j = 0; j < 3; j++)
      	{
      		printf ("Parent: %d\n", j);
    		kill(ret, SIGUSR1);
	    	pause();
    	}

    	printf ("Parent ends\n");
	}

}//end

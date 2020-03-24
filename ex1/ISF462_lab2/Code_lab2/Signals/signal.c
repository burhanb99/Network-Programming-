#include <signal.h>
#include <stdio.h>
#include <unistd.h>
void int_handler(int signo);
void main ()
{
	signal (SIGINT, int_handler);
	sigset_t blockSet , prevMask , pending;
	sigemptyset(&blockSet);
	sigaddset(&blockSet, SIGINT);
	sigprocmask(SIG_BLOCK, &blockSet, &prevMask);
	sleep(5);
	//printf ("Entering infinite loop\n");
	sigprocmask(SIG_SETMASK, &prevMask, NULL);

	sigpending(&pending);
	/*if(sigismember(&pending, SIGINT))
		raise(SIGINT);*/

	while(1);
	printf (" This is unreachable\n");
}

/* will be called asynchronously, even during a sleep */
void int_handler(int signo)
{
	printf ("Running int_handler\n");
}

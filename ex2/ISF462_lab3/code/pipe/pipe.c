#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MSGSIZE 16
int main ()
{
  int i;
  char *msg = "How are you?";
  char inbuff[MSGSIZE];
  int p[2];
  int p2[2];
  pid_t ret;
  pipe (p);
  pipe (p2);
  ret = fork ();
  if (ret > 0)
    {
	close(p2[1]);
	close(p[0]);      
	i = 0;
      while (i < 10)
        {
          write (p[1], msg, MSGSIZE);
          //sleep (2);
          read (p2[0], inbuff, MSGSIZE);
          printf ("Parent: %s\n", inbuff);
          i++;
        }
    while(1);
    }
  else
    {
	close(p2[0]);
	close(p[1]);      
	i = 0;
      while (i < 10)
        {
          //sleep (1);
          read (p[0], inbuff, MSGSIZE);
          printf ("Child: %s\n", inbuff);
          write (p2[1], "i am fine", strlen ("i am fine"));
          i++;
        }
    }
  while(1);
return 0;
}

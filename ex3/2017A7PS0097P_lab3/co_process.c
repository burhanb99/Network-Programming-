// C Program for Message Queue (Reader Process)  
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <sys/wait.h>
#include <sys/types.h>
#include<time.h> 

// structure for message queue 

struct mesg_buffer { 
	long mesg_type; 
	char mesg_text[100]; 
} message; 

void upper_string(char *s) {
   int c = 0;
   
   while (s[c] != '\0') {
      if (s[c] >= 'a' && s[c] <= 'z') {
         s[c] = s[c] - 32;
      }
      c++;
   }
}
int main() 
{ 
	key_t key; 
	int msgid; 

	key = ftok("progfile", 65);  
	msgid = msgget(key, 0666 | IPC_CREAT); 

	printf("Message Queue ID : %d\n" , msgid);
	
	printf("Enter a string to format or Enter 'exit' to exit \n"); 
    printf("> ");
    fgets(message.mesg_text ,sizeof(message), stdin);
    
    if(strcmp(message.mesg_text , "exit\n")==0)
    {	
    	message.mesg_type = 30;
    	msgsnd(msgid, &message, sizeof(message), 0);
    	exit(0);
    }    

	srand(time(0)); 
    message.mesg_type = rand()%20;    
	printf("Message Type : %ld\n" , message.mesg_type); 
	upper_string(message.mesg_text);
	msgsnd(msgid, &message, sizeof(message), 0);

	exit(0);
	return 0; 
} 

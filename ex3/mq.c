// C Program for Message Queue (Writer Process) 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <sys/wait.h>
#include <sys/types.h>

// structure for message queue 
struct mesg_buffer { 
    long mesg_type; 
    char mesg_text[100]; 
} message; 
  
int main() 
{ 
    
    key_t key; 
    int msqid; 
    int type ;
    
    key = ftok("progfile", 65); 
  
    // msgget creates a message queue 
    // and returns identifier
    msqid = msgget(key, IPC_EXCL);
    msgctl(msqid, IPC_RMID, NULL); 
    // ftok to generate unique key 
    key = ftok("progfile", 65); 
  
    // msgget creates a message queue 
    // and returns identifier
    msqid = msgget(key, 0666 | IPC_CREAT);
    
    do{
        pid_t p  = fork();

        if(p != 0)
        {//parent process
            wait(NULL);
            
            msgrcv(msqid, &message, sizeof(message), 0, 0);
            
            if(message.mesg_type == 30)
                exit(0);

            printf("Formatted String : %s\n" , message.mesg_text); 
        }
        else if(p == 0)
        {//child process
            
            char* args[] = {"./co_process", NULL};
            execvp(args[0] , args);
            

        } 
    }while(1);
    
    msgctl(msqid, IPC_RMID, NULL); 

    
    printf("Exiting...\n");
    sleep(1);
    return 0; 
} 
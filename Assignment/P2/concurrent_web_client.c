#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ipc.h> 
#include <sys/msg.h>
#include <sys/wait.h>
//Structure for message queue
typedef struct msg_buffer { 
    int msg_type; 
    int clen;
    int status;
    char* msg_text; 
}msg;

typedef struct pointer
{
    char* data;
    int size;
}pointer;

int msgid; 
int sock;
char* domain_name;
char* file_path;
int N;

int ReadHttpStatus(int sock)
{
    char c;
    char buff[1024]="",*ptr=buff+1;
    int bytes_received, status;
    //printf("Begin Response ..\n");
    while(bytes_received = recv(sock, ptr, 1, 0)){
        if(bytes_received==-1){
            perror("ReadHttpStatus");
            exit(1);
        }

        if((ptr[-1]=='\r')  && (*ptr=='\n' )) break;
        ptr++;
    }
    *ptr=0;
    ptr=buff+1;

    sscanf(ptr,"%*s %d ", &status);

    //printf("%s\n",ptr);
    //printf("status=%d\n",status);
    //printf("End Response ..\n");
    return (bytes_received>0)?status:0;

}//end ReadHttpStatus

//Parses response and returns 'Content-Length' 
int ParseHeader(int sock)
{
    char c;
    char buff[1024]="",*ptr=buff+4;
    int bytes_received, status;
    //printf("Begin HEADER ..\n");
    while(bytes_received = recv(sock, ptr, 1, 0)){
        if(bytes_received==-1)
        {
            perror("Parse Header");
            exit(1);
        }

        if(
            (ptr[-3]=='\r')  && (ptr[-2]=='\n' ) &&
            (ptr[-1]=='\r')  && (*ptr=='\n' )
        ) break;
        ptr++;
    }

    *ptr=0;
    ptr=buff+4;
    //printf("%s",ptr);

    if(bytes_received)
    {
        ptr=strstr(ptr,"Content-Length:");
        if(ptr)
        {
            sscanf(ptr,"%*s %d",&bytes_received);
        }
        else
            bytes_received=-1; //unknown size

       //printf("Content-Length: %d\n",bytes_received);
    }
    //printf("End HEADER ..\n");
    return  bytes_received ;

}//end parse header

int fetch_and_push(int range, int i)
{
    char send_data[1024];
    //printf("in fetch_and_push for child %d\n" , i);
    if(i != N)
        snprintf(send_data, sizeof(send_data), "GET /%s HTTP/1.1\r\nHost: %s\r\nRange: bytes=%d-%d\r\n\r\n", file_path, domain_name, ((i-1)*range), (i*range-1));
    else if(i == N)
        snprintf(send_data, sizeof(send_data), "GET /%s HTTP/1.1\r\nHost: %s\r\nRange: bytes=%d-\r\n\r\n", file_path, domain_name, ((i-1)*range));

    if(send(sock, send_data, strlen(send_data), 0)==-1)
    {
        perror("Failed to send GET msg to server\n");
        exit(2); 
    }
    msg m;
    m.msg_type = i;
    //printf("m.type : %d\n", m.msg_type);
    m.status = ReadHttpStatus(sock);
    printf("m.status : %d\n", m.status);
    m.clen = ParseHeader(sock);
    printf("m.clen : %d\n", m.clen);
    m.msg_text = (char * )malloc(sizeof(char) * m.clen);
    int bytes_received = recv(sock, m.msg_text, m.clen, 0);
    printf("bytes_received : %d\n" , bytes_received);
    if(bytes_received==-1)
    {
        perror("recieve");
        exit(3);
    }
    msgsnd(msgid, &m, sizeof(m), 0); 
    exit(0);
    return 0;
}//end fetch_and_push

int main(int argc, char* argv[])
{
	if(argc != 5)
	{
		printf("Incorrect format or too few arguments\n");
		printf("Use as : \n");
		printf("concurrent_downloader <domain_name> <file_path(on_server)> <N(level_of_concurrency)> <save_file_name>\n");
        exit(1);
	}//end if


	//Store CLA's in varibles
	domain_name = argv[1];
	file_path = argv[2];
	N = atoi(argv[3]);

	//Declare necessary variables
	struct sockaddr_in server_addr;
    struct hostent *h;
    char send_data[1024];
  
    //Inistialize the message queues
    key_t key = ftok("progfile", 65); 
    msgid = msgget(key, 0666 | IPC_CREAT); 


    //Resolve domain name to IP 
    h = gethostbyname(domain_name);
    if (h == NULL)
    {
       perror("gethostbyname Error\n");
       exit(1);
    }

    //open socket for communication
    if ((sock = socket(AF_INET, SOCK_STREAM, 0))== -1)
    {
       perror("Socket Error\n");
       exit(1);
    }

    //set the paramters of sockaddr_in structure to establish a connection
	server_addr.sin_family = AF_INET;//ipv4     
    server_addr.sin_port = htons(80);//http port 80
    server_addr.sin_addr = *((struct in_addr *)h->h_addr_list[0]);//ip address
    bzero(&(server_addr.sin_zero),8); 

    //Establish the connection
    printf("Connecting ...\n");
    if (connect(sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1)
    {
       perror("Connection Error\n");
       exit(1); 
    }

    printf("Sending HEAD request to fetch file size ...\n");
    snprintf(send_data, sizeof(send_data), "HEAD /%s HTTP/1.1\r\nHost: %s\r\n\r\n", file_path, domain_name);
	if(send(sock, send_data, strlen(send_data), 0)==-1)
    {
        perror("Failed to send HEAD msg to server\n");
        exit(2); 
    }

    int clen = ParseHeader(sock);
    int range = clen / N;

    for(int i = 1 ; i <= N ; i++)
    {
        pid_t p = fork();
        if(p == 0)
        {
            //execute the child process which fetches data in the range and pushes it to message queue
            fetch_and_push(range, i);

        }//end if in child
        else
        {
            wait(NULL);
            continue;
        }
    }//end of main for
    
    int count = N;

    pointer* p = (pointer*)malloc(sizeof(pointer) * N);
    
    //Extract data and store it in array
    while(count > 0)//keep looping until all segments have been received correctly
    {
        msg ms;
        msgrcv(msgid, &ms, sizeof(ms), 1, 0); 
        if(ms.status != 206 && ms.status != 200)
        {
            pid_t p = fork();
            if(p == 0)
            {
                fetch_and_push(range, ms.msg_type);
            }
            else
            {
                wait(NULL);
                continue;
            }
            
        }//end if wrong status

        p[ms.msg_type-1].data = ms.msg_text;
        p[ms.msg_type-1].size = ms.clen;
        count--;
    }//end while

    FILE* fd=fopen(argv[4],"wb");
    printf("Assembling and Saving data...\n\n");
    //join the data and fwrite it
    for(int i = 0 ; i < N ; i++)
    {
        fwrite(p[i].data,1,p[i].size,fd);

    }//end for 
    printf("Success..\n");
    fclose(fd);
    close(sock);
    msgctl(msgid, IPC_RMID, NULL); 
    printf("\n\nDone.\n\n");

    return 0;
}//end main
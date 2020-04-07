//Example code: A simple server side code, which echos back the received message. 
//Handle multiple socket connections with select and fd_set on Linux 
#include <stdio.h> 
#include <stdlib.h>
#include <sys/ioctl.h> 
#include <sys/poll.h>
#include <sys/socket.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <netinet/in.h> 
#include <errno.h> 
#include <string.h> //strlen 
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <unistd.h> //close 
#include <arpa/inet.h> //close 
#include <sys/types.h> 
#define TRUE 1 
#define FALSE 0 
#define PORT 8888 
	
int main(int argc , char *argv[]) 
{ 
	int opt = TRUE; 
	int master_socket , addrlen , new_socket , max_clients = atoi(argv[1]) , activity, i , valread , sd; 
	int max_sd; 
	struct sockaddr_in address; 
		
	char buffer[1025]; //data buffer of 1K 
		
	/*//set of socket descriptors 
	fd_set readfds; */

	//array of pull fds
	struct pollfd fds[max_clients];
		
	//a message 
	char *message = "connected to poll server..type something to broadcast to other clients: \r\n";  
		
	//create a master socket 
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	//set master socket to allow multiple connections , 
	//this is just a good habit, it will work without this 
	if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	
	//type of socket created 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons(PORT); 
		
	//bind the socket to localhost port 8888 
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	printf("Listener on port %d \n", PORT); 
		
	//try to specify maximum of 3 pending connections for the master socket 
	if (listen(master_socket, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
		
	//accept the incoming connection 
	addrlen = sizeof(address); 
	puts("Waiting for connections ..."); 
	
	
	memset(fds, 0 , sizeof(fds));
	fds[0].fd = master_socket;
	fds[0].events = POLLIN;	
	while(TRUE) 
	{ 
	
		//wait for an activity on one of the sockets , timeout is -ve , so wait indefinitely  
		activity = poll(fds, max_clients, -1);

		if ((activity < 0) && (errno!=EINTR)) 
		{ 
			printf("select error"); 
		} 
			
		//If something happened on the master socket ,then its an incoming connection 
		if (fds[0].revents == POLLIN) 
		{ 
			if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
			{ 
				perror("accept"); 
				exit(EXIT_FAILURE); 
			} 
			
			//inform user of socket number - used in send and receive commands 
			printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
		
			//send new connection greeting message 
			if( send(new_socket, message, strlen(message), 0) != strlen(message) ) 
			{ 
				perror("send"); 
			} 
				
			puts("Welcome message sent successfully"); 
				
			//add new socket to array of sockets 
			for (i = 1; i < max_clients; i++) 
			{ 
				//if position is empty 
				if( fds[i].fd == 0 ) 
				{ 
					fds[i].fd = new_socket; 
					fds[i].events = POLLIN;
					printf("Adding to list of sockets as %d\n" , i); 
						
					break; 
				} 
			}


		} 
		//else its some IO operation on some other socket 
		for (i = 1 ; i < max_clients; i++) 
		{  		
			if (fds[i].revents == POLLIN) 
			{ 
				//Check if it was for closing , and also read the incoming message 
				if ((valread = read( fds[i].fd , buffer, 1024)) == 0) 
				{ 
					//Somebody disconnected , get his details and print 
					getpeername(fds[i].fd , (struct sockaddr*)&address , (socklen_t*)&addrlen); 
					printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
						
					//Close the socket and mark as 0 in list for reuse 
					close(fds[i].fd); 
					fds[i].fd = 0; 
				} 
					
				//Echo back the message that came in to all clients
				else
				{ 
					//set the string terminating NULL byte on the end of the data read 
					buffer[valread] = '\0';
					for(int i = 1 ; i < max_clients ; i++)
					{
						if(fds[i].fd > 0)
						{
							send(fds[i].fd , buffer , strlen(buffer) , 0 ); 
						}	
					}
				}//end else 
			}//end if
		}//end for  
	}//end main while 
	
	return 0; 
} 

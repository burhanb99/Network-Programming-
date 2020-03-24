 
#include<stdio.h>
#include<stdlib.h>
#include<netdb.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include <unistd.h>
#define size 100
int main(int argc, char *argv[])
{
	struct sockaddr_in sock;
	struct hostent *hp,port;
	char *req,*hostname,*cp;
	FILE *local;
	char buff[size];
	int con,i,l,nrv,sd;
	
	if(argc!=3)
	{
		printf("\nUsage: %s <server> filename\n",argv[0]);
		exit(1);
	}
	
	if(cp=strchr(argv[1],'/'))
	{
		*cp='\0';
		l=strlen(argv[1]);
		hostname=malloc(l+1);
		strcpy(hostname,argv[1]);
		*cp='/';
		l=strlen(cp);
		req=malloc(l+1);
		strcpy(req,cp);
	}
	else
	{
		hostname=argv[1];
		req="/";
	}
	
	printf("\nHost=%s\nReq= %s\n",hostname,req);
	sd=socket(AF_INET,SOCK_STREAM,0);
	if(sd<0)
	{
		perror("\nCannot open socket");
		exit(1);
	}
	bzero(&sock,sizeof(sock));
	sock.sin_family=AF_INET;
	sock.sin_addr.s_addr=inet_addr(hostname);
	sock.sin_port=htons(80);
	con=connect(sd,(struct sockaddr *)&sock, sizeof(sock));
	if(con<0)
	{
		perror("\nConnection failed");
		exit(1);
	}
	sprintf(buff,"Get HTTP:%s//1.1\r\nHost: %s\r\nConnection: class\r\n\r", req, hostname);
	printf("Buff=%s\n", buff);
	l=strlen(buff);
	local=fopen(argv[2],"w");
	write(sd,buff,1);
	do
	{
		nrv=read(sd,buff,size);
		if(nrv>0)
		{
			for(i=0;i<nrv;i++)
				putc(buff[i],local);
		}
		else 
			break;
	}while(1);
	
	close(sd);
	fclose(local);
	printf("\n");
	return 0;
}

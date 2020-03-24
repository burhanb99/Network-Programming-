#include<stdio.h>
#include<string.h>    
#include<sys/socket.h>
#include<arpa/inet.h>

// www.axmag.com/download/pdfurl-guide.pdf
int main(int argc , char *argv[])
{
    int socket_desc;

    char *message;
    char server_reply[10000];
    char *filename = "file.pdf";
    int total_len = 0;
    int file_len = 99352;

    int len; 

    FILE *file = NULL;
    struct sockaddr_in server;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    server.sin_addr.s_addr = inet_addr("198.11.181.184");
    server.sin_family = AF_INET;
    server.sin_port = htons( 80 );

    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("connect error");
        return 1;
    }

    puts("Connected\n");

    //Send request
    message = "GET /download/pdfurl-guide.pdf HTTP/1.1\r\nHost: www.axmag.com\r\n\r\n Connection: keep-alive\r\n\r\n Keep-Alive: 300\r\n";

    if( send(socket_desc , message , strlen(message) , 0) < 0)
    {
        puts("Send failed");
        return 1;
    }

    puts("Data Send\n"); 

    remove(filename);
    file = fopen(filename, "ab");

    if(file == NULL){
        printf("File could not opened");
    }   

    while(1)
    {
        int received_len = recv(socket_desc, server_reply , sizeof server_reply , 0);

        if( received_len < 0 ){
            puts("recv failed");
            break;
        }

        total_len += received_len;

        //puts(server_reply);   
        fwrite(server_reply , received_len , 1, file);

        printf("\nReceived byte size = %d\nTotal lenght = %d", received_len, total_len);

        if( total_len >= file_len ){
            break;
        }   
    }

    puts("Reply received\n");

    fclose(file);

    return 0;
}

all : output

output : client.c server.c 
	gcc -g server.c -o server -lrt -pthread
	gcc -g client.c -o client -lrt -pthread
 
clean : 
	rm client
	rm server

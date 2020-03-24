all : exec 

exec : mq
	./mq

mq : mq.c co_process
	gcc -g mq.c -o mq 

co_process : co_process.c
	gcc -g co_process.c -o co_process
clean : 
	rm mq
	rm co_process

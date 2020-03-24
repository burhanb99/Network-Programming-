all : output

exec : output
	./output 16 2

output : pipe.c
	gcc -g pipe.c -pthread -o output 
clean : 
	rm output

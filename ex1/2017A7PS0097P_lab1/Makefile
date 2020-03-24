all : exec

exec : output
	./output 5 1 2

output : signal_tree.c
	gcc -g signal_tree.c -pthread -lm -o output 
clean : 
	rm output

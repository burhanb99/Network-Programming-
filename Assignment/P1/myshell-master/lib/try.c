// C program to show how to read 
// entire string using scanf() 

#include <stdio.h> 
#include <ctype.h>

int main() 
{ 

	char str[20]; 
	printf("Enter something\n"); 

	// Here \n indicates that take the input 
	// until newline is encountered 
	fgets(str , 20 , stdin); 
	for(int i = 0 ; str[i] != '\n' ; i++)
	{
		printf("%c" , str[i]); 
	}
	printf("\n");
	return 0; 
} 

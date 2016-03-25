#include<stdio.h>
#include<stdlib.h>


void stringLower(char *str)
{
	/**Convert a string with uppercase letters to one with all lowercase.**/
	int i = 0;
	while(str[i] != '\0')
	{
		if(str[i] >= 65 && str[i] <= 90)
			str[i] += 32;
		i++;
	}
}



void printError(char c)
{
	/**Printing error messages for socket functions.**/
	switch(c)
	{
		case 'a':
					perror("ERROR. Accepting socket.");
					break;
		case 'b':
					perror("ERROR. Binding socket.");
					break;
		case 'c':
					perror("ERROR. Connecting socket.");
					break;
		case 'f':
					perror("ERROR. Fork a child.");
					break;
		case 'h':
					perror("ERROR. No such host.");
					break;
		case 'o':
					perror("ERROR. Opening socket.");
					break;
		case 'r':
					perror("ERROR. Reading from socket.");
					break;
		case 'w':
					perror("ERROR. Writing on socket.");
					break;
		default:
					perror("ERROR. Unkown.");
	}		
	exit(1);
}
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>


void stringLower(char *str)
{
	/**Convert a string with uppercase letters to one with all lowercase.**/
	while(*str)
	{
		if(*str >= 65 && *str <= 90)
			*str += 32;
		str++;
	}
}

int stringCompare(const char *str1, const char *str2)
{
	/**Compare two strings.**/
	for( ; *str1 == *str2; str1++, str2++)
		if(*str1 == '\0' && *str2 == '\0')
			return 0;
	return *str1 - *str2;
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
		case 'r':
					perror("ERROR. Reading from socket.");
					break;
		case 's':
					perror("ERROR. Opening socket.");
					break;
		case 'w':
					perror("ERROR. Writing on socket.");
					break;
		default:
					perror("ERROR. Unkown.");
	}		
	exit(1);
}

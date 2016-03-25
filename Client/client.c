//client 

#include<stdio.h>
#include<stdlib.h>
#include<netdb.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include"myheader.h"

#define MAXLINE 500

void startClient(int sockFd)
{
	char buffer[MAXLINE], sendMsg[MAXLINE], recvMsg[MAXLINE];
	int n;
	bool entry = false;
	while(!entry)
	{
		// First message from server
		n = read(sockFd, recvMsg, MAXLINE);
		if(n < 0)
			printError('r');
		printf("Server : %s\n", recvMsg);
		fgets(sendMsg, MAXLINE, stdin);
		// Reply to server - Register or Log In
		n = write(sockFd, sendMsg, MAXLINE);
		if(n < 0)
			printError('w');
		stringLower(sendMsg);
		if(strcmp("register", sendMsg) == 0)
		{
			// Server asking for credentials
			n = read(sockFd, recvMsg, MAXLINE);
				printError('r');
			printf("Server : \n%s", recvMsg);
			bzero(sendMsg, MAXLINE);
			fgets(sendMsg, MAXLINE, stdin);
			// Providing credentials to server
			n = write(sockFd, sendMsg, MAXLINE);
			if(n < 0)
				printError('w');
			// Reading success or failure message
			n = read(sockFd, recvMsg, MAXLINE);
			if(n < 0)
				printError('r');
		}
		stringLower(recvMsg);
		if(strcmp("registered successfully", recvMsg) == 0)
		{
			entry = true;
		}
	}
	// successfull log in
	while(true)
	{
		break;
	}
}

int main(int argc, char const *argv[])
{
	int sockFd, port = 9876, n;
	struct sockaddr_in servAddr;
	struct hostent *server;

	if(argc < 3)
	{
		fprintf(stderr, "Not the right number of parameters required after %s.\n", argv[0]);
		exit(0);
	}
	port = atoi(argv[2]);
	// opening the socket
	sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockFd < 0)
		printError('o');
	server = gethostbyname(argv[1]);
	if(server == NULL)
		printError('h');
	// setting the socket address structure
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	bcopy(server->h_addr, &servAddr.sin_addr.s_addr, server->h_length);
	servAddr.sin_port = htons(port);
	// connecting to the server
	if(connect(sockFd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
		printError('c');
	startClient(sockFd);
	close(sockFd);
	return 0;
}
//client 

#include<stdio.h>
#include<stdlib.h>
#include<netdb.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include"myheader.h"

#define MAXLINE 500
#define EQUAL -10

int startCommunication(int sockFd)
{
	/**
		Start the client communication with the server.
		Return - 1, when the user has successfully logged out.
	**/
	printf("In startCommunication.\n");
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	int n, logout = 0;

	while(1)
	{
		// server providing options
		if((n = read(sockFd, recvMsg, MAXLINE)) < 0)
			printError('r');
		printf("\nServer :\n%s\n", recvMsg);

		printf("\nClient : \n");
		fgets(sendMsg, MAXLINE, stdin);

		// replying to server
		if((n = write(sockFd, sendMsg, MAXLINE)) < 0)
			printError('w');
		
		if((n = read(sockFd, recvMsg, MAXLINE)) < 0)
			printError('r');

		stringLower(recvMsg);
		if(strcmp("logout successful.", recvMsg) == 0)
		{
			logout = 1;
			return logout;
		}
	}
}

int logIn(int sockFd)
{
	/**
		Asks client to provide credentials to log in with.
		Return - 1, if log in is successful
				 -1, if there's some error
	**/
	printf("In logIn\n");
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	char welcome[MAXLINE] = "Welcome ";
	int n, i, k, count = 0, wrong = 0;
	int logout;

	bzero(&recvMsg, MAXLINE);
	bzero(&sendMsg, MAXLINE);
	
	// server asking for credentials to log in
	printf("server asking for credentials to log in\n");
	n = read(sockFd, recvMsg, MAXLINE);
	if(n < 0)
		printError('w');

	printf("\nServer :\n%s\n", recvMsg);
	printf("\nClient : \n");
	fgets(sendMsg, MAXLINE, stdin);
	
	// providing log in credentials
	n = write(sockFd, sendMsg, MAXLINE);
	if(n < 0)
		printError('w');
	
	bzero(&recvMsg, MAXLINE);
	if((n = read(sockFd, recvMsg, MAXLINE)) < 0)
		printError('r');
	printf("%s\n", recvMsg);

	// matching the welcome message and id
	k = strlen(welcome);
	for(i = 0; sendMsg[i] != ' '; ++i, ++k)
		welcome[k] = sendMsg[i];

	if(strcmp(welcome, recvMsg) != 0)
		wrong = 1;

	// begin operation
	if(!wrong)
		logout = startCommunication(sockFd);
	else
		logout = -1;
	return logout;
}


void regis(int sockFd)
{
	/**
		Asks client for id and password to register with.
	**/
	printf("In regis.\n");
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	int n;

	while(1)
	{
		// server asking for credentials
		printf("In regis\n");
		printf("server asking for credentials\n");
		
		n = read(sockFd, recvMsg, MAXLINE);
		if(n < 0)
			printError('r');
		printf("\nServer : \n%s\n", recvMsg);
		
		printf("\nClient : \n");
		fgets(sendMsg, MAXLINE, stdin);

		// providing credentials to server
		printf("providing credentials to server\n");
		n = write(sockFd, sendMsg, MAXLINE);
		if(n < 0)
			printError('w');

		// response from the server
		printf("response from the server\n");
		n = read(sockFd, recvMsg, MAXLINE);
		if(n < 0)
			printError('r');
		
		stringLower(recvMsg);
		printf("server : \n%s\n", recvMsg);
		printf("%d", strcmp("success", recvMsg));
		if(strcmp("registered successfully", recvMsg) == 0)
		{
			printf("Returning to start client\n");
			return;
		}
	}
}

void startClient(int sockFd)
{
	/**
		Start the client side operations.
	**/
	printf("In startClient.\n");
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	int n;
	int logout = 0;
	
	while(1)
	{
		printf("In startClient\n");
		bzero(&recvMsg, MAXLINE);
		bzero(&sendMsg, MAXLINE);

		// message from the server
		printf("message from the server\n");
		n = read(sockFd, recvMsg, MAXLINE);
		if(n < 0)
			printError('r');
		printf("\nServer : \n%s\n", recvMsg);
		
		printf("\nClient : \n");
		fgets(sendMsg, MAXLINE, stdin);

		// sending response to server
		printf("sending response to server\n");
		n = write(sockFd, sendMsg, MAXLINE);
		if(n < 0)
			printError('w');
		
		// register
		if(strcmp("register", sendMsg) == EQUAL)
		{
			regis(sockFd);
			printf("returned to start client\n");
			logout = logIn(sockFd);
			if(logout)
			{
				printf("You are logged out.\n");
				return;
			}
		}
		else
			// log in
			if(strcmp("log in", sendMsg) == EQUAL || strcmp("login", sendMsg) == EQUAL)
			{
				logout = logIn(sockFd);
				if(logout)
				{
					printf("You are logged out.\n");
					return;
				}
			}
		
		if(logout == -1)
		{
			printf("Something wrong with the server.\n");
		}
	}
}

int main(int argc, char const *argv[])
{
	int sockFd, port, n;
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